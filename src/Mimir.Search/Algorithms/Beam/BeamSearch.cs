using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using GroundAction = Mimir.Core.Grounding.Action;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;
using Expansion = (Mimir.Core.Grounding.ExtendedState State, System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)> Successors);
using Candidate = (Mimir.Search.SearchNode Parent, Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State, double Value, double Priority, int Order);
using StateCandidate = (Mimir.Search.SearchNode Parent, Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State, int Order);

namespace Mimir.Search.Algorithms.Beam;

internal sealed class BeamSearch : ISearchAlgorithm
{
    private readonly State _initialState;
    private readonly IDeadEndDetector? _deadEndDetector;
    private readonly GoalCondition _goal;
    private readonly IApplicableActionGenerator _generator;
    private readonly IHeuristic? _heuristic;
    private readonly IQHeuristic? _qHeuristic;
    private readonly int _beamSize;
    private readonly int _maxDepth;
    private readonly bool _maximize;
    private readonly Func<int, bool>? _shouldStop;

    public event Action<SearchNode>? NodeExpanded;
    public event Action<SearchNode>? NodeGenerated;
    public event Action<SearchTransition>? TransitionGenerated;
    public event Action<SearchTransition>? TransitionDiscovered;
    public event Action<SearchTransition>? TransitionPruned;

    public BeamSearch(State initialState, GoalCondition goal,
        IHeuristic? heuristic, IQHeuristic? qHeuristic, int beamSize, int maxDepth,
        bool maximize, Func<int, bool>? shouldStop, IDeadEndDetector? deadEndDetector)
    {
        if ((heuristic == null) == (qHeuristic == null))
            throw new ArgumentException("Exactly one heuristic must be provided.");
        if (beamSize <= 0) throw new ArgumentOutOfRangeException(nameof(beamSize));
        if (maxDepth < 0) throw new ArgumentOutOfRangeException(nameof(maxDepth));
        _initialState = initialState;
        _deadEndDetector = deadEndDetector;
        _goal = goal;
        _generator = initialState.Context.Problem.GetApplicableActionGenerator(initialState);
        _heuristic = heuristic is null ? null : HeuristicBinding.Bind(heuristic, goal, _generator);
        _qHeuristic = qHeuristic;
        _beamSize = beamSize;
        _maxDepth = maxDepth;
        _maximize = maximize;
        _shouldStop = shouldStop;
    }

    public SearchResult Search(CancellationToken cancellationToken = default, int? maxExpandedStates = null)
    {
        if (maxExpandedStates < 0) throw new ArgumentOutOfRangeException(nameof(maxExpandedStates));
        var stopwatch = Stopwatch.StartNew();
        var root = new SearchNode(_initialState);
        var beam = new List<(SearchNode Node, ExtendedState State)> { (root, _initialState.Expand()) };
        // Width-pruned candidates remain eligible to be reached at a later depth.
        var closed = new HashSet<State> { _initialState };
        int expanded = 0;
        int generated = 0;
        int evaluated = 0;
        int maxDepth = 0;
        NodeGenerated?.Invoke(root);

        SearchResult Finish(SearchStatus status, SearchNode node)
        {
            var values = new List<double?>();
            for (SearchNode? current = node; current?.Parent != null; current = current.Parent)
                values.Add(current.ActionValue);
            values.Reverse();
            IReadOnlyList<GroundAction> path = node.ExtractPlan();
            return new SearchResult(status, status == SearchStatus.Succeeded ? path : [],
                new SearchStatistics(expanded, closed.Count, stopwatch.Elapsed, maxDepth,
                    closed.Count, generated, evaluated))
            {
                PartialPlan = status == SearchStatus.Succeeded ? [] : path,
                ActionValues = values,
                EndState = node.State,
            };
        }

        if (_goal.IsSatisfied(beam[0].State)) return Finish(SearchStatus.Succeeded, root);

        if (cancellationToken.IsCancellationRequested)
            return Finish(SearchStatus.Canceled, root);
        if (_deadEndDetector?.IsDeadEnd(beam[0].State, _goal) ?? false)
            return Finish(SearchStatus.DeadEnd, root);

        while (true)
        {
            SearchNode partial = beam[0].Node;
            if (cancellationToken.IsCancellationRequested) return Finish(SearchStatus.Canceled, partial);
            if (partial.Depth >= _maxDepth) return Finish(SearchStatus.DepthLimitReached, partial);
            if (expanded >= maxExpandedStates) return Finish(SearchStatus.ExpansionLimitReached, partial);

            var expansions = new List<Expansion>();
            var parents = new List<SearchNode>();
            var stateCandidates = new Dictionary<State, StateCandidate>();
            bool sawAction = false;
            bool truncatedLayer = false;
            foreach (var (node, state) in beam)
            {
                if (cancellationToken.IsCancellationRequested) return Finish(SearchStatus.Canceled, partial);
                if (expanded >= maxExpandedStates)
                {
                    truncatedLayer = true;
                    break;
                }
                expanded++;
                NodeExpanded?.Invoke(node);
                var successors = _qHeuristic == null ? null : new List<(GroundAction Action, ExtendedState State)>();
                bool hasCandidate = false;
                foreach (GroundAction action in _generator.GetApplicableActions(state))
                {
                    if (cancellationToken.IsCancellationRequested) return Finish(SearchStatus.Canceled, partial);
                    ExtendedState successor = state.Apply(action).Expand();
                    successors?.Add((action, successor));
                    generated++;
                    sawAction = true;
                    TransitionGenerated?.Invoke(new SearchTransition(node.State, action, action.Cost, successor.State));
                    if (_goal.IsSatisfied(successor))
                    {
                        var goalNode = new SearchNode(successor.State, action, node, node.Cost + action.Cost, node.Depth + 1);
                        closed.Add(successor.State);
                        maxDepth = goalNode.Depth;
                        NodeGenerated?.Invoke(goalNode);
                        TransitionDiscovered?.Invoke(new SearchTransition(node.State, action, action.Cost, successor.State));
                        return Finish(SearchStatus.Succeeded, goalNode);
                    }
                    if (_qHeuristic != null)
                    {
                        if (!closed.Contains(successor.State)) hasCandidate = true;
                    }
                    else if (closed.Contains(successor.State) || !stateCandidates.TryAdd(successor.State,
                        (node, action, successor, stateCandidates.Count)))
                    {
                        TransitionPruned?.Invoke(new SearchTransition(node.State, action, action.Cost, successor.State));
                    }
                }
                if (successors == null) continue;
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

            if (cancellationToken.IsCancellationRequested) return Finish(SearchStatus.Canceled, partial);
            if (expansions.Count == 0 && stateCandidates.Count == 0)
            {
                if (truncatedLayer) return Finish(SearchStatus.ExpansionLimitReached, partial);
                return Finish(sawAction ? SearchStatus.Failed : SearchStatus.DeadEnd, partial);
            }

            List<Candidate> candidates;
            if (_qHeuristic != null)
            {
                candidates = EvaluateQ(expansions, parents, closed);
                evaluated += expansions.Sum(expansion => expansion.Successors.Count);
            }
            else
            {
                candidates = EvaluateStates(stateCandidates.Values.ToArray());
                evaluated += stateCandidates.Count;
            }
            candidates.Sort((left, right) =>
            {
                int priority = left.Priority.CompareTo(right.Priority);
                return priority != 0 ? priority : left.Order.CompareTo(right.Order);
            });
            if (candidates.Count > 0)
            {
                beam.Clear();
                for (int index = 0; index < candidates.Count; index++)
                {
                    var (parent, action, state, value, _, _) = candidates[index];
                    if (index >= _beamSize)
                    {
                        TransitionPruned?.Invoke(new SearchTransition(parent.State, action, action.Cost, state.State));
                        continue;
                    }
                    var child = new SearchNode(state.State, action, parent, parent.Cost + action.Cost, parent.Depth + 1)
                    {
                        ActionValue = value,
                    };
                    beam.Add((child, state));
                    closed.Add(state.State);
                    maxDepth = child.Depth;
                    NodeGenerated?.Invoke(child);
                    TransitionDiscovered?.Invoke(new SearchTransition(parent.State, action, action.Cost, state.State));
                }
                partial = beam[0].Node;
            }
            if (cancellationToken.IsCancellationRequested || (_shouldStop?.Invoke(expanded) ?? false))
                return Finish(SearchStatus.Canceled, partial);
            if (candidates.Count == 0)
                return Finish(truncatedLayer ? SearchStatus.ExpansionLimitReached : SearchStatus.DeadEnd, partial);
            if (expanded >= maxExpandedStates) return Finish(SearchStatus.ExpansionLimitReached, partial);
        }
    }

    private List<Candidate> EvaluateQ(IReadOnlyList<Expansion> expansions,
        IReadOnlyList<SearchNode> parents, HashSet<State> closed)
    {
        IReadOnlyList<QHeuristicEvaluation> evaluations = _qHeuristic!.Evaluate(expansions, _goal);
        if (evaluations == null || evaluations.Count != expansions.Count)
            throw new InvalidOperationException("Q-heuristic returned an incorrect number of rows.");
        var bestByState = new Dictionary<State, Candidate>();
        int order = 0;
        for (int row = 0; row < expansions.Count; row++)
        {
            Successors successors = expansions[row].Successors;
            IReadOnlyList<double> values = evaluations[row].Values;
            if (values == null || values.Count != successors.Count)
                throw new InvalidOperationException("Q-heuristic score count must match the number of successors.");
            SearchNode parent = parents[row];
            for (int index = 0; index < successors.Count; index++)
            {
                var (action, successor) = successors[index];
                double value = values[index];
                if (!double.IsFinite(value))
                    throw new InvalidOperationException($"Q-heuristic score at row {row}, action {index} must be finite.");
                double priority = _maximize ? -value : value;
                int candidateOrder = order++;
                bool replacing = bestByState.TryGetValue(successor.State, out Candidate previous);
                if (closed.Contains(successor.State)
                    || (_deadEndDetector?.IsDeadEnd(successor, _goal) ?? false)
                    || (replacing && priority >= previous.Priority))
                {
                    TransitionPruned?.Invoke(new SearchTransition(parent.State, action, action.Cost, successor.State));
                    continue;
                }
                if (replacing)
                    TransitionPruned?.Invoke(new SearchTransition(previous.Parent.State, previous.Action,
                        previous.Action.Cost, previous.State.State));
                bestByState[successor.State] = (parent, action, successor, value, priority, candidateOrder);
            }
        }
        return bestByState.Values.ToList();
    }

    private List<Candidate> EvaluateStates(StateCandidate[] candidates)
    {
        ExtendedState[] states = candidates.Select(candidate => candidate.State).ToArray();
        IReadOnlyList<HeuristicEvaluation> evaluations = _heuristic!.Evaluate(states, _goal);
        if (evaluations == null || evaluations.Count != candidates.Length)
            throw new InvalidOperationException("Heuristic score count must match the number of states.");
        var scored = new List<Candidate>(candidates.Length);
        for (int index = 0; index < candidates.Length; index++)
        {
            var (parent, action, state, order) = candidates[index];
            double value = evaluations[index].Value;
            if (double.IsNaN(value) || value < 0)
                throw new InvalidOperationException("Heuristic values must be non-negative and not NaN.");
            if (double.IsPositiveInfinity(value) || (_deadEndDetector?.IsDeadEnd(state, _goal) ?? false))
            {
                TransitionPruned?.Invoke(new SearchTransition(parent.State, action, action.Cost, state.State));
                continue;
            }
            scored.Add((parent, action, state, value, value, order));
        }
        return scored;
    }
}
