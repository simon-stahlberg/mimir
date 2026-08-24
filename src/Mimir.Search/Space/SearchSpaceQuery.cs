using Mimir.Core.Grounding;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Space;

/// <summary>
/// Read-only query interface for a fully-expanded search space.
/// Designed for efficient access during supervised learning data collection.
/// </summary>
public class SearchSpaceQuery
{
    private readonly SearchSpace _space;

    internal SearchSpaceQuery(SearchSpace space)
    {
        _space = space;
    }

    /// <summary>
    /// Returns the BFS shortest path distance from the state to the nearest goal state.
    /// Returns -1 if the state is a dead-end or not in the search space.
    /// </summary>
    public int GetDistanceToGoal(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.DistanceToGoal;
        return -1;
    }

    /// <summary>
    /// Returns the minimum cost from the state to the nearest goal state.
    /// Returns PositiveInfinity if the state is a dead-end or not in the search space.
    /// </summary>
    public double GetCostToGoal(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.CostToGoal;
        return double.PositiveInfinity;
    }

    /// <summary>
    /// Returns true if the state satisfies the goal condition.
    /// Returns false if the state is not in the search space.
    /// </summary>
    public bool IsGoalState(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.IsGoal;
        return false;
    }

    /// <summary>
    /// Returns true if the state is a dead-end (cannot reach any goal).
    /// Returns false if the state is not in the search space.
    /// </summary>
    public bool IsDeadEnd(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.IsDeadEnd;
        return false;
    }

    /// <summary>
    /// Returns the BFS depth of the state from the initial state.
    /// Returns -1 if the state is not in the search space.
    /// </summary>
    public int GetDepth(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.Depth;
        return -1;
    }

    /// <summary>
    /// Returns the minimum cost to reach the state from the initial state.
    /// Returns -1 if the state is not in the search space.
    /// </summary>
    public double GetCost(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.CostFromRoot;
        return -1;
    }

    /// <summary>
    /// Returns all successor states with the action that reaches them.
    /// Returns empty list if the state is not in the search space.
    /// </summary>
    public IReadOnlyList<(State Successor, Action Action)> GetSuccessors(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.SuccessorView;
        return Array.Empty<(State, Action)>();
    }

    /// <summary>
    /// Returns all predecessor states with the action that reaches the given state.
    /// Returns empty list if the state is not in the search space.
    /// </summary>
    public IReadOnlyList<(State Predecessor, Action Action)> GetPredecessors(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.PredecessorView;
        return Array.Empty<(State, Action)>();
    }

    /// <summary>
    /// Returns all applicable actions at the state.
    /// Returns empty list if the state is not in the search space.
    /// </summary>
    public IReadOnlyList<Action> GetApplicableActions(State state)
    {
        if (_space.TryGetNode(state, out var node))
            return node.ApplicableActionView;
        return Array.Empty<Action>();
    }

    /// <summary>
    /// Returns the full distance-to-goal result for the state.
    /// </summary>
    public StateDistanceResult GetDistanceResult(State state)
    {
        if (!_space.TryGetNode(state, out var node))
            return StateDistanceResult.None;

        if (node.IsGoal)
            return StateDistanceResult.Goal;

        if (node.IsDeadEnd)
            return StateDistanceResult.DeadEnd;

        return StateDistanceResult.Reachable(node.DistanceToGoal, node.CostToGoal);
    }

    /// <summary>
    /// Returns statistics about the search space.
    /// </summary>
    public SearchSpaceStatistics GetStatistics()
    {
        return _space.Statistics;
    }
}
