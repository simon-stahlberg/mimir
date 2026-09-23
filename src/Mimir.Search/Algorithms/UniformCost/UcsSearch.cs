using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.UniformCost;

internal sealed class UcsSearch : ISearchAlgorithm
{
    private readonly State _initialState;
    private readonly IDeadEndDetector? _deadEndDetector;
    private readonly GoalCondition _goalCondition;
    private readonly IApplicableActionGenerator _actionGenerator;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;

    public UcsSearch(
        State initialState,
        GoalCondition goalCondition,
        IDeadEndDetector? deadEndDetector)
    {
        _initialState = initialState;
        _deadEndDetector = deadEndDetector;
        _goalCondition = goalCondition;
        _actionGenerator = initialState.Context.Problem.GetApplicableActionGenerator(initialState);
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
                return SearchResult.Failure(new SearchStatistics(0, 1, stopwatch.Elapsed, 0));
            }
        }

        var open = new PriorityQueue<SearchNode, (double PathCost, long Sequence)>();
        var bestCosts = new Dictionary<State, double>();

        long sequence = 0;
        var root = new SearchNode(_initialState);
        open.Enqueue(root, (0d, sequence++));
        bestCosts[_initialState] = 0d;
        NodeGenerated?.Invoke(root);

        int expanded = 0;
        int generated = 1;
        int maxDepth = 0;

        while (open.Count > 0)
        {
            if (cancellationToken.IsCancellationRequested)
                return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            var current = open.Dequeue();
            if (bestCosts.TryGetValue(current.State, out var bestKnownCost) && current.Cost > bestKnownCost)
                continue;

            if (expanded >= maxExpandedStates)
                return SearchResult.ExpansionLimitReached(
                    new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            expanded++;
            NodeExpanded?.Invoke(current);
            maxDepth = Math.Max(maxDepth, current.Depth);

            ExtendedState extendedState = current.State.Expand();
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
                TransitionGenerated?.Invoke(transition);

                if (bestCosts.TryGetValue(nextState, out var bestExistingCost) && nextCost >= bestExistingCost)
                {
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                if (_deadEndDetector is not null)
                {
                    ExtendedState next = nextState.Expand();
                    if (!_goalCondition.IsSatisfied(next) && _deadEndDetector.IsDeadEnd(next, _goalCondition))
                    {
                        TransitionPruned?.Invoke(transition);
                        continue;
                    }
                }

                bestCosts[nextState] = nextCost;

                var nextNode = new SearchNode(nextState, action, current, nextCost, current.Depth + 1);
                generated++;
                NodeGenerated?.Invoke(nextNode);
                TransitionDiscovered?.Invoke(transition);
                open.Enqueue(nextNode, (nextCost, sequence++));
            }
        }

        stopwatch.Stop();
        return SearchResult.Failure(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
    }
}
