using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    // -- Built-in heuristics: blind, goal-count, lifted-ff --

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_blind")]
    public static int HeuristicBlind()
        => ObjectRegistry.Store(BlindHeuristic.Instance);

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_goal_count")]
    public static int HeuristicGoalCount(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new GoalCountHeuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_lifted_ff")]
    public static int HeuristicLiftedFf(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new LiftedFfHeuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_perfect")]
    public static int HeuristicPerfect(int problemHandle, int goalHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        var goal = goalHandle == 0
            ? problem == null ? null : GoalCondition.FromProblem(problem)
            : ObjectRegistry.Get<GoalCondition>(goalHandle);
        if (problem == null || goal == null) return 0;
        return CreateHandle(() => new PerfectHeuristic(problem, goal));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_add")]
    public static int HeuristicAdd(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new AddHeuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_max")]
    public static int HeuristicMax(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new MaxHeuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_ff_grounded")]
    public static int HeuristicFfGrounded(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new FFHeuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_set_add")]
    public static int HeuristicSetAdd(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new SetAddHeuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_h2")]
    public static int HeuristicH2(int problemHandle)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        return CreateHandle(() => new H2Heuristic(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_evaluate")]
    public static double HeuristicEvaluate(int heuristicHandle, int stateHandle, int goalHandle)
    {
        var heuristic = ObjectRegistry.Get<IHeuristic>(heuristicHandle);
        ExtendedState? state = ReadExtendedState(stateHandle);
        var goal = goalHandle == 0
            ? state == null ? null : GoalCondition.FromProblem(state.State.Context.Problem)
            : ObjectRegistry.Get<GoalCondition>(goalHandle);
        if (heuristic == null || state == null || goal == null) return double.NaN;

        try
        {
            IHeuristic effectiveHeuristic = heuristic;
            if (heuristic is IGroundedHeuristic && heuristic is not H2Heuristic)
            {
                Problem problem = state.State.Context.Problem;
                GroundedApplicableActionGenerator generator = GetGroundedGenerator(problem, state.State);
                effectiveHeuristic = HeuristicBinding.Bind(
                    heuristic,
                    goal,
                    generator);
            }
            return effectiveHeuristic.Evaluate(state, goal).Value;
        }
        catch (ArgumentException)
        {
            return double.NaN;
        }
        catch (InvalidOperationException)
        {
            return double.NaN;
        }
        catch (CallbackAbortedException exception)
        {
            RecordError(exception);
            return double.NaN;
        }
    }

    /// <summary>
    /// Evaluate the heuristic AND collect the preferred actions for the given state.
    /// Returns a handle to a List&lt;GroundAction&gt; — empty if the heuristic doesn't
    /// expose preferred actions or none of the applicable actions are preferred.
    /// </summary>
    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_get_preferred_actions")]
    public static int HeuristicGetPreferredActions(
        int heuristicHandle,
        int stateHandle,
        int problemHandle,
        int goalHandle)
    {
        var heuristic = ObjectRegistry.Get<IHeuristic>(heuristicHandle);
        ExtendedState? state = ReadExtendedState(stateHandle);
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        var goal = goalHandle == 0
            ? problem == null ? null : GoalCondition.FromProblem(problem)
            : ObjectRegistry.Get<GoalCondition>(goalHandle);
        if (heuristic == null || state == null || problem == null || goal == null) return 0;

        return CreateHandle(() =>
        {
            GroundedApplicableActionGenerator? generator = null;
            IHeuristic effectiveHeuristic = heuristic;
            if (heuristic is IGroundedHeuristic && heuristic is not H2Heuristic)
            {
                generator = GetGroundedGenerator(problem, state.State);
                effectiveHeuristic = HeuristicBinding.Bind(
                    heuristic,
                    goal,
                    generator);
            }

            var eval = effectiveHeuristic.Evaluate(state, goal);
            var preferred = new List<GroundAction>();
            if (eval.IsPreferredAction != null)
            {
                generator ??= GetGroundedGenerator(problem, state.State);
                foreach (var action in generator.GetApplicableActions(state))
                {
                    if (eval.IsPreferredAction(action))
                        preferred.Add(action);
                }
            }
            return preferred;
        });
    }

    private sealed class CallbackAbortedException : Exception
    {
        public CallbackAbortedException()
            : base("A Python callback aborted the search.")
        {
        }
    }

    private sealed class CallbackHeuristic : IHeuristic
    {
        private readonly Problem _problem;
        private readonly IntPtr _evaluate;
        private readonly IntPtr _isPreferred;
        private readonly IntPtr _evaluateBatch;

        public CallbackHeuristic(Problem problem, IntPtr evaluate, IntPtr isPreferred, IntPtr evaluateBatch)
        {
            _problem = problem;
            _evaluate = evaluate;
            _isPreferred = isPreferred;
            _evaluateBatch = evaluateBatch;
        }

        public unsafe IReadOnlyList<HeuristicEvaluation> Evaluate(IReadOnlyList<ExtendedState> states,
            GoalCondition? goal = null)
        {
            var evaluations = new HeuristicEvaluation[states.Count];
            if (_evaluateBatch == IntPtr.Zero)
            {
                for (int index = 0; index < states.Count; index++)
                    evaluations[index] = Evaluate(states[index], goal);
                return evaluations;
            }
            if (goal != null && !ReferenceEquals(goal.Problem, _problem))
                throw new ArgumentException("Goal belongs to a different problem.", nameof(goal));
            var handles = new int[states.Count];
            var values = new double[states.Count];
            try
            {
                for (int index = 0; index < states.Count; index++)
                {
                    if (!ReferenceEquals(states[index].State.Context.Problem, _problem))
                        throw new ArgumentException("State belongs to a different problem.", nameof(states));
                    handles[index] = StoreExtendedState(states[index]);
                }
                fixed (int* handlePointer = handles)
                fixed (double* valuePointer = values)
                {
                    if (((delegate* unmanaged<int*, int, double*, byte>)_evaluateBatch)(
                        handlePointer, states.Count, valuePointer) == 0)
                        throw new CallbackAbortedException();
                }
                for (int index = 0; index < states.Count; index++)
                {
                    evaluations[index] = CreateEvaluation(states[index], values[index]);
                }
                return evaluations;
            }
            finally
            {
                // Python zeros each slot as its wrapper adopts the handle.
                foreach (int handle in handles)
                    if (handle != 0) ObjectRegistry.Release(handle);
            }
        }

        public unsafe HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
        {
            if (!ReferenceEquals(state.State.Context.Problem, _problem))
                throw new ArgumentException("State belongs to a different problem.", nameof(state));
            if (goal != null && !ReferenceEquals(goal.Problem, _problem))
                throw new ArgumentException("Goal belongs to a different problem.", nameof(goal));

            int stateHandle = StoreExtendedState(state);
            double value = 0d;
            byte succeeded = ((delegate* unmanaged<int, double*, byte>)_evaluate)(stateHandle, &value);
            if (succeeded == 0) throw new CallbackAbortedException();
            return CreateEvaluation(state, value);
        }

        private unsafe HeuristicEvaluation CreateEvaluation(ExtendedState state, double value)
        {
            if (double.IsNaN(value) || double.IsNegativeInfinity(value) || value < 0d)
                throw new InvalidOperationException($"Heuristic returned invalid value '{value}'.");

            if (_isPreferred == IntPtr.Zero)
                return new HeuristicEvaluation(value);

            bool IsPreferred(GroundAction action)
            {
                int callbackStateHandle = StoreExtendedState(state);
                int actionHandle = ObjectRegistry.Store(action);
                byte preferred = 0;
                byte callbackSucceeded = ((delegate* unmanaged<int, int, byte*, byte>)_isPreferred)(
                    callbackStateHandle,
                    actionHandle,
                    &preferred);
                if (callbackSucceeded == 0) throw new CallbackAbortedException();
                return preferred != 0;
            }

            return new HeuristicEvaluation(value, IsPreferred);
        }
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_callback")]
    public static int HeuristicCallback(
        int problemHandle,
        IntPtr evaluate,
        IntPtr isPreferred,
        IntPtr evaluateBatch)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null || evaluate == IntPtr.Zero) return 0;
        return ObjectRegistry.Store(new CallbackHeuristic(problem, evaluate, isPreferred, evaluateBatch));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_heuristic_evaluate_batch")]
    public static unsafe byte HeuristicEvaluateBatch(int heuristicHandle, IntPtr stateHandles,
        int count, int goalHandle, IntPtr values)
    {
        return ReadValue<IHeuristic, byte>(heuristicHandle, 0, heuristic =>
        {
            if (count < 0) throw new ArgumentOutOfRangeException(nameof(count));
            if (count > 0 && (stateHandles == IntPtr.Zero || values == IntPtr.Zero))
                throw new ArgumentNullException(nameof(stateHandles));
            var states = new ExtendedState[count];
            for (int index = 0; index < count; index++)
                states[index] = RequireHandle<ExtendedState>(((int*)stateHandles)[index]);
            if (count == 0) return 1;
            GoalCondition goal = goalHandle == 0
                ? GoalCondition.FromProblem(states[0].State.Context.Problem)
                : RequireHandle<GoalCondition>(goalHandle);
            foreach (ExtendedState state in states)
                if (!ReferenceEquals(state.State.Context.Problem, goal.Problem))
                    throw new ArgumentException("State and goal belong to different problems.");

            if (heuristic is IGroundedHeuristic && heuristic is not H2Heuristic)
            {
                // Grounding is relative to each query state, just as for scalar evaluation.
                for (int index = 0; index < count; index++)
                {
                    IHeuristic bound = HeuristicBinding.Bind(heuristic, goal,
                        GetGroundedGenerator(goal.Problem, states[index].State));
                    ((double*)values)[index] = bound.Evaluate(states[index], goal).Value;
                }
                return 1;
            }
            IReadOnlyList<HeuristicEvaluation> evaluations = heuristic.Evaluate(states, goal);
            if (evaluations == null || evaluations.Count != count)
                throw new InvalidOperationException("Heuristic returned an incorrect number of values.");
            for (int index = 0; index < count; index++)
                ((double*)values)[index] = evaluations[index].Value;
            return 1;
        });
    }
}
