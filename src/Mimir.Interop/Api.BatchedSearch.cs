using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Heuristics;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_qgbfs_search")]
    public static int QGbfsSearch(int problemHandle, int startStateHandle, int goalHandle,
        IntPtr evaluate, int batchTarget, byte maximize, double timeoutSeconds, int maxExpandedStates, IntPtr shouldStop)
        => RunBatchedSearch(problemHandle, startStateHandle, goalHandle, timeoutSeconds, maxExpandedStates, shouldStop,
            (problem, start, goal, stop) => new SearchBuilder()
                .WithInitialState(start).WithGoal(goal)
                .WithActionGenerator(problem.GetApplicableActionGenerator(start))
                .WithQHeuristic(new CallbackQHeuristic(problem, evaluate))
                .BuildQGbfs(batchTarget, maximize != 0, stop));

    [UnmanagedCallersOnly(EntryPoint = "mimir_qbeam_search")]
    public static int QBeamSearch(int problemHandle, int startStateHandle, int goalHandle,
        IntPtr evaluate, int beamSize, int maxDepth, byte maximize,
        double timeoutSeconds, int maxExpandedStates, IntPtr shouldStop)
        => RunBatchedSearch(problemHandle, startStateHandle, goalHandle, timeoutSeconds, maxExpandedStates, shouldStop,
            (problem, start, goal, stop) => new SearchBuilder()
                .WithInitialState(start).WithGoal(goal)
                .WithActionGenerator(problem.GetApplicableActionGenerator(start))
                .WithQHeuristic(new CallbackQHeuristic(problem, evaluate))
                .BuildQBeam(beamSize, maxDepth, maximize != 0, stop));

    [UnmanagedCallersOnly(EntryPoint = "mimir_beam_search")]
    public static int BeamSearch(int problemHandle, int startStateHandle, int goalHandle,
        int heuristicHandle, int beamSize, int maxDepth,
        double timeoutSeconds, int maxExpandedStates, IntPtr shouldStop)
        => RunBatchedSearch(problemHandle, startStateHandle, goalHandle, timeoutSeconds, maxExpandedStates, shouldStop,
            (problem, start, goal, stop) =>
            {
                IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(start);
                IHeuristic heuristic = BindHeuristicV10(RequireHandle<IHeuristic>(heuristicHandle), goal, generator);
                return new SearchBuilder().WithInitialState(start).WithGoal(goal)
                    .WithActionGenerator(generator).WithHeuristic(heuristic)
                    .BuildBeam(beamSize, maxDepth, stop);
            });

    private static unsafe int RunBatchedSearch(int problemHandle, int startStateHandle, int goalHandle,
        double timeoutSeconds, int maxExpandedStates, IntPtr shouldStop,
        Func<Problem, State, GoalCondition, Func<int, bool>?, ISearchAlgorithm> build)
    {
        return CreateHandle(() =>
        {
            Problem problem = RequireHandle<Problem>(problemHandle);
            State start = startStateHandle == 0 ? problem.InitialState : RequireHandle<ExtendedState>(startStateHandle).State;
            GoalCondition goal = goalHandle == 0 ? GoalCondition.FromProblem(problem) : RequireHandle<GoalCondition>(goalHandle);
            if (!double.IsFinite(timeoutSeconds) || (timeoutSeconds < 0 && timeoutSeconds != -1))
                throw new ArgumentOutOfRangeException(nameof(timeoutSeconds));
            if (maxExpandedStates < -1) throw new ArgumentOutOfRangeException(nameof(maxExpandedStates));
            if (!ReferenceEquals(start.Context, problem.Context) || !ReferenceEquals(goal.Problem, problem))
                throw new ArgumentException("Start state and goal must belong to the search problem.");

            bool Stop(int expanded)
            {
                byte result = ((delegate* unmanaged<int, byte>)shouldStop)(expanded);
                if (result > 1) throw new CallbackAbortedException();
                return result == 1;
            }

            using var cancellation = new CancellationTokenSource();
            if (timeoutSeconds == 0) cancellation.Cancel();
            if (timeoutSeconds > 0) cancellation.CancelAfter(TimeSpan.FromSeconds(timeoutSeconds));
            var setupWatch = System.Diagnostics.Stopwatch.StartNew();
            ISearchAlgorithm search = build(problem, start, goal, shouldStop == IntPtr.Zero ? null : Stop);
            setupWatch.Stop();
            var searchWatch = System.Diagnostics.Stopwatch.StartNew();
            SearchResult result = search.Search(cancellation.Token, maxExpandedStates < 0 ? null : maxExpandedStates);
            searchWatch.Stop();
            return new PlanResult(result.Status, result.Plan, result.Statistics, setupWatch.Elapsed, searchWatch.Elapsed)
            {
                PartialPlan = result.PartialPlan,
                ActionValues = result.ActionValues,
                EndState = result.EndState,
            };
        });
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_visited_states")]
    public static int PlanResultVisitedStates(int handle)
        => ReadValue(handle, -1, (PlanResult result) => result.Statistics.VisitedStates ?? -1);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_generated_transitions")]
    public static int PlanResultGeneratedTransitions(int handle)
        => ReadValue(handle, -1, (PlanResult result) => result.Statistics.GeneratedTransitions ?? -1);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_evaluated_candidates")]
    public static int PlanResultEvaluatedCandidates(int handle)
        => ReadValue(handle, -1, (PlanResult result) => result.Statistics.EvaluatedCandidates ?? -1);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_partial_plan_length")]
    public static int PlanResultPartialPlanLength(int handle)
        => ReadValue(handle, -1, (PlanResult result) => result.PartialPlan.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_copy_scored_path")]
    public static unsafe int PlanResultCopyScoredPath(int handle, IntPtr handles, IntPtr values, IntPtr hasValues, int count)
    {
        return ReadValue(handle, -1, (PlanResult result) =>
        {
            IReadOnlyList<GroundAction> path = result.IsSuccess ? result.Plan : result.PartialPlan;
            if (count != path.Count || result.ActionValues.Count != count)
                throw new ArgumentException("Path buffer length must match the scored path.");
            if (handles == IntPtr.Zero || (count > 0 && (values == IntPtr.Zero || hasValues == IntPtr.Zero)))
                throw new ArgumentNullException(nameof(handles));
            var destination = new Span<int>((void*)handles, count + 1);
            destination.Clear();
            try
            {
                for (int index = 0; index < count; index++)
                {
                    destination[index] = ObjectRegistry.Store(path[index]);
                    ((double*)values)[index] = result.ActionValues[index].GetValueOrDefault();
                    ((byte*)hasValues)[index] = result.ActionValues[index].HasValue ? (byte)1 : (byte)0;
                }
                if (result.IsSuccess && result.EndState != null)
                    destination[count] = ExpandAndStoreState(result.EndState);
                return count;
            }
            catch
            {
                foreach (int owned in destination) ObjectRegistry.Release(owned);
                destination.Clear();
                throw;
            }
        });
    }
}
