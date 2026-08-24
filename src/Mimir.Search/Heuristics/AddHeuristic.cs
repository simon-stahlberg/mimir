using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Heuristics;

/// <summary>
/// h^add heuristic (Bonet &amp; Geffner): for each fact, the minimum cost to
/// derive it in the delete-relaxation summing precondition costs; h^add(s) is
/// the sum of those costs over the goal facts.
///
/// Inadmissible but informative.
/// </summary>
public sealed class AddHeuristic : IHeuristic, IGroundedRpgHeuristic
{
    private readonly GroundedApplicableActionGenerator _actionGenerator;
    private readonly GroundedRelaxedPlanningGraph _rpg;
    private readonly GoalCondition _defaultGoal;

    public AddHeuristic(GroundedApplicableActionGenerator actionGenerator, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(actionGenerator);

        Problem problem = actionGenerator.Problem;
        _actionGenerator = actionGenerator;
        _rpg = new GroundedRelaxedPlanningGraph(actionGenerator);
        _defaultGoal = goal ?? GoalCondition.FromProblem(problem);
        _rpg.EnsureSupportedGoal(_defaultGoal);
    }

    GroundedApplicableActionGenerator IGroundedRpgHeuristic.ActionGenerator => _actionGenerator;

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        _rpg.EnsureCompatibleState(state.State);
        var g = goal ?? _defaultGoal;
        _rpg.EnsureSupportedGoal(g);
        if (g.IsSatisfied(state)) return new HeuristicEvaluation(0);
        var result = _rpg.Compute(state.State, GroundedRelaxedPlanningGraph.Combiner.Add);
        return new HeuristicEvaluation(result.SumGoalCosts(g, state.State));
    }
}
