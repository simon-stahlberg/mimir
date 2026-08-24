using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

/// <summary>
/// h^2 heuristic (Haslum &amp; Geffner, 2000): admissible heuristic that tracks
/// the cheapest cost to achieve every pair of facts simultaneously in the
/// delete-relaxation. Returns the max over goal-fact-pairs of that cost.
/// </summary>
public sealed class H2Heuristic : IHeuristic, IGroundedRpgHeuristic
{
    private readonly GroundedApplicableActionGenerator _actionGenerator;
    private readonly GoalCondition _defaultGoal;
    private readonly GroundedRelaxedPlanningGraph _rpg;
    private readonly IReadOnlyList<GroundAction> _actions;
    private readonly RelaxedSupportOption[][] _supportOptionsByAction;

    public H2Heuristic(GroundedApplicableActionGenerator actionGenerator, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(actionGenerator);

        Problem problem = actionGenerator.Problem;
        _actionGenerator = actionGenerator;
        _defaultGoal = goal ?? GoalCondition.FromProblem(problem);
        _rpg = new GroundedRelaxedPlanningGraph(actionGenerator);
        _rpg.EnsureSupportedGoal(_defaultGoal);
        _actions = _rpg.GroundActions;
        _supportOptionsByAction = Enumerable.Range(0, _actions.Count)
            .Select(actionIndex => _rpg.SupportOptions
                .Where(option => option.ActionIndex == actionIndex && option.StaticPreconditionsHold)
                .ToArray())
            .ToArray();
    }

    GroundedApplicableActionGenerator IGroundedRpgHeuristic.ActionGenerator => _actionGenerator;

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        State compactState = state.State;
        _rpg.EnsureCompatibleState(compactState);
        GoalCondition g = goal ?? _defaultGoal;
        if (!_rpg.TryGetSupportedGoalFluentIndices(g, compactState, out int[] goalFacts))
            return new HeuristicEvaluation(double.PositiveInfinity);
        if (g.IsSatisfied(state))
            return new HeuristicEvaluation(0);

        int factCount = _rpg.FluentCount;
        var costs = new double[factCount, factCount];
        for (int first = 0; first < factCount; first++)
        {
            for (int second = 0; second < factCount; second++)
                costs[first, second] = double.PositiveInfinity;
        }

        var trueFacts = new List<int>();
        for (int fact = 0; fact < factCount; fact++)
        {
            if (compactState.IsTrue(_rpg.Fluents[fact]))
                trueFacts.Add(fact);
        }

        foreach (int first in trueFacts)
        {
            foreach (int second in trueFacts)
                costs[first, second] = 0d;
        }

        bool changed;
        do
        {
            changed = false;
            for (int actionIndex = 0; actionIndex < _actions.Count; actionIndex++)
            {
                RelaxedSupportOption[] options = _supportOptionsByAction[actionIndex];
                if (options.Length == 0)
                    continue;

                double actionCost = _actions[actionIndex].Cost;
                foreach (RelaxedSupportOption option in options)
                {
                    double preconditionCost = MaxPairCost(costs, option.PositiveFluentPreconditions);
                    if (!double.IsPositiveInfinity(preconditionCost))
                    {
                        double candidate = actionCost + preconditionCost;
                        foreach (int effect in option.AddEffects)
                            changed |= TryUpdate(costs, effect, effect, candidate);
                    }

                    foreach (int effect in option.AddEffects)
                    {
                        for (int persisted = 0; persisted < factCount; persisted++)
                        {
                            if (persisted == effect)
                                continue;

                            double regressionCost = MaxPairCostWithPersistedFact(
                                costs,
                                option.PositiveFluentPreconditions,
                                persisted);
                            if (double.IsPositiveInfinity(regressionCost))
                                continue;

                            changed |= TryUpdate(
                                costs,
                                effect,
                                persisted,
                                actionCost + regressionCost);
                        }
                    }
                }

                for (int firstOptionIndex = 0; firstOptionIndex < options.Length; firstOptionIndex++)
                {
                    RelaxedSupportOption firstOption = options[firstOptionIndex];
                    for (int secondOptionIndex = firstOptionIndex;
                         secondOptionIndex < options.Length;
                         secondOptionIndex++)
                    {
                        RelaxedSupportOption secondOption = options[secondOptionIndex];
                        int[] regression = UnionSorted(
                            firstOption.PositiveFluentPreconditions,
                            secondOption.PositiveFluentPreconditions);
                        double regressionCost = MaxPairCost(costs, regression);
                        if (double.IsPositiveInfinity(regressionCost))
                            continue;

                        double candidate = actionCost + regressionCost;
                        foreach (int firstEffect in firstOption.AddEffects)
                        {
                            foreach (int secondEffect in secondOption.AddEffects)
                                changed |= TryUpdate(costs, firstEffect, secondEffect, candidate);
                        }
                    }
                }
            }
        }
        while (changed);

        double value = MaxPairCost(costs, goalFacts);
        return new HeuristicEvaluation(value);
    }

    private static bool TryUpdate(double[,] costs, int first, int second, double candidate)
    {
        if (candidate >= costs[first, second])
            return false;

        costs[first, second] = candidate;
        costs[second, first] = candidate;
        return true;
    }

    private static double MaxPairCost(double[,] costs, int[] facts)
    {
        double max = 0d;
        for (int first = 0; first < facts.Length; first++)
        {
            for (int second = first; second < facts.Length; second++)
            {
                double cost = costs[facts[first], facts[second]];
                if (double.IsPositiveInfinity(cost))
                    return double.PositiveInfinity;
                if (cost > max)
                    max = cost;
            }
        }

        return max;
    }

    private static double MaxPairCostWithPersistedFact(
        double[,] costs,
        int[] preconditions,
        int persistedFact)
    {
        double max = costs[persistedFact, persistedFact];
        if (double.IsPositiveInfinity(max))
            return double.PositiveInfinity;

        double preconditionCost = MaxPairCost(costs, preconditions);
        if (double.IsPositiveInfinity(preconditionCost))
            return double.PositiveInfinity;
        max = Math.Max(max, preconditionCost);

        foreach (int precondition in preconditions)
        {
            double pairCost = costs[precondition, persistedFact];
            if (double.IsPositiveInfinity(pairCost))
                return double.PositiveInfinity;
            max = Math.Max(max, pairCost);
        }

        return max;
    }

    private static int[] UnionSorted(int[] first, int[] second)
    {
        if (first.Length == 0)
            return second;
        if (second.Length == 0)
            return first;

        return first.Concat(second).Distinct().OrderBy(index => index).ToArray();
    }
}
