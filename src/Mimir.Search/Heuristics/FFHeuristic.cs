using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

/// <summary>
/// FF (Fast-Forward) heuristic on a fully grounded action set. Computes a
/// relaxed plan via h^add propagation, then extracts the plan by backward
/// chaining from goal facts through first-achievers.
///
/// Counterpart to <see cref="LiftedFfHeuristic"/>; this variant is faster on
/// small grounded problems but uses more memory upfront. It also exposes
/// preferred actions (the actions chosen at the first relaxed-plan layer).
/// </summary>
public sealed class FFHeuristic : IHeuristic, IGroundedHeuristic
{
    private readonly GroundedApplicableActionGenerator _actionGenerator;
    private readonly GroundedRelaxedPlanningGraph _rpg;
    private readonly GoalCondition _defaultGoal;

    public FFHeuristic(Problem problem, GoalCondition? goal = null)
        : this(HeuristicBinding.GetInitialGenerator(problem), goal)
    {
    }

    internal FFHeuristic(GroundedApplicableActionGenerator actionGenerator, GoalCondition? goal = null)
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
        State compactState = state.State;
        _rpg.EnsureCompatibleState(compactState);
        var g = goal ?? _defaultGoal;
        _rpg.EnsureSupportedGoal(g);
        if (g.IsSatisfied(state)) return new HeuristicEvaluation(0);
        var result = _rpg.Compute(compactState, GroundedRelaxedPlanningGraph.Combiner.Add);
        if (double.IsPositiveInfinity(result.SumGoalCosts(g, compactState)))
            return new HeuristicEvaluation(double.PositiveInfinity);

        var plan = result.ExtractRelaxedPlan(g, state);
        var preferredActionSet = new HashSet<GroundAction>(
            plan.PreferredActionIndices.Select(i => _rpg.GroundActions[i]),
            ReferenceEqualityComparer.Instance);

        bool IsPreferred(GroundAction action) => preferredActionSet.Contains(action);

        return new HeuristicEvaluation(plan.TotalCost, IsPreferred);
    }
}
