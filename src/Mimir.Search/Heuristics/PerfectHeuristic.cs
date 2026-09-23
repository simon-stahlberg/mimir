using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Space;

namespace Mimir.Search.Heuristics;

/// <summary>
/// Perfect heuristic: returns the true minimum cost from the given state to any
/// goal state, computed by fully expanding the search space.
///
/// Useful as a benchmark / ground-truth heuristic. Construction is expensive
/// (O(|reachable states|)) and the entire space must fit in memory.
/// </summary>
public sealed class PerfectHeuristic : IHeuristic
{
    private readonly SearchSpace _space;
    private readonly GoalCondition _goal;

    public PerfectHeuristic(Problem problem, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(problem);
        _goal = goal ?? GoalCondition.FromProblem(problem);
        if (!ReferenceEquals(_goal.Problem, problem))
            throw new ArgumentException("The goal condition belongs to a different problem instance.", nameof(goal));

        _space = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(_goal)
            .Build();
    }

    /// <summary>
    /// Build a perfect heuristic from a pre-computed search space (caller owns
    /// disposal of the space).
    /// </summary>
    public PerfectHeuristic(SearchSpace space)
    {
        _space = space ?? throw new ArgumentNullException(nameof(space));
        _goal = space.Goal;
    }

    internal bool IsBoundTo(GoalCondition goal) => _goal.Equals(goal);

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        if (goal is not null && !IsBoundTo(goal))
            throw new ArgumentException("The supplied goal does not match the goal bound to this heuristic.", nameof(goal));

        _goal.EnsureCompatible(state);
        if (!_space.TryGetNode(state.State, out SearchSpaceNode node))
            throw new ArgumentException("The state is not reachable in this heuristic's search space.", nameof(state));

        return new HeuristicEvaluation(node.CostToGoal);
    }
}
