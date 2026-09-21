using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Heuristics;

/// <summary>
/// h^max heuristic (Bonet &amp; Geffner): for each fact, the minimum cost to
/// derive it in the delete-relaxation taking the max of precondition costs;
/// h^max(s) is the max of those costs over the goal facts.
///
/// Admissible for the supported grounded RPG fragment.
/// </summary>
public sealed class MaxHeuristic : IHeuristic, IGroundedHeuristic
{
    private readonly GroundedApplicableActionGenerator _actionGenerator;
    private readonly GroundedRelaxedPlanningGraph _rpg;
    private readonly GoalCondition _defaultGoal;

    public MaxHeuristic(Problem problem, GoalCondition? goal = null)
        : this(HeuristicBinding.GetInitialGenerator(problem), goal)
    {
    }

    internal MaxHeuristic(GroundedApplicableActionGenerator actionGenerator, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(actionGenerator);

        Problem problem = actionGenerator.Problem;
        _actionGenerator = actionGenerator;
        _rpg = new GroundedRelaxedPlanningGraph(actionGenerator);
        _defaultGoal = goal ?? GoalCondition.FromProblem(problem);
        _rpg.EnsureSupportedGoal(_defaultGoal);
    }

    GroundedApplicableActionGenerator IGroundedHeuristic.ActionGenerator => _actionGenerator;

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        _rpg.EnsureCompatibleState(state.State);
        var g = goal ?? _defaultGoal;
        _rpg.EnsureSupportedGoal(g);
        if (g.IsSatisfied(state)) return new HeuristicEvaluation(0);
        var result = _rpg.Compute(state.State, GroundedRelaxedPlanningGraph.Combiner.Max);
        return new HeuristicEvaluation(result.MaxGoalCost(g, state.State));
    }
}
