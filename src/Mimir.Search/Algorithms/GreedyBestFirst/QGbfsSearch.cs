using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using GroundAction = Mimir.Core.Grounding.Action;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;
using OpenEntry = (Mimir.Search.SearchNode Node, Mimir.Core.Grounding.ExtendedState State, double Priority, long Order);

namespace Mimir.Search.Algorithms.GreedyBestFirst;

internal sealed class QGbfsSearch : ISearchAlgorithm
{
    private readonly State _initialState;
    private readonly IDeadEndDetector? _deadEndDetector;
    private readonly GoalCondition _goal;
    private readonly IApplicableActionGenerator _generator;
    private readonly IQHeuristic _heuristic;
    private readonly int _batchTarget;
    private readonly bool _maximize;
    private readonly Func<int, bool>? _shouldStop;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;

    public QGbfsSearch(State initialState, GoalCondition goal,
        IQHeuristic heuristic, int batchTarget, bool maximize, Func<int, bool>? shouldStop, IDeadEndDetector? deadEndDetector)
    {
        if (batchTarget <= 0)
            throw new ArgumentOutOfRangeException(nameof(batchTarget));
        _initialState = initialState;
        _deadEndDetector = deadEndDetector;
        _goal = goal;
        _generator = initialState.Context.Problem.GetApplicableActionGenerator(initialState);
        _heuristic = heuristic;
        _batchTarget = batchTarget;
        _maximize = maximize;
        _shouldStop = shouldStop;
    }

    public SearchResult Search(CancellationToken cancellationToken = default, int? maxExpandedStates = null)
    {
        if (maxExpandedStates < 0)
            throw new ArgumentOutOfRangeException(nameof(maxExpandedStates));

        var stopwatch = Stopwatch.StartNew();
        var root = new SearchNode(_initialState);
        ExtendedState initial = _initialState.Expand();
        var reached = new HashSet<State> { _initialState };
        var closed = new HashSet<State>();
        var open = new SortedSet<OpenEntry>(Comparer<OpenEntry>.Create((left, right) =>
        {
            int priority = left.Priority.CompareTo(right.Priority);
            return priority != 0 ? priority : left.Order.CompareTo(right.Order);
        }));
        var entries = new Dictionary<State, OpenEntry>();
        open.Add((root, initial, 0, 0));
        entries.Add(_initialState, (root, initial, 0, 0));
        NodeGenerated?.Invoke(root);
        int expanded = 0;
        int generated = 0;
        int evaluated = 0;
        int nodesGenerated = 1;
        int maxDepth = 0;
        long sequence = 0;

        SearchResult Finish(SearchStatus status, SearchNode? node = null)
        {
            var values = new List<double?>();
            for (SearchNode? current = node; current?.Parent != null; current = current.Parent)
                values.Add(current.ActionValue);
            values.Reverse();
            IReadOnlyList<GroundAction> path = node?.ExtractPlan() ?? [];
            return new SearchResult(status, status == SearchStatus.Succeeded ? path : [],
                new SearchStatistics(expanded, nodesGenerated, stopwatch.Elapsed, maxDepth,
                    reached.Count, generated, evaluated))
            {
                PartialPlan = status == SearchStatus.Succeeded ? [] : path,
                ActionValues = values,
                EndState = node?.State,
            };
        }

        if (_goal.IsSatisfied(initial))
            return Finish(SearchStatus.Succeeded, root);

        if (cancellationToken.IsCancellationRequested)
            return Finish(SearchStatus.Canceled, root);
        if (_deadEndDetector?.IsDeadEnd(initial, _goal) ?? false)
            return Finish(SearchStatus.DeadEnd, root);

        while (open.Count > 0)
        {
            if (cancellationToken.IsCancellationRequested)
                return Finish(SearchStatus.Canceled, open.Min.Node);
            if (expanded >= maxExpandedStates)
                return Finish(SearchStatus.ExpansionLimitReached, open.Min.Node);

            var expansions = new List<(ExtendedState State, Successors Successors)>();
            var parents = new List<SearchNode>();
            int previouslyReached = reached.Count;
            SearchNode? lastExpanded = null;

            while (open.Count > 0 && reached.Count - previouslyReached < _batchTarget)
            {
                if (cancellationToken.IsCancellationRequested)
                    return Finish(SearchStatus.Canceled, open.Min.Node);
                if (expanded >= maxExpandedStates)
                    break;

                OpenEntry entry = open.Min;
                open.Remove(entry);
                SearchNode node = entry.Node;
                ExtendedState state = entry.State;
                entries.Remove(node.State);
                closed.Add(node.State);
                lastExpanded = node;
                expanded++;
                NodeExpanded?.Invoke(node);
                var successors = new List<(GroundAction Action, ExtendedState State)>();
                bool hasCandidate = false;
                foreach (GroundAction action in _generator.GetApplicableActions(state))
                {
                    if (cancellationToken.IsCancellationRequested)
                        return Finish(SearchStatus.Canceled, node);
                    ExtendedState successor = state.Apply(action).Expand();
                    successors.Add((action, successor));
                    reached.Add(successor.State);
                    generated++;
                    TransitionGenerated?.Invoke(new SearchTransition(node.State, action, action.Cost, successor.State));
                    if (_goal.IsSatisfied(successor))
                    {
                        var goalNode = new SearchNode(successor.State, action, node, node.Cost + action.Cost, node.Depth + 1);
                        nodesGenerated++;
                        maxDepth = Math.Max(maxDepth, goalNode.Depth);
                        NodeGenerated?.Invoke(goalNode);
                        TransitionDiscovered?.Invoke(new SearchTransition(node.State, action, action.Cost, successor.State));
                        return Finish(SearchStatus.Succeeded, goalNode);
                    }
                    if (!closed.Contains(successor.State))
                        hasCandidate = true;
                }
                if (hasCandidate)
                {
                    parents.Add(node);
                    expansions.Add((state, successors));
                }
                else
                {
                    foreach (var (action, successor) in successors)
                        TransitionPruned?.Invoke(new SearchTransition(node.State, action, action.Cost, successor.State));
                }
            }

            if (cancellationToken.IsCancellationRequested)
                return Finish(SearchStatus.Canceled, open.Count > 0 ? open.Min.Node : lastExpanded);

            if (expansions.Count > 0)
            {
                IReadOnlyList<QHeuristicEvaluation> evaluations = _heuristic.Evaluate(expansions, _goal);
                if (evaluations == null || evaluations.Count != expansions.Count)
                    throw new InvalidOperationException("Q-heuristic returned an incorrect number of rows.");
                for (int row = 0; row < expansions.Count; row++)
                {
                    Successors successors = expansions[row].Successors;
                    IReadOnlyList<double> values = evaluations[row].Values;
                    if (values == null || values.Count != successors.Count)
                        throw new InvalidOperationException("Q-heuristic score count must match the number of successors.");
                    for (int index = 0; index < values.Count; index++)
                        if (!double.IsFinite(values[index]))
                            throw new InvalidOperationException($"Q-heuristic score at row {row}, action {index} must be finite.");
                    evaluated += successors.Count;
                    SearchNode parent = parents[row];
                    for (int index = 0; index < successors.Count; index++)
                    {
                        var (action, successor) = successors[index];
                        if (closed.Contains(successor.State)
                            || (_deadEndDetector?.IsDeadEnd(successor, _goal) ?? false))
                        {
                            TransitionPruned?.Invoke(new SearchTransition(parent.State, action, action.Cost, successor.State));
                            continue;
                        }
                        double value = values[index];
                        double priority = _maximize ? -value : value;
                        bool replacing = entries.TryGetValue(successor.State, out OpenEntry previous);
                        if (replacing && priority >= previous.Priority)
                        {
                            TransitionPruned?.Invoke(new SearchTransition(parent.State, action, action.Cost, successor.State));
                            continue;
                        }
                        if (replacing)
                        {
                            open.Remove(previous);
                            SearchNode pruned = previous.Node;
                            TransitionPruned?.Invoke(new SearchTransition(pruned.Parent!.State, pruned.Action!, pruned.Action!.Cost, pruned.State));
                        }
                        var child = new SearchNode(successor.State, action, parent, parent.Cost + action.Cost, parent.Depth + 1)
                        {
                            ActionValue = value,
                        };
                        OpenEntry entry = (child, successor, priority, replacing ? previous.Order : ++sequence);
                        entries[successor.State] = entry;
                        open.Add(entry);
                        nodesGenerated++;
                        maxDepth = Math.Max(maxDepth, child.Depth);
                        NodeGenerated?.Invoke(child);
                        TransitionDiscovered?.Invoke(new SearchTransition(parent.State, action, action.Cost, child.State));
                    }
                }
            }

            SearchNode? partial = open.Count > 0 ? open.Min.Node : lastExpanded;
            if (cancellationToken.IsCancellationRequested || (_shouldStop?.Invoke(expanded) ?? false))
                return Finish(SearchStatus.Canceled, partial);
        }

        return Finish(SearchStatus.Failed);
    }
}
