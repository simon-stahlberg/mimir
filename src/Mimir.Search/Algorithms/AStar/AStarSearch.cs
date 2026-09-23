using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.AStar;

internal sealed class AStarSearch : ISearchAlgorithm
{
    private readonly State _initialState;
    private readonly IDeadEndDetector? _deadEndDetector;
    private readonly GoalCondition _goalCondition;
    private readonly IApplicableActionGenerator _actionGenerator;
    private readonly IHeuristic _heuristic;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchTransition>? StateGenerated;
    public event Action<State>? StatePruned;
    public event Action<double>? FLayerFinished;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;

    public AStarSearch(
        State initialState,
        GoalCondition goalCondition,
        IHeuristic heuristic,
        IDeadEndDetector? deadEndDetector)
    {
        _initialState = initialState;
        _deadEndDetector = deadEndDetector;
        _goalCondition = goalCondition;
        _actionGenerator = initialState.Context.Problem.GetApplicableActionGenerator(initialState);
        _heuristic = HeuristicBinding.Bind(heuristic, goalCondition, _actionGenerator);
    }

    public SearchResult Search(
        CancellationToken cancellationToken = default,
        int? maxExpandedStates = null)
    {
        if (maxExpandedStates < 0)
            throw new ArgumentOutOfRangeException(nameof(maxExpandedStates));

        var stopwatch = Stopwatch.StartNew();
        if (cancellationToken.IsCancellationRequested)
            return SearchResult.Canceled(new SearchStatistics(0, 0, stopwatch.Elapsed, 0));

        if (_deadEndDetector is not null)
        {
            ExtendedState initial = _initialState.Expand();
            if (!_goalCondition.IsSatisfied(initial) && _deadEndDetector.IsDeadEnd(initial, _goalCondition))
            {
                NodeGenerated?.Invoke(new SearchNode(_initialState));
                StatePruned?.Invoke(_initialState);
                return SearchResult.Failure(new SearchStatistics(0, 1, stopwatch.Elapsed, 0));
            }
        }

        var open = new PriorityQueue<
            (SearchNode Node, ExtendedState ExtendedState, double FValue),
            (double EstimatedTotalCost, double HeuristicValue, long Sequence)>();
        var bestCosts = new Dictionary<State, double>();
        var prunedStates = new HashSet<State>();

        long sequence = 0;
        var root = new SearchNode(_initialState);
        ExtendedState extendedInitialState = _initialState.Expand();
        double rootHeuristic = _goalCondition.IsSatisfied(extendedInitialState)
            ? 0d
            : EvaluateHeuristic(extendedInitialState).Value;
        double currentFValue = rootHeuristic;
        open.Enqueue(
            (root, extendedInitialState, rootHeuristic),
            (rootHeuristic, rootHeuristic, sequence++));
        bestCosts[_initialState] = 0;
        NodeGenerated?.Invoke(root);
        FLayerFinished?.Invoke(currentFValue);

        int expanded = 0;
        int generated = 1;
        int maxDepth = 0;

        while (open.Count > 0)
        {
            if (cancellationToken.IsCancellationRequested)
                return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            var currentEntry = open.Dequeue();
            var current = currentEntry.Node;
            if (bestCosts.TryGetValue(current.State, out var bestKnownCost) && current.Cost > bestKnownCost)
                continue;

            if (expanded >= maxExpandedStates)
                return SearchResult.ExpansionLimitReached(
                    new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            if (currentEntry.FValue > currentFValue)
            {
                FLayerFinished?.Invoke(currentFValue);
                currentFValue = currentEntry.FValue;
            }

            expanded++;
            NodeExpanded?.Invoke(current);
            maxDepth = Math.Max(maxDepth, current.Depth);

            ExtendedState extendedState = currentEntry.ExtendedState;
            if (_goalCondition.IsSatisfied(extendedState))
            {
                stopwatch.Stop();
                return SearchResult.Success(current.ExtractPlan(), new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
            }

            foreach (var action in _actionGenerator.GetApplicableActions(extendedState))
            {
                if (cancellationToken.IsCancellationRequested)
                    return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

                var nextState = extendedState.Apply(action);
                double nextCost = current.Cost + action.Cost;
                var transition = new SearchTransition(current.State, action, action.Cost, nextState);
                StateGenerated?.Invoke(transition);
                TransitionGenerated?.Invoke(transition);

                if (bestCosts.TryGetValue(nextState, out var bestExistingCost) && nextCost >= bestExistingCost)
                {
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                if (prunedStates.Contains(nextState))
                {
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                var nextNode = new SearchNode(nextState, action, current, nextCost, current.Depth + 1);
                ExtendedState extendedNextState = nextState.Expand();
                bool successorIsGoal = _goalCondition.IsSatisfied(extendedNextState);
                double heuristicValue = EvaluateHeuristic(extendedNextState).Value;
                if (!successorIsGoal && (double.IsPositiveInfinity(heuristicValue)
                    || (_deadEndDetector?.IsDeadEnd(extendedNextState, _goalCondition) ?? false)))
                {
                    prunedStates.Add(nextState);
                    StatePruned?.Invoke(nextState);
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                bestCosts[nextState] = nextCost;
                double estimatedTotalCost = nextCost + heuristicValue;
                generated++;
                NodeGenerated?.Invoke(nextNode);
                TransitionDiscovered?.Invoke(transition);
                open.Enqueue(
                    (nextNode, extendedNextState, estimatedTotalCost),
                    (estimatedTotalCost, heuristicValue, sequence++));
            }
        }

        stopwatch.Stop();
        return SearchResult.Failure(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
    }

    private HeuristicEvaluation EvaluateHeuristic(ExtendedState state)
    {
        HeuristicEvaluation evaluation = _heuristic.Evaluate(state, _goalCondition);
        double value = evaluation.Value;
        if (double.IsNaN(value) || double.IsNegativeInfinity(value) || value < 0d)
        {
            throw new InvalidOperationException(
                $"Heuristic returned invalid value '{value}'.");
        }

        return evaluation;
    }
}
