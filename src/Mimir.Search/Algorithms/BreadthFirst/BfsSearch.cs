using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.BreadthFirst;

internal sealed class BfsSearch : ISearchAlgorithm
{
    private readonly State _initialState;
    private readonly GoalCondition _goalCondition;
    private readonly IApplicableActionGenerator _actionGenerator;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchTransition>? StateGenerated;
    public event Action<SearchTransition>? StateGeneratedInSearchTree;
    public event Action<SearchTransition>? StateGeneratedNotInSearchTree;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;
    public event Action<double>? GLayerFinished;

    public BfsSearch(
        State initialState,
        GoalCondition goalCondition,
        IApplicableActionGenerator actionGenerator)
    {
        _initialState = initialState;
        _goalCondition = goalCondition;
        _actionGenerator = actionGenerator;
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

        var open = new Queue<SearchNode>();
        var closed = new HashSet<State>();
        // A subclass can reimplement the public generator interface, so bypass
        // interface dispatch only for the concrete implementation we own.
        GroundedApplicableActionGenerator? groundedGenerator =
            _actionGenerator is GroundedApplicableActionGenerator candidate
            && candidate.GetType() == typeof(GroundedApplicableActionGenerator)
                ? candidate
                : null;
        CliqueApplicableActionGenerator? cliqueGenerator =
            _actionGenerator is CliqueApplicableActionGenerator cliqueCandidate
            && cliqueCandidate.GetType() == typeof(CliqueApplicableActionGenerator)
                ? cliqueCandidate
                : null;
        List<Action>? reusableActions =
            groundedGenerator is null && cliqueGenerator is null
            ? null
            : new List<Action>();

        var root = new SearchNode(_initialState);
        open.Enqueue(root);
        closed.Add(_initialState);
        NodeGenerated?.Invoke(root);

        int expanded = 0;
        int generated = 1;
        int maxDepth = 0;
        int currentGValue = 0;
        GLayerFinished?.Invoke(currentGValue);

        while (open.Count > 0)
        {
            if (cancellationToken.IsCancellationRequested)
                return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            if (expanded >= maxExpandedStates)
                return SearchResult.ExpansionLimitReached(
                    new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

            var current = open.Dequeue();
            if (current.Depth > currentGValue)
            {
                GLayerFinished?.Invoke(currentGValue);
                currentGValue = current.Depth;
            }

            expanded++;
            NodeExpanded?.Invoke(current);
            maxDepth = Math.Max(maxDepth, current.Depth);

            ExtendedState extendedState = current.State.Expand();
            if (_goalCondition.IsSatisfied(extendedState))
            {
                stopwatch.Stop();
                return SearchResult.Success(current.ExtractPlan(), new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
            }

            if (reusableActions is not null)
            {
                if (groundedGenerator is not null)
                    groundedGenerator.CollectApplicableActions(extendedState, reusableActions);
                else
                    cliqueGenerator!.CollectApplicableActions(extendedState, reusableActions);

                List<Action> actions = reusableActions;
                for (int actionIndex = 0; actionIndex < actions.Count; actionIndex++)
                {
                    if (cancellationToken.IsCancellationRequested)
                        return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

                    Action action = actions[actionIndex];
                    var nextState = extendedState.Apply(action);
                    var transition = new SearchTransition(current.State, action, action.Cost, nextState);
                    StateGenerated?.Invoke(transition);
                    TransitionGenerated?.Invoke(transition);
                    if (closed.Add(nextState))
                    {
                        var nextNode = new SearchNode(nextState, action, current, current.Cost + action.Cost, current.Depth + 1);
                        generated++;
                        NodeGenerated?.Invoke(nextNode);
                        StateGeneratedInSearchTree?.Invoke(transition);
                        TransitionDiscovered?.Invoke(transition);
                        open.Enqueue(nextNode);
                        continue;
                    }

                    StateGeneratedNotInSearchTree?.Invoke(transition);
                    TransitionPruned?.Invoke(transition);
                }

                continue;
            }

            foreach (var action in _actionGenerator.GetApplicableActions(extendedState))
            {
                if (cancellationToken.IsCancellationRequested)
                    return SearchResult.Canceled(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));

                var nextState = extendedState.Apply(action);
                var transition = new SearchTransition(current.State, action, action.Cost, nextState);
                StateGenerated?.Invoke(transition);
                TransitionGenerated?.Invoke(transition);
                if (closed.Add(nextState))
                {
                    var nextNode = new SearchNode(nextState, action, current, current.Cost + action.Cost, current.Depth + 1);
                    generated++;
                    NodeGenerated?.Invoke(nextNode);
                    StateGeneratedInSearchTree?.Invoke(transition);
                    TransitionDiscovered?.Invoke(transition);
                    open.Enqueue(nextNode);
                    continue;
                }

                StateGeneratedNotInSearchTree?.Invoke(transition);
                TransitionPruned?.Invoke(transition);
            }
        }

        stopwatch.Stop();
        return SearchResult.Failure(new SearchStatistics(expanded, generated, stopwatch.Elapsed, maxDepth));
    }
}
