using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.IteratedWidth;

internal sealed class IwSearch : ISearchAlgorithm
{
    private readonly int _k;
    private readonly State _initialState;
    private readonly IDeadEndDetector? _deadEndDetector;
    private readonly GoalCondition _goalCondition;
    // What this search is looking for. Serialization wants the first state with fewer goals left to achieve,
    // which is not a condition over atoms, so it is supplied rather than derived from the goal.
    private readonly Func<ExtendedState, bool> _isTarget;
    private readonly IApplicableActionGenerator _actionGenerator;
    private readonly FactoredExpansion? _factoredExpansion;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;
    // Fires at the start of each sub-search with the current novelty width (0, 1, 2, …).
    public event Action<int>? WidthStarted;

    public IwSearch(
        int k,
        State initialState,
        GoalCondition goalCondition,
        IDeadEndDetector? deadEndDetector)
        : this(k, initialState, goalCondition, deadEndDetector, target: null)
    {
    }

    public IwSearch(
        int k,
        State initialState,
        GoalCondition goalCondition,
        IDeadEndDetector? deadEndDetector,
        Func<ExtendedState, bool>? target)
    {
        if (k is < 0 or > 3)
            throw new ArgumentOutOfRangeException(nameof(k), "IW(k) supports k = 0..3.");
        // Novelty only tracks atoms, so states that differ only in numeric values would be pruned as not novel.
        if (initialState.Context.NumericStateSize > 0)
            throw new NotSupportedException("IW does not support problems with changing numeric fluents.");

        _k = k;
        _initialState = initialState;
        _deadEndDetector = deadEndDetector;
        _goalCondition = goalCondition;
        _isTarget = target ?? goalCondition.IsSatisfied;
        _actionGenerator = initialState.Context.Problem.GetApplicableActionGenerator(initialState);

        // IW(k) runs a width-1 pass on its way to k, and that pass can use the shortcut whatever k is.
        if (_actionGenerator is IFactoredApplicableActionGenerator factored)
            _factoredExpansion = FactoredExpansion.TryCreate(factored, goalCondition, initialState.Expand());
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

        int totalExpanded = 0;
        int totalGenerated = 0;
        int totalMaxDepth = 0;

        for (int width = 0; width <= _k; width++)
        {
            if (cancellationToken.IsCancellationRequested)
            {
                stopwatch.Stop();
                return SearchResult.Canceled(
                    new SearchStatistics(totalExpanded, totalGenerated, stopwatch.Elapsed, totalMaxDepth));
            }

            WidthStarted?.Invoke(width);
            int? remainingExpansions = maxExpandedStates - totalExpanded;
            var sub = width == 0
                ? SearchK0(cancellationToken, remainingExpansions)
                : SearchKN(width, cancellationToken, remainingExpansions);

            totalExpanded  += sub.Statistics.NodesExpanded;
            totalGenerated += sub.Statistics.NodesGenerated;
            totalMaxDepth   = Math.Max(totalMaxDepth, sub.Statistics.MaxDepth);

            if (sub.IsSuccess)
            {
                stopwatch.Stop();
                return SearchResult.Success(sub.Plan, new SearchStatistics(totalExpanded, totalGenerated, stopwatch.Elapsed, totalMaxDepth));
            }

            if (sub.IsCanceled)
            {
                stopwatch.Stop();
                return SearchResult.Canceled(
                    new SearchStatistics(totalExpanded, totalGenerated, stopwatch.Elapsed, totalMaxDepth));
            }

            if (sub.IsExpansionLimitReached)
            {
                stopwatch.Stop();
                return SearchResult.ExpansionLimitReached(
                    new SearchStatistics(totalExpanded, totalGenerated, stopwatch.Elapsed, totalMaxDepth));
            }
        }

        stopwatch.Stop();
        return SearchResult.Failure(new SearchStatistics(totalExpanded, totalGenerated, stopwatch.Elapsed, totalMaxDepth));
    }

    // IW(0): expand the root once and test each depth-1 successor for the goal.
    private SearchResult SearchK0(
        CancellationToken cancellationToken,
        int? maxExpandedStates)
    {
        var stopwatch = Stopwatch.StartNew();
        if (cancellationToken.IsCancellationRequested)
        {
            stopwatch.Stop();
            return SearchResult.Canceled(new SearchStatistics(0, 0, stopwatch.Elapsed, 0));
        }

        var root = new SearchNode(_initialState);
        NodeGenerated?.Invoke(root);
        int generated = 1;
        ExtendedState extendedInitialState = _initialState.Expand();

        if (_isTarget(extendedInitialState))
        {
            stopwatch.Stop();
            return SearchResult.Success([], new SearchStatistics(0, generated, stopwatch.Elapsed, 0));
        }

        if (maxExpandedStates == 0)
            return SearchResult.ExpansionLimitReached(
                new SearchStatistics(0, generated, stopwatch.Elapsed, 0));

        NodeExpanded?.Invoke(root);

        foreach (var action in _actionGenerator.GetApplicableActions(extendedInitialState))
        {
            if (cancellationToken.IsCancellationRequested)
            {
                stopwatch.Stop();
                return SearchResult.Canceled(new SearchStatistics(1, generated, stopwatch.Elapsed, 0));
            }

            var nextState = extendedInitialState.Apply(action);
            var transition = new SearchTransition(_initialState, action, action.Cost, nextState);
            TransitionGenerated?.Invoke(transition);
            ExtendedState next = nextState.Expand();
            // Dead ends are judged against the real goal even when the search is aiming at something nearer.
            if (!_goalCondition.IsSatisfied(next) && (_deadEndDetector?.IsDeadEnd(next, _goalCondition) ?? false))
            {
                TransitionPruned?.Invoke(transition);
                continue;
            }
            var child = new SearchNode(nextState, action, root, action.Cost, 1);
            generated++;
            NodeGenerated?.Invoke(child);
            TransitionDiscovered?.Invoke(transition);

            if (_isTarget(nextState.Expand()))
            {
                stopwatch.Stop();
                return SearchResult.Success(child.ExtractPlan(), new SearchStatistics(1, generated, stopwatch.Elapsed, 0));
            }
        }

        stopwatch.Stop();
        return SearchResult.Failure(new SearchStatistics(1, generated, stopwatch.Elapsed, 0));
    }

    // Width 1 is the only case where a group's members collapse to a projection without losing a successor the
    // search would have kept: above it, novelty also turns on which atoms an action deletes, and members that
    // agree on every effect parameter are the same successor and were collapsed by the generator already.
    private IEnumerable<Action> CandidateActions(ExtendedState state, int width, NoveltyTracker tracker)
    {
        return _factoredExpansion is null
            ? _actionGenerator.GetApplicableActions(state)
            : _factoredExpansion.CandidateActions(state, width, tracker);
    }

    // BFS pruned at the given novelty width (width ≥ 1).
    private SearchResult SearchKN(
        int width,
        CancellationToken cancellationToken,
        int? maxExpandedStates)
    {
        var stopwatch = Stopwatch.StartNew();
        if (cancellationToken.IsCancellationRequested)
        {
            stopwatch.Stop();
            return SearchResult.Canceled(new SearchStatistics(0, 0, stopwatch.Elapsed, 0));
        }

        var tracker = new NoveltyTracker(width, _initialState.Context);
        var open = new Queue<(SearchNode Node, ExtendedState ExtendedState)>();
        var closed = new HashSet<State>();

        var root = new SearchNode(_initialState);
        tracker.IsNovel(root.State);
        closed.Add(_initialState);
        NodeGenerated?.Invoke(root);

        int expanded = 0;
        int generated = 1;
        int maxDepth = 0;

        if (maxExpandedStates == 0)
        {
            stopwatch.Stop();
            return SearchResult.ExpansionLimitReached(
                new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
        }

        ExtendedState extendedInitialState = _initialState.Expand();
        open.Enqueue((root, extendedInitialState));

        while (open.Count > 0)
        {
            if (cancellationToken.IsCancellationRequested)
            {
                stopwatch.Stop();
                return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
            }

            var currentEntry = open.Dequeue();
            SearchNode current = currentEntry.Node;
            if (expanded >= maxExpandedStates)
            {
                stopwatch.Stop();
                return SearchResult.ExpansionLimitReached(
                    new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
            }

            expanded++;
            NodeExpanded?.Invoke(current);
            maxDepth = Math.Max(maxDepth, current.Depth);

            ExtendedState extendedState = currentEntry.ExtendedState;
            if (_isTarget(extendedState))
            {
                stopwatch.Stop();
                return SearchResult.Success(current.ExtractPlan(), new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
            }

            foreach (var action in CandidateActions(extendedState, width, tracker))
            {
                if (cancellationToken.IsCancellationRequested)
                {
                    stopwatch.Stop();
                    return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
                }

                var nextState = extendedState.Apply(action);
                var transition = new SearchTransition(current.State, action, action.Cost, nextState);
                TransitionGenerated?.Invoke(transition);
                if (!closed.Add(nextState))
                {
                    TransitionPruned?.Invoke(transition);
                    continue;
                }

                var child = new SearchNode(nextState, action, current, current.Cost + action.Cost, current.Depth + 1);
                generated++;
                NodeGenerated?.Invoke(child);

                ExtendedState extendedNextState = nextState.Expand();
                bool isGoal = _isTarget(extendedNextState);
                if (!isGoal && (_deadEndDetector?.IsDeadEnd(extendedNextState, _goalCondition) ?? false))
                {
                    TransitionPruned?.Invoke(transition);
                    continue;
                }
                bool isNovel = tracker.IsNovel(child.State, current.State);
                if (isGoal || isNovel)
                {
                    TransitionDiscovered?.Invoke(transition);
                    open.Enqueue((child, extendedNextState));
                }
                else
                {
                    TransitionPruned?.Invoke(transition);
                }
            }
        }

        stopwatch.Stop();
        return SearchResult.Failure(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
    }
}
