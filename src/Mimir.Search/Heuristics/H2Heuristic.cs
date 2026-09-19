using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

/// <summary>
/// h^2 heuristic (Haslum &amp; Geffner, 2000): admissible heuristic that tracks the
/// cheapest cost of achieving every pair of facts simultaneously, and returns the
/// most expensive such pair over the goal.
///
/// Unlike the h^add/h^max family this is not a delete relaxation: an action carries a
/// fact across only when it cannot delete it, which is what makes h^2 stronger than h^1.
///
/// Facts are literals. Every fluent contributes a positive literal, and a fluent that
/// occurs negatively in some precondition also contributes a negative literal, so that
/// negative preconditions are evaluated exactly instead of being dropped. Negative goal
/// literals are rejected.
/// </summary>
public sealed class H2Heuristic : IHeuristic, IGroundedHeuristic
{
    private readonly GroundedApplicableActionGenerator _actionGenerator;
    private readonly GoalCondition _defaultGoal;

    private readonly Problem _problem;
    private readonly IReadOnlyList<Fact<Fluent>> _fluents;
    private readonly int _fluentCount;

    private readonly int[] _negativeLiterals;   // fluent index -> literal index, or -1
    private readonly H2Option[][] _optionsByAction;
    private readonly int _factCount;

    // Reused across evaluations. H2Heuristic is therefore not thread-safe, and the search
    // algorithms and the interop layer evaluate heuristics serially.
    private readonly double[] _costs;           // _factCount x _factCount, row-major
    private readonly int[] _trueLiterals;
    private readonly int[] _mergedPreconditions;

    public H2Heuristic(GroundedApplicableActionGenerator actionGenerator, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(actionGenerator);

        Problem problem = actionGenerator.Problem;
        EnsureSupportedDomain(problem);
        _actionGenerator = actionGenerator;
        _problem = problem;
        _fluents = problem.Context.Fluents;
        _fluentCount = _fluents.Count;
        _defaultGoal = goal ?? GoalCondition.FromProblem(problem);
        EnsureSupportedGoal(_defaultGoal);

        IReadOnlyList<GroundAction> actions = actionGenerator.GroundActions;
        _negativeLiterals = CreateNegativeLiterals(
            actions,
            problem.Context.StaticBitboardWords,
            _fluentCount,
            out _factCount);
        _optionsByAction = CompileOptions(actions, problem.Context.StaticBitboardWords);

        _costs = new double[_factCount * _factCount];
        _trueLiterals = new int[_factCount];
        _mergedPreconditions = new int[_factCount];
    }

    GroundedApplicableActionGenerator IGroundedHeuristic.ActionGenerator => _actionGenerator;

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        State compactState = state.State;
        EnsureCompatibleState(compactState);
        GoalCondition effectiveGoal = goal ?? _defaultGoal;
        if (!TryGetGoalFacts(effectiveGoal, compactState, out int[] goalFacts))
            return new HeuristicEvaluation(double.PositiveInfinity);
        if (effectiveGoal.IsSatisfied(state))
            return new HeuristicEvaluation(0);

        Seed(compactState);
        ComputeFixpoint();
        return new HeuristicEvaluation(MaxPairCost(goalFacts));
    }

    // -------- Validation --------

    private static void EnsureSupportedDomain(Problem problem)
    {
        foreach (ActionSchema action in problem.Domain.Actions)
        {
            if (action.DerivedPreconditions.Count > 0)
                throw new NotSupportedException(
                    $"h^2 does not support derived preconditions on action '{action.Name}'.");

            foreach (ConditionalEffect effect in action.Effects)
            {
                if (effect.DerivedConditions.Count > 0)
                    throw new NotSupportedException(
                        $"h^2 does not support derived conditions on action '{action.Name}'.");
            }
        }
    }

    private void EnsureCompatibleState(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.Context, _problem.Context))
            throw new ArgumentException(
                "State belongs to a different problem instance than this heuristic.",
                nameof(state));
    }

    private void EnsureSupportedGoal(GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        if (!ReferenceEquals(goal.Problem, _problem))
            throw new ArgumentException(
                "Goal belongs to a different problem instance than this heuristic.",
                nameof(goal));

        foreach (Literal<Fact> literal in goal.GoalLiterals)
        {
            if (!literal.IsPositive || literal.Value is not Fact<Fluent>)
                throw new NotSupportedException("h^2 only supports conjunctive positive fluent goals.");
        }
    }

    private bool TryGetGoalFacts(GoalCondition goal, State state, out int[] goalFacts)
    {
        EnsureSupportedGoal(goal);

        var facts = new List<int>();
        foreach (Literal<Fact> literal in goal.GoalLiterals)
        {
            var fact = (Fact<Fluent>)literal.Value;
            if (fact.LocalIndex < _fluentCount)
            {
                facts.Add(fact.LocalIndex);
                continue;
            }

            // Grounded after this heuristic compiled its fact universe, so no action can
            // establish it and only the state can satisfy it.
            if (!state.IsTrue(fact))
            {
                goalFacts = [];
                return false;
            }
        }

        goalFacts = facts.ToArray();
        return true;
    }

    // -------- Compilation --------

    /// <summary>
    /// Assign a literal index to every fluent that occurs negatively in some condition.
    /// Nothing else can require one: goals are positive by construction.
    /// </summary>
    private static int[] CreateNegativeLiterals(
        IReadOnlyList<GroundAction> actions,
        ReadOnlySpan<ulong> staticBitboard,
        int fluentCount,
        out int factCount)
    {
        var negativeLiterals = new int[fluentCount];
        Array.Fill(negativeLiterals, -1);
        factCount = fluentCount;

        foreach (GroundAction action in actions)
        {
            if (!BitboardOps.StaticPreconditionHolds(
                    action.PositiveStaticPreconditions,
                    action.NegativeStaticPreconditions,
                    staticBitboard))
                continue;

            foreach (int fluent in BitboardOps.EnumerateSetBits(action.NegativeFluentPreconditions))
            {
                if (negativeLiterals[fluent] < 0)
                    negativeLiterals[fluent] = factCount++;
            }

            foreach (GroundConditionalEffect effect in action.ConditionalEffects)
            {
                if (!BitboardOps.StaticPreconditionHolds(
                        effect.PositiveStaticConditions,
                        effect.NegativeStaticConditions,
                        staticBitboard))
                    continue;

                foreach (int fluent in BitboardOps.EnumerateSetBits(effect.NegativeFluentConditions))
                {
                    if (negativeLiterals[fluent] < 0)
                        negativeLiterals[fluent] = factCount++;
                }
            }
        }

        return negativeLiterals;
    }

    private H2Option[][] CompileOptions(
        IReadOnlyList<GroundAction> actions,
        ReadOnlySpan<ulong> staticBitboard)
    {
        var optionsByAction = new H2Option[actions.Count][];
        var options = new List<H2Option>();

        for (int actionIndex = 0; actionIndex < actions.Count; actionIndex++)
        {
            GroundAction action = actions[actionIndex];
            options.Clear();

            if (BitboardOps.StaticPreconditionHolds(
                    action.PositiveStaticPreconditions,
                    action.NegativeStaticPreconditions,
                    staticBitboard))
            {
                int[] preconditions = ToLiterals(
                    action.PositiveFluentPreconditions,
                    action.NegativeFluentPreconditions);
                int[] addedFluents = BitboardOps.DecodeSetBits(action.AddEffects);
                int[] deletedFluents = BitboardOps.DecodeSetBits(action.DeleteEffects);

                var conditionalEffects = new List<GroundConditionalEffect>();
                foreach (GroundConditionalEffect effect in action.ConditionalEffects)
                {
                    if (BitboardOps.StaticPreconditionHolds(
                            effect.PositiveStaticConditions,
                            effect.NegativeStaticConditions,
                            staticBitboard))
                        conditionalEffects.Add(effect);
                }

                // Deletes are applied before adds, so a fact that the action might add
                // survives its delete list and must not count as deleted.
                var rescuedFluents = new HashSet<int>(addedFluents);
                foreach (GroundConditionalEffect effect in conditionalEffects)
                {
                    if (effect.Effect.IsPositive)
                        rescuedFluents.Add(effect.Effect.Value.LocalIndex);
                }

                AddOption(options, preconditions, addedFluents, deletedFluents, rescuedFluents, action.Cost);

                foreach (GroundConditionalEffect effect in conditionalEffects)
                {
                    int fluent = effect.Effect.Value.LocalIndex;
                    // A conditional delete of an unmodelled negative literal achieves
                    // nothing the unconditional option does not already achieve.
                    if (!effect.Effect.IsPositive && _negativeLiterals[fluent] < 0)
                        continue;

                    int[] effectPreconditions = Union(
                        preconditions,
                        ToLiterals(effect.PositiveFluentConditions, effect.NegativeFluentConditions));
                    int[] effectAdds = effect.Effect.IsPositive
                        ? Union(addedFluents, [fluent])
                        : addedFluents;
                    int[] effectDeletes = effect.Effect.IsPositive
                        ? deletedFluents
                        : Union(deletedFluents, [fluent]);
                    AddOption(options, effectPreconditions, effectAdds, effectDeletes, rescuedFluents, action.Cost);
                }
            }

            optionsByAction[actionIndex] = options.ToArray();
        }

        return optionsByAction;
    }

    /// <summary>
    /// Compile one way of applying an action into literal space. Adds are the facts the
    /// option may establish and deletes are the facts it certainly destroys: a spurious
    /// add or a missing delete only lowers h^2 and keeps it admissible, whereas a missing
    /// add or a spurious delete could invent a dead end.
    /// </summary>
    private void AddOption(
        List<H2Option> options,
        int[] preconditions,
        int[] addedFluents,
        int[] deletedFluents,
        HashSet<int> rescuedFluents,
        double cost)
    {
        var adds = new List<int>(addedFluents);
        foreach (int fluent in deletedFluents)
        {
            // Deletes are applied before adds, so a fluent the option certainly adds
            // stays true and its negative literal cannot be established here.
            if (Array.BinarySearch(addedFluents, fluent) >= 0)
                continue;

            int literal = _negativeLiterals[fluent];
            if (literal >= 0)
                adds.Add(literal);
        }

        if (adds.Count == 0)
            return;

        var deletes = new List<int>();
        foreach (int fluent in deletedFluents)
        {
            if (!rescuedFluents.Contains(fluent))
                deletes.Add(fluent);
        }
        foreach (int fluent in addedFluents)
        {
            int literal = _negativeLiterals[fluent];
            if (literal >= 0)
                deletes.Add(literal);
        }

        adds.Sort();
        deletes.Sort();
        options.Add(new H2Option(preconditions, adds.ToArray(), deletes.ToArray(), cost));
    }

    private int[] ToLiterals(OffsetBitboard positiveFluents, OffsetBitboard negativeFluents)
    {
        var literals = new List<int>();
        foreach (int fluent in BitboardOps.EnumerateSetBits(positiveFluents))
            literals.Add(fluent);

        foreach (int fluent in BitboardOps.EnumerateSetBits(negativeFluents))
        {
            int literal = _negativeLiterals[fluent];
            if (literal < 0)
                throw new InvalidOperationException("A negative fluent condition has no compiled literal.");
            literals.Add(literal);
        }

        literals.Sort();
        return literals.ToArray();
    }

    private static int[] Union(int[] first, int[] second)
    {
        if (first.Length == 0)
            return second;
        if (second.Length == 0)
            return first;

        return first.Concat(second).Distinct().OrderBy(literal => literal).ToArray();
    }

    // -------- Propagation --------

    private void Seed(State state)
    {
        Array.Fill(_costs, double.PositiveInfinity);

        int count = 0;
        for (int fluent = 0; fluent < _negativeLiterals.Length; fluent++)
        {
            if (state.IsTrue(_fluents[fluent]))
                _trueLiterals[count++] = fluent;
            else if (_negativeLiterals[fluent] >= 0)
                _trueLiterals[count++] = _negativeLiterals[fluent];
        }

        for (int first = 0; first < count; first++)
        {
            for (int second = first; second < count; second++)
            {
                _costs[_trueLiterals[first] * _factCount + _trueLiterals[second]] = 0d;
                _costs[_trueLiterals[second] * _factCount + _trueLiterals[first]] = 0d;
            }
        }
    }

    private void ComputeFixpoint()
    {
        bool changed;
        do
        {
            changed = false;
            foreach (H2Option[] options in _optionsByAction)
            {
                for (int index = 0; index < options.Length; index++)
                {
                    H2Option option = options[index];
                    double preconditionCost = MaxPairCost(option.Preconditions);
                    if (double.IsPositiveInfinity(preconditionCost))
                        continue;

                    changed |= UpdateAddedPairs(option, option.Cost + preconditionCost);
                    changed |= UpdatePersistedPairs(option, preconditionCost);

                    // Two conditional effects of the same action fire together whenever
                    // both conditions hold, so their added facts are achieved jointly.
                    for (int other = index + 1; other < options.Length; other++)
                        changed |= UpdateJointlyAddedPairs(option, options[other]);
                }
            }
        }
        while (changed);
    }

    private bool UpdateAddedPairs(H2Option option, double candidate)
    {
        bool changed = false;
        int[] adds = option.Adds;
        for (int first = 0; first < adds.Length; first++)
        {
            for (int second = first; second < adds.Length; second++)
                changed |= TryUpdate(adds[first], adds[second], candidate);
        }

        return changed;
    }

    private bool UpdatePersistedPairs(H2Option option, double preconditionCost)
    {
        bool changed = false;
        int[] deletes = option.Deletes;
        int deleteIndex = 0;

        for (int persisted = 0; persisted < _factCount; persisted++)
        {
            while (deleteIndex < deletes.Length && deletes[deleteIndex] < persisted)
                deleteIndex++;
            if (deleteIndex < deletes.Length && deletes[deleteIndex] == persisted)
                continue;

            double regressionCost = PairCostWith(option.Preconditions, persisted);
            if (double.IsPositiveInfinity(regressionCost))
                continue;

            double candidate = option.Cost + Math.Max(preconditionCost, regressionCost);
            foreach (int add in option.Adds)
            {
                if (add != persisted)
                    changed |= TryUpdate(add, persisted, candidate);
            }
        }

        return changed;
    }

    private bool UpdateJointlyAddedPairs(H2Option first, H2Option second)
    {
        int length = Merge(first.Preconditions, second.Preconditions, _mergedPreconditions);
        double preconditionCost = MaxPairCost(_mergedPreconditions.AsSpan(0, length));
        if (double.IsPositiveInfinity(preconditionCost))
            return false;

        bool changed = false;
        double candidate = first.Cost + preconditionCost;
        foreach (int firstAdd in first.Adds)
        {
            foreach (int secondAdd in second.Adds)
                changed |= TryUpdate(firstAdd, secondAdd, candidate);
        }

        return changed;
    }

    private bool TryUpdate(int first, int second, double candidate)
    {
        if (candidate >= _costs[first * _factCount + second])
            return false;

        _costs[first * _factCount + second] = candidate;
        _costs[second * _factCount + first] = candidate;
        return true;
    }

    private double MaxPairCost(ReadOnlySpan<int> facts)
    {
        double max = 0d;
        for (int first = 0; first < facts.Length; first++)
        {
            for (int second = first; second < facts.Length; second++)
            {
                double cost = _costs[facts[first] * _factCount + facts[second]];
                if (double.IsPositiveInfinity(cost))
                    return double.PositiveInfinity;
                if (cost > max)
                    max = cost;
            }
        }

        return max;
    }

    private double PairCostWith(int[] facts, int persisted)
    {
        double max = _costs[persisted * _factCount + persisted];
        if (double.IsPositiveInfinity(max))
            return double.PositiveInfinity;

        foreach (int fact in facts)
        {
            double cost = _costs[fact * _factCount + persisted];
            if (double.IsPositiveInfinity(cost))
                return double.PositiveInfinity;
            if (cost > max)
                max = cost;
        }

        return max;
    }

    private static int Merge(int[] first, int[] second, int[] buffer)
    {
        int firstIndex = 0;
        int secondIndex = 0;
        int count = 0;

        while (firstIndex < first.Length && secondIndex < second.Length)
        {
            if (first[firstIndex] < second[secondIndex])
                buffer[count++] = first[firstIndex++];
            else if (second[secondIndex] < first[firstIndex])
                buffer[count++] = second[secondIndex++];
            else
            {
                buffer[count++] = first[firstIndex++];
                secondIndex++;
            }
        }

        while (firstIndex < first.Length)
            buffer[count++] = first[firstIndex++];
        while (secondIndex < second.Length)
            buffer[count++] = second[secondIndex++];

        return count;
    }

    private sealed record H2Option(int[] Preconditions, int[] Adds, int[] Deletes, double Cost);
}
