using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

/// <summary>
/// Reusable per-state relaxed-planning-graph computation over a pre-grounded
/// action set. Backs <see cref="AddHeuristic"/>, <see cref="MaxHeuristic"/>,
/// <see cref="FFHeuristic"/>, and <see cref="SetAddHeuristic"/>.
///
/// The graph ignores negative effects (delete-relaxation) and computes for
/// every reachable fact <c>f</c>:
///   - <see cref="Result.FactCost"/>[f] — the cost to derive <c>f</c> in the
///     delete-relaxation, combining precondition costs by either summation
///     (h^add) or maximum (h^max);
///   - the support option and action layer that achieved <c>f</c> at that cost.
///
/// Static preconditions are evaluated exactly. Derived action preconditions and
/// derived conditional-effect conditions are not supported.
/// </summary>
internal sealed class GroundedRelaxedPlanningGraph
{
    public enum Combiner
    {
        /// <summary>Sum precondition costs (h^add semantics).</summary>
        Add,
        /// <summary>Max precondition costs (h^max semantics, admissible).</summary>
        Max,
    }

    private readonly Problem _problem;
    private readonly IReadOnlyList<Fact<Fluent>> _fluents;
    private readonly Dictionary<Fact<Fluent>, int> _fluentIndices;
    public IReadOnlyList<GroundAction> GroundActions { get; }
    internal IReadOnlyList<Fact<Fluent>> Fluents => _fluents;
    public int FluentCount { get; }

    // Per-action precomputed data.
    private readonly int[][] _actionPosFluentPreconds;   // graph-local fluent indices

    internal IReadOnlyList<RelaxedSupportOption> SupportOptions { get; }

    // Inverted index: fact-local-id -> indices of support options that consume that fact.
    private readonly int[][] _factToSupportOptions;

    /// <summary>
    /// Build the RPG once for a problem (compiles all grounded actions).
    /// Re-use across states.
    /// </summary>
    public GroundedRelaxedPlanningGraph(GroundedApplicableActionGenerator actionGenerator)
    {
        ArgumentNullException.ThrowIfNull(actionGenerator);

        Problem problem = actionGenerator.Problem;
        _problem = problem;
        EnsureSupportedDomain(problem);
        IReadOnlyList<GroundAction> actions = actionGenerator.GroundActions;
        GroundActions = actions;
        _fluents = problem.Context.Fluents;
        FluentCount = _fluents.Count;
        _fluentIndices = new Dictionary<Fact<Fluent>, int>(ReferenceEqualityComparer.Instance);
        for (int i = 0; i < _fluents.Count; i++)
            _fluentIndices.Add(_fluents[i], i);

        _actionPosFluentPreconds = new int[actions.Count][];
        var supportOptions = new List<RelaxedSupportOption>();

        var consuming = new List<int>[FluentCount];
        for (int i = 0; i < FluentCount; i++) consuming[i] = new List<int>();

        var staticBits = problem.Context.StaticBitboardWords;
        for (int a = 0; a < actions.Count; a++)
        {
            var action = actions[a];
            _actionPosFluentPreconds[a] = BitboardOps.DecodeSetBits(action.PositiveFluentPreconditions);
            bool actionStaticOk = BitboardOps.StaticPreconditionHolds(
                action.PositiveStaticPreconditions, action.NegativeStaticPreconditions, staticBits);

            var addEffects = BitboardOps.DecodeSetBits(action.AddEffects);
            if (addEffects.Length > 0)
            {
                supportOptions.Add(new RelaxedSupportOption(
                    a,
                    _actionPosFluentPreconds[a],
                    addEffects,
                    actionStaticOk,
                    null));
            }

            foreach (var condEff in action.ConditionalEffects)
            {
                if (!condEff.Effect.IsPositive)
                    continue;

                bool conditionalStaticOk = BitboardOps.StaticPreconditionHolds(
                    condEff.PositiveStaticConditions,
                    condEff.NegativeStaticConditions,
                    staticBits);

                supportOptions.Add(new RelaxedSupportOption(
                    a,
                    UnionSorted(_actionPosFluentPreconds[a], BitboardOps.DecodeSetBits(condEff.PositiveFluentConditions)),
                    [GetFluentIndex(condEff.Effect.Value)],
                    actionStaticOk && conditionalStaticOk,
                    condEff));
            }
        }

        SupportOptions = supportOptions;
        for (int o = 0; o < supportOptions.Count; o++)
        {
            foreach (int localId in supportOptions[o].PositiveFluentPreconditions)
                consuming[localId].Add(o);
        }

        _factToSupportOptions = consuming.Select(l => l.ToArray()).ToArray();
    }

    /// <summary>
    /// Build the RPG from a starting state and propagate to fixed point.
    /// </summary>
    public Result Compute(State state, Combiner combiner)
    {
        EnsureCompatibleState(state);

        var factCost = new double[FluentCount];
        var factLayer = new int[FluentCount];
        var firstAchieverOption = new int[FluentCount];
        var supportOptionCost = new double[SupportOptions.Count];
        var supportOptionLayer = new int[SupportOptions.Count];

        for (int i = 0; i < FluentCount; i++)
        {
            factCost[i] = double.PositiveInfinity;
            factLayer[i] = int.MaxValue;
            firstAchieverOption[i] = -1;
        }
        for (int i = 0; i < SupportOptions.Count; i++)
        {
            supportOptionCost[i] = double.PositiveInfinity;
            supportOptionLayer[i] = int.MaxValue;
        }

        // Seed: facts true in `state` have cost 0.
        var dirtyFacts = new Queue<int>();
        var pendingFacts = new bool[FluentCount];
        for (int i = 0; i < FluentCount; i++)
        {
            if (state.IsTrue(_fluents[i]))
            {
                factCost[i] = 0;
                factLayer[i] = 0;
                dirtyFacts.Enqueue(i);
                pendingFacts[i] = true;
            }
        }

        for (int o = 0; o < SupportOptions.Count; o++)
        {
            if (SupportOptions[o].PositiveFluentPreconditions.Length == 0)
                TryApplySupportOption(o);
        }

        // Propagate to fixed point. Each fact propagates its cost change to
        // all consuming support options; each option whose preconditions are all known
        // updates its add-effects.
        while (dirtyFacts.Count > 0)
        {
            int f = dirtyFacts.Dequeue();
            pendingFacts[f] = false;
            foreach (int o in _factToSupportOptions[f])
                TryApplySupportOption(o);
        }

        return new Result(factCost, factLayer, firstAchieverOption, this);

        void TryApplySupportOption(int optionIndex)
        {
            var option = SupportOptions[optionIndex];
            if (!option.StaticPreconditionsHold)
                return;

            double combined = 0;
            int maxPreconditionLayer = 0;
            foreach (int p in option.PositiveFluentPreconditions)
            {
                double pc = factCost[p];
                if (double.IsPositiveInfinity(pc))
                    return;

                combined = combiner == Combiner.Add ? combined + pc : Math.Max(combined, pc);
                maxPreconditionLayer = Math.Max(maxPreconditionLayer, factLayer[p]);
            }

            double newCost = GroundActions[option.ActionIndex].Cost + combined;
            int newLayer = maxPreconditionLayer + 1;
            if (!IsBetterLabel(
                    newCost,
                    newLayer,
                    supportOptionCost[optionIndex],
                    supportOptionLayer[optionIndex]))
                return;

            supportOptionCost[optionIndex] = newCost;
            supportOptionLayer[optionIndex] = newLayer;
            foreach (int e in option.AddEffects)
            {
                if ((uint)e >= (uint)factCost.Length)
                    throw new InvalidOperationException("A support effect is outside the compiled graph.");

                if (IsBetterLabel(newCost, newLayer, factCost[e], factLayer[e]))
                {
                    factCost[e] = newCost;
                    factLayer[e] = newLayer;
                    firstAchieverOption[e] = optionIndex;
                    if (!pendingFacts[e])
                    {
                        dirtyFacts.Enqueue(e);
                        pendingFacts[e] = true;
                    }
                }
            }
        }
    }

    private static bool IsBetterLabel(double cost, int layer, double currentCost, int currentLayer)
    {
        return cost < currentCost || (cost == currentCost && layer < currentLayer);
    }

    private static void EnsureSupportedDomain(Problem problem)
    {
        foreach (ActionSchema action in problem.Domain.Actions)
        {
            if (action.DerivedPreconditions.Count > 0)
                throw new NotSupportedException(
                    $"Grounded RPG heuristics do not support derived preconditions on action '{action.Name}'.");

            foreach (ConditionalEffect effect in action.Effects)
            {
                if (effect.DerivedConditions.Count > 0)
                    throw new NotSupportedException(
                        $"Grounded RPG heuristics do not support derived conditions on action '{action.Name}'.");
            }
        }
    }

    internal void EnsureCompatibleState(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.Context, _problem.Context))
            throw new ArgumentException(
                "State belongs to a different problem instance than this heuristic.",
                nameof(state));
    }

    internal void EnsureSupportedGoal(GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        if (!ReferenceEquals(goal.Problem, _problem))
            throw new ArgumentException("Goal belongs to a different problem instance than this heuristic.", nameof(goal));

        _ = GetSupportedGoalFluentFacts(goal);
    }

    private static Fact<Fluent>[] GetSupportedGoalFluentFacts(GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        var facts = new List<Fact<Fluent>>();
        var seen = new HashSet<Fact<Fluent>>(ReferenceEqualityComparer.Instance);
        foreach (var lit in goal.GoalLiterals)
        {
            if (!lit.IsPositive || lit.Value is not Fact<Fluent> ff)
                throw new NotSupportedException("Grounded RPG heuristics only support conjunctive positive fluent goals.");

            if (seen.Add(ff))
                facts.Add(ff);
        }

        return facts.ToArray();
    }

    internal bool TryGetSupportedGoalFluentIndices(
        GoalCondition goal,
        State state,
        out int[] goalFluentIndices)
    {
        EnsureSupportedGoal(goal);
        EnsureCompatibleState(state);

        var indices = new List<int>();
        foreach (Fact<Fluent> fact in GetSupportedGoalFluentFacts(goal))
        {
            if (_fluentIndices.TryGetValue(fact, out int index))
            {
                indices.Add(index);
                continue;
            }

            if (!ReferenceEquals(fact.Context, _problem.Context))
                throw new InvalidOperationException("A goal fact belongs to a different problem context.");

            if (!state.IsTrue(fact))
            {
                goalFluentIndices = Array.Empty<int>();
                return false;
            }
        }

        goalFluentIndices = indices.ToArray();
        return true;
    }

    private int GetFluentIndex(Fact<Fluent> fact)
    {
        if (_fluentIndices.TryGetValue(fact, out int index))
            return index;

        throw new InvalidOperationException("A fluent fact is outside the compiled relaxed planning graph.");
    }

    private static int[] UnionSorted(int[] first, int[] second)
    {
        if (first.Length == 0)
            return second;
        if (second.Length == 0)
            return first;

        return first.Concat(second).Distinct().OrderBy(id => id).ToArray();
    }

    /// <summary>
    /// Per-state result of an RPG computation.
    /// </summary>
    internal sealed class Result
    {
        public double[] FactCost { get; }
        public int[] FactLayer { get; }
        private int[] FirstAchieverOption { get; }
        public GroundedRelaxedPlanningGraph Graph { get; }

        internal Result(
            double[] factCost,
            int[] factLayer,
            int[] firstAchieverOption,
            GroundedRelaxedPlanningGraph graph)
        {
            FactCost = factCost;
            FactLayer = factLayer;
            FirstAchieverOption = firstAchieverOption;
            Graph = graph;
        }

        /// <summary>
        /// Cost summed over a goal condition's positive fluent literals (h^add).
        /// </summary>
        public double SumGoalCosts(GoalCondition goal, State state)
        {
            if (!Graph.TryGetSupportedGoalFluentIndices(goal, state, out int[] goalFacts))
                return double.PositiveInfinity;

            double total = 0;
            foreach (int lit in goalFacts)
            {
                double c = FactCost[lit];
                if (double.IsPositiveInfinity(c)) return double.PositiveInfinity;
                total += c;
            }
            return total;
        }

        /// <summary>
        /// Max over a goal condition's positive fluent literals (h^max, admissible).
        /// </summary>
        public double MaxGoalCost(GoalCondition goal, State state)
        {
            if (!Graph.TryGetSupportedGoalFluentIndices(goal, state, out int[] goalFacts))
                return double.PositiveInfinity;

            double max = 0;
            foreach (int lit in goalFacts)
            {
                double c = FactCost[lit];
                if (double.IsPositiveInfinity(c)) return double.PositiveInfinity;
                if (c > max) max = c;
            }
            return max;
        }

        /// <summary>
        /// Extract a relaxed plan via backward chaining from goal facts.
        /// </summary>
        public ExtractedRelaxedPlan ExtractRelaxedPlan(GoalCondition goal, ExtendedState state)
        {
            Graph.EnsureCompatibleState(state.State);
            var occurrences = new HashSet<ActionOccurrence>();
            var preferredActions = new HashSet<int>();
            var queue = new Queue<int>();
            var visited = new HashSet<int>();

            if (!Graph.TryGetSupportedGoalFluentIndices(goal, state.State, out int[] goalFacts))
                throw new InvalidOperationException("Cannot extract a relaxed plan for an unreachable goal.");

            foreach (int f in goalFacts)
            {
                if (visited.Add(f)) queue.Enqueue(f);
            }

            while (queue.Count > 0)
            {
                int f = queue.Dequeue();
                if (f >= FirstAchieverOption.Length)
                    throw new InvalidOperationException("A relaxed-plan fact is outside the compiled graph.");

                int optionIndex = FirstAchieverOption[f];
                if (optionIndex < 0)
                    continue;

                RelaxedSupportOption option = Graph.SupportOptions[optionIndex];
                int layer = FactLayer[f];
                occurrences.Add(new ActionOccurrence(option.ActionIndex, layer));

                GroundAction action = Graph.GroundActions[option.ActionIndex];
                bool conditionalEffectHolds = option.ConditionalEffect?.IsSatisfied(state) ?? true;
                if (layer == 1 && conditionalEffectHolds && action.IsApplicable(state))
                    preferredActions.Add(option.ActionIndex);

                foreach (int p in option.PositiveFluentPreconditions)
                {
                    if (visited.Add(p)) queue.Enqueue(p);
                }
            }

            double total = 0;
            foreach (ActionOccurrence occurrence in occurrences)
                total += Graph.GroundActions[occurrence.ActionIndex].Cost;

            return new ExtractedRelaxedPlan(total, preferredActions);
        }

        private readonly record struct ActionOccurrence(int ActionIndex, int Layer);

        internal sealed class ExtractedRelaxedPlan
        {
            public double TotalCost { get; }
            public HashSet<int> PreferredActionIndices { get; }

            public ExtractedRelaxedPlan(double totalCost, HashSet<int> preferredActionIndices)
            {
                TotalCost = totalCost;
                PreferredActionIndices = preferredActionIndices;
            }
        }
    }
}

internal sealed class RelaxedSupportOption
{
    public int ActionIndex { get; }
    public int[] PositiveFluentPreconditions { get; }
    public int[] AddEffects { get; }
    public bool StaticPreconditionsHold { get; }
    public GroundConditionalEffect? ConditionalEffect { get; }

    public RelaxedSupportOption(
        int actionIndex,
        int[] positiveFluentPreconditions,
        int[] addEffects,
        bool staticPreconditionsHold,
        GroundConditionalEffect? conditionalEffect)
    {
        ActionIndex = actionIndex;
        PositiveFluentPreconditions = positiveFluentPreconditions;
        AddEffects = addEffects;
        StaticPreconditionsHold = staticPreconditionsHold;
        ConditionalEffect = conditionalEffect;
    }
}
