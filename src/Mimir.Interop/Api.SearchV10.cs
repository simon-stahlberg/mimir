using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;

namespace Mimir.Interop;

public static partial class Exports
{
    private sealed class CallbackBridgeV10
    {
        public IntPtr OnExpand;
        public IntPtr OnGoal;
        public IntPtr OnGenerate;
        public IntPtr OnDiscover;
        public IntPtr OnPrune;
        public IntPtr OnFinishLayer;
        public IntPtr OnNewBestH;
        public required GoalCondition Goal;

        public void FireOnExpanded(SearchNode node)
        {
            ExtendedState state = node.State.Expand();
            InvokeState(OnExpand, state);
            if (Goal.IsSatisfied(state)) InvokeState(OnGoal, state);
        }

        public void FireOnGenerate(SearchTransition value) => InvokeTransition(OnGenerate, value);
        public void FireOnDiscover(SearchTransition value) => InvokeTransition(OnDiscover, value);
        public void FireOnPrune(SearchTransition value) => InvokeTransition(OnPrune, value);
        public void FireOnFinishLayer(double value) => InvokeDouble(OnFinishLayer, value);
        public void FireOnNewBestH(double value) => InvokeDouble(OnNewBestH, value);

        private static unsafe void InvokeState(IntPtr callback, ExtendedState state)
        {
            if (callback == IntPtr.Zero) return;
            int handle = StoreExtendedState(state);
            if (((delegate* unmanaged<int, byte>)callback)(handle) == 0)
                throw new CallbackAbortedException();
        }

        private static unsafe void InvokeTransition(IntPtr callback, SearchTransition value)
        {
            if (callback == IntPtr.Zero) return;
            int source = ExpandAndStoreState(value.State);
            int action = ObjectRegistry.Store(value.Action);
            int target = ExpandAndStoreState(value.SuccessorState);
            if (((delegate* unmanaged<int, int, int, byte>)callback)(source, action, target) == 0)
                throw new CallbackAbortedException();
        }

        private static unsafe void InvokeDouble(IntPtr callback, double value)
        {
            if (callback == IntPtr.Zero) return;
            if (((delegate* unmanaged<double, byte>)callback)(value) == 0)
                throw new CallbackAbortedException();
        }
    }

    private static ISearchAlgorithm BuildSearchAlgorithmV10(
        string algorithm,
        Problem problem,
        IHeuristic? heuristic,
        State? startState,
        GoalCondition goal,
        Action<double>? onFinishLayer,
        Action<double>? onNewBestH,
        IDeadEndDetector? deadEndDetector)
    {
        State effectiveStart = startState ?? problem.InitialState;
        if (!ReferenceEquals(effectiveStart.Context, problem.Context))
            throw new ArgumentException("Start state belongs to a different problem.", nameof(startState));
        if (!ReferenceEquals(goal.Problem, problem))
            throw new ArgumentException("Goal belongs to a different problem.", nameof(goal));

        var builder = new SearchBuilder()
            .WithInitialState(effectiveStart)
            .WithGoal(goal)
            .WithDeadEndDetector(deadEndDetector);
        string normalized = algorithm.ToLowerInvariant();
        if (onFinishLayer != null && normalized == "bfs") builder.OnGLayerFinished(onFinishLayer);
        if (onFinishLayer != null && normalized == "astar") builder.OnFLayerFinished(onFinishLayer);
        if (onNewBestH != null && normalized == "gbfs") builder.OnNewBestHValue(onNewBestH);

        return normalized switch
        {
            "bfs" => builder.BuildBfs(),
            "ucs" => builder.BuildUcs(),
            "astar" => builder.WithHeuristic(heuristic ?? throw new ArgumentNullException(nameof(heuristic))).BuildAStar(),
            "gbfs" => builder.WithHeuristic(heuristic ?? throw new ArgumentNullException(nameof(heuristic))).BuildGbfs(),
            var iw when iw.StartsWith("iw", StringComparison.Ordinal) => builder.BuildIw(int.Parse(iw[2..])),
            _ => throw new ArgumentException($"Unknown algorithm '{algorithm}'."),
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_search")]
    public static int SearchV10(
        IntPtr algorithmPtr,
        int problemHandle,
        int heuristicHandle,
        int startStateHandle,
        int goalHandle,
        double timeoutSeconds,
        int maxExpandedStates,
        IntPtr onExpand,
        IntPtr onGoal,
        IntPtr onGenerate,
        IntPtr onDiscover,
        IntPtr onPrune,
        IntPtr onFinishLayer,
        IntPtr onNewBestH,
        int deadEndDetectorHandle)
    {
        string? algorithm = ReadUtf8(algorithmPtr);
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        ExtendedState? extendedStartState = startStateHandle == 0
            ? null
            : ReadExtendedState(startStateHandle);
        State? startState = extendedStartState?.State;
        var goal = goalHandle == 0
            ? problem == null ? null : GoalCondition.FromProblem(problem)
            : ObjectRegistry.Get<GoalCondition>(goalHandle);
        var heuristic = heuristicHandle == 0 ? null : ObjectRegistry.Get<IHeuristic>(heuristicHandle);

        if (algorithm == null || problem == null || goal == null) return 0;
        if (startStateHandle != 0 && startState == null) return 0;
        if (heuristicHandle != 0 && heuristic == null) return 0;
        if (double.IsNaN(timeoutSeconds) || double.IsPositiveInfinity(timeoutSeconds) || timeoutSeconds < -1d)
            return 0;
        if (maxExpandedStates < -1) return 0;

        return CreateHandle(() =>
        {
            var setupStopwatch = System.Diagnostics.Stopwatch.StartNew();
            var bridge = new CallbackBridgeV10
            {
                OnExpand = onExpand,
                OnGoal = onGoal,
                OnGenerate = onGenerate,
                OnDiscover = onDiscover,
                OnPrune = onPrune,
                OnFinishLayer = onFinishLayer,
                OnNewBestH = onNewBestH,
                Goal = goal,
            };
            ISearchAlgorithm search = BuildSearchAlgorithmV10(
                algorithm,
                problem,
                heuristic,
                startState,
                goal,
                onFinishLayer == IntPtr.Zero ? null : bridge.FireOnFinishLayer,
                onNewBestH == IntPtr.Zero ? null : bridge.FireOnNewBestH,
                deadEndDetectorHandle == 0 ? null : RequireHandle<IDeadEndDetector>(deadEndDetectorHandle));
            if (onExpand != IntPtr.Zero || onGoal != IntPtr.Zero)
                search.NodeExpanded += bridge.FireOnExpanded;
            if (onGenerate != IntPtr.Zero) search.TransitionGenerated += bridge.FireOnGenerate;
            if (onDiscover != IntPtr.Zero) search.TransitionDiscovered += bridge.FireOnDiscover;
            if (onPrune != IntPtr.Zero) search.TransitionPruned += bridge.FireOnPrune;
            setupStopwatch.Stop();

            using var cancellation = new CancellationTokenSource();
            if (timeoutSeconds == 0d) cancellation.Cancel();
            if (timeoutSeconds > 0d) cancellation.CancelAfter(TimeSpan.FromSeconds(timeoutSeconds));

            var searchStopwatch = System.Diagnostics.Stopwatch.StartNew();
            SearchResult result = search.Search(
                cancellation.Token,
                maxExpandedStates < 0 ? null : maxExpandedStates);
            searchStopwatch.Stop();
            return new PlanResult(
                result.Status,
                result.Plan,
                result.Statistics,
                setupStopwatch.Elapsed,
                searchStopwatch.Elapsed);
        });
    }
}
