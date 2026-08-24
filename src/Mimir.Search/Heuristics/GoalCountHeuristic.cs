using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Heuristics;

public sealed class GoalCountHeuristic : IHeuristic
{
    private readonly Problem _problem;
    private GoalCondition _currentGoal;

    public GoalCountHeuristic(Problem problem, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(problem);
        if (goal is not null && !ReferenceEquals(goal.Problem, problem))
            throw new ArgumentException("Goal belongs to a different problem instance than this heuristic.", nameof(goal));

        _problem = problem;
        _currentGoal = goal ?? GoalCondition.FromProblem(problem);
    }

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        GoalCondition effectiveGoal = goal ?? _currentGoal;
        if (goal is not null && !ReferenceEquals(goal.Problem, _problem))
            throw new ArgumentException("Goal belongs to a different problem instance than this heuristic.", nameof(goal));

        int unsatisfiedGoals = effectiveGoal.CountUnsatisfiedGoals(state);
        if (goal is not null)
            _currentGoal = goal;

        return new HeuristicEvaluation(unsatisfiedGoals);
    }
}
