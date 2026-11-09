using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

/// <summary>
/// FF over a relaxed planning graph built from factored action groups. A layer costs one projection per group
/// rather than one step per ground action, and a member is only ever materialised for the projection that adds
/// an atom -- so the number of instances the graph never enumerates is the number it never pays for.
/// <para>
/// One graph is reused across evaluations, so a single instance answers one state at a time. Evaluating several
/// states at once needs an instance each.
/// </para>
/// </summary>
public sealed class FactoredFfHeuristic : IHeuristic
{
    private const int Unreached = int.MaxValue;

    private readonly Problem _problem;
    private readonly GoalCondition? _goal;
    private readonly IFactoredApplicableActionGenerator _generator;
    private readonly Dictionary<ActionSchema, SchemaEffectModel> _models;

    private int[] _layer = [];
    private GroundAction?[] _supporter = [];
    private bool[] _marked = [];
    private readonly List<int> _written = [];
    private readonly List<List<int>> _subgoalsByLayer = [];
    private readonly List<Fact<Fluent>> _added = [];
    private readonly HashSet<int> _fresh = [];
    private readonly HashSet<GroundAction> _selected = [];

    private FactoredFfHeuristic(
        Problem problem,
        GoalCondition? goal,
        IFactoredApplicableActionGenerator generator,
        Dictionary<ActionSchema, SchemaEffectModel> models)
    {
        _problem = problem;
        _goal = goal;
        _generator = generator;
        _models = models;
    }

    /// <summary>
    /// Null when the relaxation would be unsound or incomplete here: dropping deletes keeps a negative
    /// precondition true for ever, and a conditional or numeric effect has no place in this graph at all.
    /// </summary>
    public static FactoredFfHeuristic? TryCreate(Problem problem, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(problem);
        if (goal is not null && !CanRepresent(goal)) return null;
        if (problem.GetApplicableActionGenerator(problem.InitialState)
            is not IFactoredApplicableActionGenerator generator)
            return null;

        var models = new Dictionary<ActionSchema, SchemaEffectModel>(ReferenceEqualityComparer.Instance);
        foreach (ActionSchema schema in problem.Domain.Actions)
        {
            foreach (Literal<Atom<Fluent>> precondition in schema.FluentPreconditions)
                if (!precondition.IsPositive) return null;

            if (schema.ConditionalEffects.Count > 0 || schema.NumericEffects.Count > 0) return null;
            models[schema] = SchemaEffectModel.Compile(schema);
        }

        return new FactoredFfHeuristic(problem, goal, generator, models);
    }

    /// <summary>
    /// Whether the relaxation can speak about this goal at all. A relaxation that never deletes cannot judge a
    /// negative literal, says nothing about a numeric comparison, and reading an arbitrary expression as the
    /// conjunction of the atoms inside it would answer a different question than the one asked.
    /// </summary>
    public static bool CanRepresent(GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        if (goal.Expression is not null || goal.NumericConditions.Count > 0) return false;

        foreach (Literal<Fact> literal in goal.GoalLiterals)
            if (!literal.IsPositive) return false;

        return true;
    }

    /// <summary>Whether the delete relaxation reaches the goal, without paying for a relaxed plan.</summary>
    public bool IsGoalReachable(ExtendedState state, GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(state);
        Reset();
        return BuildLayers(state, Require(goal)) >= 0;
    }

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(state);
        GoalCondition target = Require(goal ?? _goal ?? GoalCondition.FromProblem(_problem));

        Reset();
        int depth = BuildLayers(state, target);
        if (depth < 0) return new HeuristicEvaluation(double.PositiveInfinity);

        var helpful = new List<GroundAction>();
        double cost = ExtractRelaxedPlan(target, depth, helpful);
        return new HeuristicEvaluation(cost, candidate => IsHelpful(helpful, candidate));
    }

    private static GoalCondition Require(GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        if (!CanRepresent(goal))
            throw new NotSupportedException(
                "Factored FF needs a goal that is a conjunction of positive atoms; "
                + "negative literals, numeric comparisons and arbitrary expressions are outside its relaxation.");

        return goal;
    }

    /// <summary>
    /// The relaxed plan's first step is a set of actions applicable now, and those are the ones worth trying
    /// first. A group's members differ only in parameters its add effects never mention, so two members that
    /// agree on those parameters are the same step as far as the relaxed plan is concerned.
    /// </summary>
    private bool IsHelpful(List<GroundAction> helpful, GroundAction candidate)
    {
        foreach (GroundAction step in helpful)
        {
            if (!ReferenceEquals(step.Schema, candidate.Schema)) continue;
            if (!_models.TryGetValue(step.Schema, out SchemaEffectModel? model)) continue;

            bool same = true;
            foreach (int parameter in model.AddParameters)
            {
                if (!Equals(step.Arguments[parameter], candidate.Arguments[parameter])) { same = false; break; }
            }

            if (same) return true;
        }

        return false;
    }

    private void Reset()
    {
        foreach (int index in _written)
        {
            _layer[index] = Unreached;
            _supporter[index] = null;
            _marked[index] = false;
        }

        _written.Clear();
    }

    private void Grow(int index)
    {
        if (index < _layer.Length) return;

        int previous = _layer.Length;
        int length = Math.Max(previous == 0 ? 64 : previous, 4);
        while (index >= length) length *= 2;
        Array.Resize(ref _layer, length);
        Array.Resize(ref _supporter, length);
        Array.Resize(ref _marked, length);
        // Resize zero-fills, and zero is a legitimate layer, so the new entries have to be spelled out.
        for (int i = previous; i < length; i++) _layer[i] = Unreached;
    }

    private void Record(int index, int layer, GroundAction? supporter)
    {
        Grow(index);
        _layer[index] = layer;
        _supporter[index] = supporter;
        _written.Add(index);
    }

    /// <summary>The layer the goal was first reached at, or -1 when the relaxation never reaches it.</summary>
    private int BuildLayers(ExtendedState state, GoalCondition goal)
    {
        ReadOnlySpan<ulong> words = state.State.Bitboard;
        for (int word = 0; word < words.Length; word++)
        {
            ulong bits = words[word];
            while (bits != 0)
            {
                int bit = System.Numerics.BitOperations.TrailingZeroCount(bits);
                bits &= bits - 1;
                Record((word << 6) + bit, 0, null);
            }
        }

        State current = state.State;
        ExtendedState expanded = state;
        int layer = 0;

        while (true)
        {
            if (GoalReached(goal, expanded)) return layer;

            _added.Clear();
            _fresh.Clear();
            foreach (FactoredAction group in _generator.GetFactoredApplicableActions(expanded))
            {
                SchemaEffectModel model = _models[group.Schema];
                IEnumerable<GroundAction> representatives =
                    model.IsEligible ? group.ExpandProjection(model.AddParameters) : group.Expand();

                foreach (GroundAction action in representatives)
                {
                    foreach (int index in Bits(action.AddEffects))
                    {
                        if (index < _layer.Length && _layer[index] != Unreached) continue;
                        if (!_fresh.Add(index)) continue;

                        Record(index, layer + 1, action);
                        _added.Add(_problem.Context.GetFact(new FluentIndex(index)));
                    }
                }
            }

            if (_added.Count == 0) return -1;

            layer++;
            current = current.WithAdditionalFluentFacts(_added);
            expanded = current.Expand();
        }
    }

    private bool GoalReached(GoalCondition goal, ExtendedState relaxed)
    {
        foreach (Literal<Fact> literal in goal.GoalLiterals)
        {
            int index = FluentIndexOf(literal.Value);
            if (index < 0)
            {
                // Statics never change and derived atoms are read off the relaxed state, so neither has a layer.
                if (!relaxed.IsTrue(literal.Value)) return false;
                continue;
            }

            if (index >= _layer.Length || _layer[index] == Unreached) return false;
        }

        return true;
    }

    /// <summary>
    /// Standard relaxed-plan extraction: each subgoal is handed to the action that first reached it, whose own
    /// preconditions become subgoals at the earlier layers they were reached at.
    /// </summary>
    private double ExtractRelaxedPlan(GoalCondition goal, int depth, List<GroundAction> helpful)
    {
        while (_subgoalsByLayer.Count <= depth) _subgoalsByLayer.Add([]);
        for (int i = 0; i <= depth; i++) _subgoalsByLayer[i].Clear();

        foreach (Literal<Fact> literal in goal.GoalLiterals)
        {
            int index = FluentIndexOf(literal.Value);
            if (index < 0 || _marked[index]) continue;
            _marked[index] = true;
            _subgoalsByLayer[Math.Min(_layer[index], depth)].Add(index);
        }

        _selected.Clear();
        double cost = 0;
        for (int level = depth; level > 0; level--)
        {
            foreach (int index in _subgoalsByLayer[level])
            {
                GroundAction? supporter = _supporter[index];
                if (supporter is null) continue;
                if (!_selected.Add(supporter)) continue;

                cost += supporter.Cost;
                if (level == 1) helpful.Add(supporter);
                foreach (int precondition in Bits(supporter.PositiveFluentPreconditions))
                {
                    if (precondition >= _layer.Length || _marked[precondition]) continue;
                    if (_layer[precondition] == Unreached) continue;
                    _marked[precondition] = true;
                    _subgoalsByLayer[Math.Min(_layer[precondition], level - 1)].Add(precondition);
                }
            }
        }

        return cost;
    }

    private int FluentIndexOf(Fact fact) => fact is Fact<Fluent> fluent ? fluent.LocalIndex : -1;

    private static IEnumerable<int> Bits(OffsetBitboard board)
    {
        for (int word = 0; word < board.WordCount; word++)
        {
            ulong bits = board[word];
            while (bits != 0)
            {
                int bit = System.Numerics.BitOperations.TrailingZeroCount(bits);
                bits &= bits - 1;
                yield return ((board.WordOffset + word) << 6) + bit;
            }
        }
    }
}
