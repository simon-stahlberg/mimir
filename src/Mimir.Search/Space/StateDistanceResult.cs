namespace Mimir.Search.Space;

/// <summary>
/// Represents the result of a distance-to-goal query for a state.
/// </summary>
/// <param name="Distance">The shortest path distance to the nearest goal state. -1 if the state is a dead-end or unreachable.</param>
/// <param name="Cost">The minimum cost to reach the nearest goal state. Infinity if the state is a dead-end or unreachable.</param>
/// <param name="IsReachable">Whether the state can reach a goal state.</param>
/// <param name="IsDeadEnd">Whether the state is a dead-end (cannot reach any goal).</param>
/// <param name="IsGoalState">Whether the state is itself a goal state.</param>
public readonly record struct StateDistanceResult(
    int Distance,
    double Cost,
    bool IsReachable,
    bool IsDeadEnd,
    bool IsGoalState
)
{
    /// <summary>
    /// Represents an unreachable or dead-end state.
    /// </summary>
    public static StateDistanceResult None => new(-1, double.PositiveInfinity, false, false, false);

    /// <summary>
    /// Represents a dead-end state (non-goal with no path to any goal).
    /// </summary>
    public static StateDistanceResult DeadEnd => new(-1, double.PositiveInfinity, false, true, false);

    /// <summary>
    /// Represents a goal state (distance and cost are zero).
    /// </summary>
    public static StateDistanceResult Goal => new(0, 0.0, true, false, true);

    /// <summary>
    /// Represents a reachable non-goal state.
    /// </summary>
    public static StateDistanceResult Reachable(int distance, double cost)
        => new(distance, cost, true, false, false);
}
