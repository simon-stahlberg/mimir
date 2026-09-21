using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Interop;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Xunit;

namespace Mimir.Search.Tests;

public unsafe class InteropHeuristicTests
{
    [Fact]
    public void BindHeuristic_GoalMatchedPerfect_ReusesInstance()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        GoalCondition boundGoal = GoalCondition.FromProblem(problem);
        GoalCondition equivalentGoal = GoalCondition.FromProblem(problem);
        var heuristic = new PerfectHeuristic(problem, boundGoal);
        IApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        IHeuristic result = HeuristicBinding.Bind(heuristic, equivalentGoal, generator);

        Assert.Same(heuristic, result);
    }

    [Fact]
    public void BindHeuristic_DifferentGoal_RebuildsPerfect()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        GoalCondition boundGoal = GoalCondition.FromProblem(problem);
        GoalCondition differentGoal = SearchTestHelpers.ContradictoryGoal(problem);
        var heuristic = new PerfectHeuristic(problem, boundGoal);
        IApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        IHeuristic result = HeuristicBinding.Bind(heuristic, differentGoal, generator);

        PerfectHeuristic rebound = Assert.IsType<PerfectHeuristic>(result);
        Assert.NotSame(heuristic, rebound);
        Assert.True(double.IsPositiveInfinity(
            rebound.Evaluate(problem.InitialState.Expand(), differentGoal).Value));
    }

    [Fact]
    public void HeuristicEvaluate_CallbackAbort_IsRecordedAndContained()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        GoalCondition goal = GoalCondition.FromProblem(problem);
        int problemHandle = ObjectRegistry.Store(problem);
        int stateHandle = Exports.StoreExtendedState(problem.InitialState.Expand());
        int goalHandle = ObjectRegistry.Store(goal);
        int heuristicHandle = 0;
        Exports.NativeError error = default;

        try
        {
            delegate* unmanaged<int, double*, byte> abort = &AbortEvaluation;
            delegate* unmanaged<int, IntPtr, IntPtr, IntPtr, int> create = &Exports.HeuristicCallback;
            heuristicHandle = create(problemHandle, (IntPtr)abort, IntPtr.Zero, IntPtr.Zero);
            Assert.NotEqual(0, heuristicHandle);

            delegate* unmanaged<int, int, int, double> evaluate = &Exports.HeuristicEvaluate;
            double result = evaluate(heuristicHandle, stateHandle, goalHandle);

            Assert.True(double.IsNaN(result));
            delegate* unmanaged<IntPtr, byte> takeError = &Exports.TakeLastError;
            Assert.Equal(1, takeError((IntPtr)(&error)));
            Assert.Equal((int)Exports.NativeErrorCode.CallbackAborted, error.Code);
            Assert.Equal("A Python callback aborted the search.", Marshal.PtrToStringUTF8(error.Message));
        }
        finally
        {
            delegate* unmanaged<IntPtr, void> freeString = &Exports.FreeString;
            freeString(error.Message);
            freeString(error.SourcePath);
            ObjectRegistry.Release(heuristicHandle);
            ObjectRegistry.Release(goalHandle);
            ObjectRegistry.Release(stateHandle);
            ObjectRegistry.Release(problemHandle);
        }
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void CallbackBatchPreservesPreferredActionsAndSupportsScalarDefault(bool batchOverride)
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        ExtendedState state = problem.InitialState.Expand();
        IApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Mimir.Core.Grounding.Action action = generator.GetApplicableActions(state).First();
        int problemHandle = ObjectRegistry.Store(problem);
        int heuristicHandle = 0;
        try
        {
            delegate* unmanaged<int, double*, byte> scalar = &ScalarEvaluation;
            delegate* unmanaged<int*, int, double*, byte> batch = &BatchEvaluation;
            delegate* unmanaged<int, int, byte*, byte> preferred = &PreferredAction;
            delegate* unmanaged<int, IntPtr, IntPtr, IntPtr, int> create = &Exports.HeuristicCallback;
            heuristicHandle = create(problemHandle, (IntPtr)scalar, (IntPtr)preferred,
                batchOverride ? (IntPtr)batch : IntPtr.Zero);
            IHeuristic heuristic = ObjectRegistry.Get<IHeuristic>(heuristicHandle)!;
            IReadOnlyList<HeuristicEvaluation> results = heuristic.Evaluate(new[] { state, state }, GoalCondition.FromProblem(problem));
            Assert.Equal(batchOverride ? new[] { 1d, 2d } : new[] { 7d, 7d }, results.Select(result => result.Value));
            foreach (HeuristicEvaluation result in results)
            {
                Assert.NotNull(result.IsPreferredAction);
                Assert.True(result.IsPreferredAction(action));
            }
            Assert.Equal(7d, heuristic.Evaluate(state).Value);
        }
        finally
        {
            ObjectRegistry.Release(heuristicHandle);
            ObjectRegistry.Release(problemHandle);
        }
    }

    [UnmanagedCallersOnly]
    private static byte ScalarEvaluation(int stateHandle, double* result)
    {
        ObjectRegistry.Release(stateHandle);
        *result = 7;
        return 1;
    }

    [UnmanagedCallersOnly]
    private static byte BatchEvaluation(int* stateHandles, int count, double* result)
    {
        for (int index = 0; index < count; index++)
        {
            ObjectRegistry.Release(stateHandles[index]);
            stateHandles[index] = 0;
            result[index] = index + 1;
        }
        return 1;
    }

    [UnmanagedCallersOnly]
    private static byte PreferredAction(int stateHandle, int actionHandle, byte* result)
    {
        ObjectRegistry.Release(stateHandle);
        ObjectRegistry.Release(actionHandle);
        *result = 1;
        return 1;
    }

    [UnmanagedCallersOnly]
    private static byte AbortEvaluation(int stateHandle, double* _result)
    {
        ObjectRegistry.Release(stateHandle);
        return 0;
    }
}
