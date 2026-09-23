using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using System.Diagnostics;

namespace Mimir.Search.Space;

/// <summary>
/// A fully-expanded search space that explores all reachable states from the initial state.
/// Provides efficient queries for supervised learning including distance-to-goal, cost-to-goal,
/// dead-end detection, and structural statistics.
/// </summary>
public class SearchSpace
{
    private readonly State _initialState;
    private readonly GoalCondition _goalCondition;
    private readonly IApplicableActionGenerator _actionGenerator;
    private readonly int? _maxStates;

    // Core storage: state -> node mapping using StateEqualityComparer for proper equality
    private readonly Dictionary<State, SearchSpaceNode> _stateToNode;

    // Ordered list of all nodes (for iteration)
    private readonly List<SearchSpaceNode> _nodes;

    private readonly IReadOnlyList<State> _allStates;
    private readonly IReadOnlyList<State> _goalStates;
    private readonly IReadOnlyList<State> _deadEndStates;
    private readonly int _maxDistanceToGoal;

    // Computed metadata
    private readonly SearchSpaceStatistics _statistics;
    private readonly SearchSpaceQuery _query;

    // Properties exposed to query
    public State InitialState => _initialState;
    internal GoalCondition Goal => _goalCondition;

    /// <summary>
    /// Returns all states in the search space in BFS order.
    /// </summary>
    public IReadOnlyList<State> AllStates => _allStates;

    /// <summary>
    /// Returns the total number of states in the search space.
    /// </summary>
    public int TotalStates => _statistics.TotalStates;

    /// <summary>
    /// Returns all goal states in the search space.
    /// </summary>
    public IReadOnlyList<State> GoalStates => _goalStates;

    /// <summary>
    /// Returns all dead-end states in the search space.
    /// </summary>
    public IReadOnlyList<State> DeadEndStates => _deadEndStates;

    /// <summary>
    /// Returns the number of goal states.
    /// </summary>
    public int GoalStateCount => _statistics.GoalStateCount;

    /// <summary>
    /// Returns the number of dead-end states.
    /// </summary>
    public int DeadEndStateCount => _statistics.DeadEndStateCount;

    /// <summary>
    /// Returns the maximum BFS depth from the initial state.
    /// </summary>
    public int MaxDepth => _statistics.MaxDepth;

    /// <summary>
    /// Returns the maximum non-dead-end distance to a goal state.
    /// Returns -1 if there are no goal states.
    /// </summary>
    public int MaxDistanceToGoal => _maxDistanceToGoal;

    /// <summary>
    /// Returns the average branching factor (average number of applicable actions per state).
    /// </summary>
    public double AverageBranchingFactor => _statistics.AverageBranchingFactor;

    /// <summary>
    /// Returns the total number of transitions (edges) in the search space.
    /// </summary>
    public int TotalTransitions => _statistics.TotalTransitions;

    /// <summary>
    /// Returns the read-only query interface for supervised learning access.
    /// </summary>
    public SearchSpaceQuery Query => _query;

    /// <summary>
    /// Returns statistics about the search space structure.
    /// </summary>
    public SearchSpaceStatistics Statistics => _statistics;

    internal SearchSpace(
        State initialState,
        GoalCondition goalCondition,
        int? maxStates,
        CancellationToken cancellationToken)
    {
        _initialState = initialState;
        _goalCondition = goalCondition;
        _actionGenerator = initialState.Context.Problem.GetApplicableActionGenerator(initialState);
        _maxStates = maxStates;
        _stateToNode = new Dictionary<State, SearchSpaceNode>(StateEqualityComparer.Instance);
        _nodes = new List<SearchSpaceNode>();
        _query = new SearchSpaceQuery(this);

        var stopwatch = Stopwatch.StartNew();
        cancellationToken.ThrowIfCancellationRequested();
        Expand(cancellationToken);
        stopwatch.Stop();
        (_allStates, _goalStates, _deadEndStates, _maxDistanceToGoal) = Freeze(cancellationToken);
        _statistics = ComputeStatistics(stopwatch.Elapsed, cancellationToken);
    }

    /// <summary>
    /// Fully expands the search space from the initial state.
    /// Uses BFS for correct shortest-path distances, then analyzes for dead-ends and goal distances.
    /// </summary>
    private void Expand(CancellationToken cancellationToken)
    {
        // Phase 1: Forward BFS expansion from initial state
        ExpandForward(cancellationToken);

        ComputeCostFromRoot(cancellationToken);

        ComputeDistanceToGoal(cancellationToken);

        ComputeCostToGoal(cancellationToken);
    }

    /// <summary>
    /// Phase 1: BFS expansion from the initial state, building the full transition graph.
    /// </summary>
    private void ExpandForward(CancellationToken cancellationToken)
    {
        var queue = new Queue<State>();

        // Create root node
        var rootNode = CreateNode(_initialState, 0);
        _stateToNode[_initialState] = rootNode;
        queue.Enqueue(_initialState);

        while (queue.Count > 0)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var currentState = queue.Dequeue();

            SearchSpaceNode currentNode = _stateToNode[currentState];
            ExtendedState extendedState = currentState.Expand();
            currentNode.IsGoal = _goalCondition.IsSatisfied(extendedState);

            // Generate all applicable actions
            var applicableActions = _actionGenerator.GetApplicableActions(extendedState).ToList();
            currentNode.ApplicableActions.AddRange(applicableActions);

            // Explore successors
            foreach (var action in applicableActions)
            {
                cancellationToken.ThrowIfCancellationRequested();
                var nextState = extendedState.Apply(action);
                int nextDepth = currentNode.Depth + 1;

                if (_stateToNode.TryGetValue(nextState, out var existingNode))
                {
                    currentNode.Children.Add((existingNode, action));
                    existingNode.Parents.Add((currentNode, action));
                }
                else
                {
                    if (_maxStates.HasValue && _nodes.Count >= _maxStates.Value)
                    {
                        throw new StateSpaceLimitExceededException(_maxStates.Value);
                    }

                    var newNode = CreateNode(nextState, nextDepth);

                    currentNode.Children.Add((newNode, action));
                    newNode.Parents.Add((currentNode, action));

                    _stateToNode[nextState] = newNode;
                    queue.Enqueue(nextState);
                }

                // Record the successor relationship
                currentNode.Successors.Add((nextState, action));
            }
        }
    }

    private void ComputeCostFromRoot(CancellationToken cancellationToken)
    {
        foreach (SearchSpaceNode node in _nodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            node.CostFromRoot = double.PositiveInfinity;
        }

        SearchSpaceNode root = _stateToNode[_initialState];
        root.CostFromRoot = 0.0;

        var priorityQueue = new PriorityQueue<SearchSpaceNode, double>();
        priorityQueue.Enqueue(root, 0.0);

        while (priorityQueue.TryDequeue(out SearchSpaceNode? currentNode, out double queuedCost))
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (queuedCost > currentNode.CostFromRoot)
                continue;

            foreach (var (child, action) in currentNode.Children)
            {
                cancellationToken.ThrowIfCancellationRequested();
                double newCost = queuedCost + action.Cost;
                if (newCost >= child.CostFromRoot)
                    continue;

                child.CostFromRoot = newCost;
                priorityQueue.Enqueue(child, newCost);
            }
        }
    }

    /// <summary>
    /// Phase 2: Compute distance-to-goal using reverse BFS from all goal states.
    /// Also identifies dead-end states (non-goal states that cannot reach any goal).
    /// </summary>
    private void ComputeDistanceToGoal(CancellationToken cancellationToken)
    {
        var goalNodes = new List<SearchSpaceNode>();

        foreach (SearchSpaceNode node in _nodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (node.IsGoal)
                goalNodes.Add(node);

            node.DistanceToGoal = -1;
            node.IsDeadEnd = true;
        }

        foreach (var goalNode in goalNodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            goalNode.DistanceToGoal = 0;
            goalNode.IsDeadEnd = false;
        }

        var queue = new Queue<SearchSpaceNode>();
        foreach (var goalNode in goalNodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            queue.Enqueue(goalNode);
        }

        while (queue.Count > 0)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var currentNode = queue.Dequeue();

            foreach (var (parent, _) in currentNode.Parents)
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (parent.DistanceToGoal >= 0)
                    continue;

                parent.DistanceToGoal = currentNode.DistanceToGoal + 1;
                parent.IsDeadEnd = false;
                queue.Enqueue(parent);
            }
        }
    }

    /// <summary>
    /// Phase 3: Compute cost-to-goal using Dijkstra's algorithm on the reverse graph.
    /// </summary>
    private void ComputeCostToGoal(CancellationToken cancellationToken)
    {
        var priorityQueue = new PriorityQueue<SearchSpaceNode, double>();

        foreach (SearchSpaceNode node in _nodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            node.CostToGoal = double.PositiveInfinity;
        }

        foreach (SearchSpaceNode node in _nodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (!node.IsGoal)
                continue;

            node.CostToGoal = 0.0;
            priorityQueue.Enqueue(node, 0.0);
        }

        while (priorityQueue.TryDequeue(out SearchSpaceNode? currentNode, out double queuedCost))
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (queuedCost > currentNode.CostToGoal)
                continue;

            foreach (var (parent, action) in currentNode.Parents)
            {
                cancellationToken.ThrowIfCancellationRequested();
                double newCost = action.Cost + queuedCost;

                if (newCost >= parent.CostToGoal)
                    continue;

                parent.CostToGoal = newCost;
                priorityQueue.Enqueue(parent, newCost);
            }
        }
    }

    /// <summary>
    /// Creates a new SearchSpaceNode.
    /// </summary>
    private SearchSpaceNode CreateNode(State state, int depth)
    {
        var node = new SearchSpaceNode(state, depth);
        _nodes.Add(node);
        return node;
    }

    private (
        IReadOnlyList<State> AllStates,
        IReadOnlyList<State> GoalStates,
        IReadOnlyList<State> DeadEndStates,
        int MaxDistanceToGoal) Freeze(CancellationToken cancellationToken)
    {
        var allStates = new State[_nodes.Count];
        var goalStates = new List<State>();
        var deadEndStates = new List<State>();
        int maxDistanceToGoal = -1;

        for (int i = 0; i < _nodes.Count; i++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            SearchSpaceNode node = _nodes[i];
            node.Freeze();
            allStates[i] = node.State;

            if (node.IsGoal)
                goalStates.Add(node.State);
            if (node.IsDeadEnd)
                deadEndStates.Add(node.State);
            if (!node.IsDeadEnd && node.DistanceToGoal >= 0)
                maxDistanceToGoal = Math.Max(maxDistanceToGoal, node.DistanceToGoal);
        }

        return (
            Array.AsReadOnly(allStates),
            Array.AsReadOnly(goalStates.ToArray()),
            Array.AsReadOnly(deadEndStates.ToArray()),
            maxDistanceToGoal);
    }

    private SearchSpaceStatistics ComputeStatistics(
        TimeSpan expansionTime,
        CancellationToken cancellationToken)
    {
        int totalTransitions = 0;
        int goalStateCount = 0;
        int deadEndCount = 0;
        int maxDepth = 0;
        int totalApplicableActions = 0;
        var depthDistribution = new Dictionary<int, int>();
        var outDegreeDistribution = new Dictionary<int, int>();

        foreach (SearchSpaceNode node in _nodes)
        {
            cancellationToken.ThrowIfCancellationRequested();
            totalTransitions += node.Successors.Count;
            totalApplicableActions += node.ApplicableActions.Count;
            goalStateCount += node.IsGoal ? 1 : 0;
            deadEndCount += node.IsDeadEnd ? 1 : 0;
            maxDepth = Math.Max(maxDepth, node.Depth);
            depthDistribution[node.Depth] = depthDistribution.GetValueOrDefault(node.Depth) + 1;
            int outDegree = node.Successors.Count;
            outDegreeDistribution[outDegree] = outDegreeDistribution.GetValueOrDefault(outDegree) + 1;
        }

        double averageOutDegree = _nodes.Count > 0 ? (double)totalTransitions / _nodes.Count : 0;
        double averageBranchingFactor = _nodes.Count > 0 ? (double)totalApplicableActions / _nodes.Count : 0;

        return new SearchSpaceStatistics(
            TotalStates: _nodes.Count,
            TotalTransitions: totalTransitions,
            GoalStateCount: goalStateCount,
            DeadEndStateCount: deadEndCount,
            MaxDepth: maxDepth,
            AverageBranchingFactor: averageBranchingFactor,
            AverageOutDegree: averageOutDegree,
            AverageInDegree: averageOutDegree,
            ExpansionTime: expansionTime,
            DepthDistribution: depthDistribution.AsReadOnly(),
            OutDegreeDistribution: outDegreeDistribution.AsReadOnly()
        );
    }

    /// <summary>
    /// Internal: Try to find the node for a given state.
    /// </summary>
    internal bool TryGetNode(State state, out SearchSpaceNode node)
    {
        if (_stateToNode.TryGetValue(state, out var foundNode))
        {
            node = foundNode;
            return true;
        }

        node = null!;
        return false;
    }
}
