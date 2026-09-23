using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.GreedyBestFirst;

internal sealed class GbfsSearch : ISearchAlgorithm
{
    private readonly State _initialState;
    private readonly IDeadEndDetector? _deadEndDetector;
    private readonly GoalCondition _goalCondition;
    private readonly IApplicableActionGenerator _actionGenerator;
    private readonly IHeuristic _heuristic;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchNode>? GoalNodeExpanded;
    public event Action<SearchTransition>? StateGenerated;
    public event Action<State>? StatePruned;
    public event Action<double>? NewBestHValue;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;

    public GbfsSearch(
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
            (SearchNode Node, ExtendedState ExtendedState, HeuristicEvaluation Evaluation),
            (int PreferredRank, double HeuristicValue, double PathCost, long Sequence)>();
        var reached = new HashSet<State>();

        long sequence = 0;
        var root = new SearchNode(_initialState);
        ExtendedState extendedInitialState = _initialState.Expand();
        HeuristicEvaluation rootEvaluation = _goalCondition.IsSatisfied(extendedInitialState)
            ? new HeuristicEvaluation(0d)
            : EvaluateHeuristic(extendedInitialState);

        double bestHValue = rootEvaluation.Value;
        open.Enqueue(
            (root, extendedInitialState, rootEvaluation),
            (1, rootEvaluation.Value, root.Cost, sequence++));
        reached.Add(_initialState);
        NodeGenerated?.Invoke(root);

        int expanded = 0;
        int generated = 1;
        int maxDepth = 0;

        while (open.Count > 0)
        {
            if (cancellationToken.IsCancellationRequested)
                return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            var currentEntry = open.Dequeue();
            SearchNode current = currentEntry.Node;
            HeuristicEvaluation currentEvaluation = currentEntry.Evaluation;

            if (expanded >= maxExpandedStates)
                return SearchResult.ExpansionLimitReached(
                    new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            expanded++;
            NodeExpanded?.Invoke(current);
            maxDepth = Math.Max(maxDepth, current.Depth);

            ExtendedState extendedState = currentEntry.ExtendedState;
            if (_goalCondition.IsSatisfied(extendedState))
            {
                GoalNodeExpanded?.Invoke(current);
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

                bool isPreferred = currentEvaluation.IsPreferredAction?.Invoke(action) ?? false;
                if (!reached.Add(nextState))
                {
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                var nextNode = new SearchNode(nextState, action, current, nextCost, current.Depth + 1);
                ExtendedState extendedNextState = nextState.Expand();
                bool successorIsGoal = _goalCondition.IsSatisfied(extendedNextState);
                HeuristicEvaluation nextEvaluation = EvaluateHeuristic(extendedNextState);

                if (!successorIsGoal && (double.IsPositiveInfinity(nextEvaluation.Value)
                    || (_deadEndDetector?.IsDeadEnd(extendedNextState, _goalCondition) ?? false)))
                {
                    StatePruned?.Invoke(nextState);
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                if (successorIsGoal)
                {
                    generated++;
                    NodeGenerated?.Invoke(nextNode);
                    TransitionDiscovered?.Invoke(transition);
                    maxDepth = Math.Max(maxDepth, nextNode.Depth);
                    open.Enqueue(
                        (nextNode, extendedNextState, nextEvaluation),
                        (isPreferred ? 0 : 1, nextEvaluation.Value, nextCost, sequence++));
                    continue;
                }

                if (nextEvaluation.Value < bestHValue)
                {
                    bestHValue = nextEvaluation.Value;
                    NewBestHValue?.Invoke(bestHValue);
                }

                generated++;
                NodeGenerated?.Invoke(nextNode);
                TransitionDiscovered?.Invoke(transition);
                open.Enqueue(
                    (nextNode, extendedNextState, nextEvaluation),
                    (isPreferred ? 0 : 1, nextEvaluation.Value, nextCost, sequence++));
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
