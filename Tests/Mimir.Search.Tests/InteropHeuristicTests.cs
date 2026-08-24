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
    public void BindHeuristicV10_GoalMatchedPerfect_ReusesInstance()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        GoalCondition boundGoal = GoalCondition.FromProblem(problem);
        GoalCondition equivalentGoal = GoalCondition.FromProblem(problem);
        var heuristic = new PerfectHeuristic(problem, boundGoal);
        IApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        IHeuristic result = Exports.BindHeuristicV10(heuristic, equivalentGoal, generator);

        Assert.Same(heuristic, result);
    }

    [Fact]
    public void BindHeuristicV10_DifferentGoal_RebuildsPerfect()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        GoalCondition boundGoal = GoalCondition.FromProblem(problem);
        GoalCondition differentGoal = SearchTestHelpers.ContradictoryGoal(problem);
        var heuristic = new PerfectHeuristic(problem, boundGoal);
        IApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        IHeuristic result = Exports.BindHeuristicV10(heuristic, differentGoal, generator);

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
            delegate* unmanaged<int, IntPtr, IntPtr, int> create = &Exports.HeuristicCallback;
            heuristicHandle = create(problemHandle, (IntPtr)abort, IntPtr.Zero);
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

    [UnmanagedCallersOnly]
    private static byte AbortEvaluation(int stateHandle, double* _result)
    {
        ObjectRegistry.Release(stateHandle);
        return 0;
    }
}
