using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Heuristics;

/// <summary>
/// Set-additive heuristic h^set-add: for each fact, track the SET of best
/// supporters (actions on the cheapest path to that fact) and combine over
/// goal facts by set-union, then summing the costs of the union.
///
/// Unlike h^add which double-counts shared supporters, h^set-add charges
/// each supporting action only once. This is the same notion of "relaxed plan
/// cost" used by h^FF, but extracted incrementally during forward propagation
/// rather than via backward chaining.
/// </summary>
public sealed class SetAddHeuristic : IHeuristic, IGroundedHeuristic
{
    private readonly GroundedApplicableActionGenerator _actionGenerator;
    private readonly GroundedRelaxedPlanningGraph _rpg;
    private readonly GoalCondition _defaultGoal;

    public SetAddHeuristic(GroundedApplicableActionGenerator actionGenerator, GoalCondition? goal = null)
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
        GoalCondition g = goal ?? _defaultGoal;
        _rpg.EnsureSupportedGoal(g);
        if (g.IsSatisfied(state))
            return new HeuristicEvaluation(0);

        var labels = new SupportLabel?[_rpg.FluentCount];
        for (int i = 0; i < labels.Length; i++)
        {
            if (compactState.IsTrue(_rpg.Fluents[i]))
                labels[i] = SupportLabel.Empty;
        }

        bool changed;
        do
        {
            changed = false;
            foreach (RelaxedSupportOption option in _rpg.SupportOptions)
            {
                if (!TryBuildLabel(option, labels, out SupportLabel candidate))
                    continue;

                foreach (int effect in option.AddEffects)
                {
                    if (!IsBetter(candidate, labels[effect]))
                        continue;

                    labels[effect] = candidate;
                    changed = true;
                }
            }
        }
        while (changed);

        if (!_rpg.TryGetSupportedGoalFluentIndices(g, compactState, out int[] goalFacts))
            return new HeuristicEvaluation(double.PositiveInfinity);

        var goalSupporters = new HashSet<int>();
        foreach (int goalFact in goalFacts)
        {
            SupportLabel? label = labels[goalFact];
            if (label is null)
                return new HeuristicEvaluation(double.PositiveInfinity);

            goalSupporters.UnionWith(label.ActionIndices);
        }

        double value = goalSupporters.Sum(index => _rpg.GroundActions[index].Cost);
        return new HeuristicEvaluation(value);
    }

    private bool TryBuildLabel(
        RelaxedSupportOption option,
        SupportLabel?[] factLabels,
        out SupportLabel candidate)
    {
        candidate = null!;
        if (!option.StaticPreconditionsHold)
            return false;

        var actionIndices = new HashSet<int>();
        foreach (int precondition in option.PositiveFluentPreconditions)
        {
            SupportLabel? label = factLabels[precondition];
            if (label is null)
                return false;

            actionIndices.UnionWith(label.ActionIndices);
        }

        actionIndices.Add(option.ActionIndex);
        int[] sortedActionIndices = actionIndices.OrderBy(index => index).ToArray();
        double cost = sortedActionIndices.Sum(index => _rpg.GroundActions[index].Cost);
        candidate = new SupportLabel(sortedActionIndices, cost);
        return true;
    }

    private static bool IsBetter(SupportLabel candidate, SupportLabel? current)
    {
        return current is null
            || candidate.Cost < current.Cost
            || (candidate.Cost == current.Cost
                && candidate.ActionIndices.Length < current.ActionIndices.Length);
    }

    private sealed record SupportLabel(int[] ActionIndices, double Cost)
    {
        public static SupportLabel Empty { get; } = new(Array.Empty<int>(), 0d);
    }
}
