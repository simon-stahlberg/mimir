using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.IteratedWidth;

/// <summary>
/// Picks, out of each factored group, the members the novelty test could tell apart, so the search never builds
/// the successors it would immediately discard.
/// <para>
/// At width 1 a successor is novel exactly when it makes some atom true for the first time, so members agreeing
/// on the parameters of the positive effects share a verdict and one of them stands for the class.
/// </para>
/// <para>
/// At width 2 the verdict also turns on which atoms the action deletes, because a pair is novel when it joins an
/// added atom to any atom of the successor. The pairs split into those shared by a whole add-class and those
/// involving an atom some sibling deletes, and only the latter separate siblings. Claiming the shared pairs
/// takes one member, and each remaining pair is claimed by a member that spares its partner — which is one more
/// member per delete literal, not one per element of the product.
/// </para>
/// </summary>
internal sealed class FactoredExpansion
{
    private readonly IFactoredApplicableActionGenerator _generator;
    private readonly InstanceContext _context;
    private readonly Dictionary<ActionSchema, SchemaEffectModel> _models;
    private readonly FluentIndex[] _goalAtoms;
    private readonly HashSet<Predicate> _goalPredicates;

    /// <summary>
    /// The shortcut for this generator, or null when the problem is outside what it reasons about. Declining
    /// leaves the search running on the ordinary expansion rather than failing outright.
    /// </summary>
    internal static FactoredExpansion? TryCreate(IFactoredApplicableActionGenerator generator)
    {
        ArgumentNullException.ThrowIfNull(generator);

        // A negated goal literal is satisfied by deleting an atom, so reaching the goal would no longer be
        // decided by what a member adds, and classifying members by their added atoms would hide goal states.
        foreach (Literal<Fact> literal in generator.Problem.Goal)
        {
            if (!literal.IsPositive) return null;
        }

        return new FactoredExpansion(generator);
    }

    private FactoredExpansion(IFactoredApplicableActionGenerator generator)
    {
        _generator = generator;
        Problem problem = generator.Problem;
        _context = problem.Context;

        _models = new Dictionary<ActionSchema, SchemaEffectModel>(ReferenceEqualityComparer.Instance);
        foreach (ActionSchema schema in problem.Domain.Actions)
            _models.Add(schema, SchemaEffectModel.Compile(schema));

        _goalAtoms = CollectGoalAtoms(problem);
        _goalPredicates = new HashSet<Predicate>(ReferenceEqualityComparer.Instance);
        foreach (Literal<Fact> literal in problem.Goal) _goalPredicates.Add(literal.Value.Predicate);
    }

    /// <summary>
    /// The members worth generating from <paramref name="state"/>. Falls back to the whole group for a width the
    /// shortcut does not cover, and for a schema whose effects are not a function of its arguments.
    /// </summary>
    internal IEnumerable<Action> CandidateActions(ExtendedState state, int width, NoveltyTracker tracker)
    {
        if (width is < 1 or > 2)
            return _generator.GetApplicableActions(state);

        return width == 1 ? SelectByAddedAtoms(state) : SelectByPairs(state, tracker);
    }

    // Goals are positive here, so a class reaches the goal only by adding every goal atom the parent is missing.
    // A delete can still destroy one, which the pair loop handles.
    private static FluentIndex[] CollectGoalAtoms(Problem problem)
    {
        var atoms = new List<FluentIndex>();
        foreach (Literal<Fact> literal in problem.Goal)
        {
            if (literal.Value is Fact<Fluent> fluent) atoms.Add(new FluentIndex(fluent.LocalIndex));
        }

        return atoms.ToArray();
    }

    private IEnumerable<Action> SelectByAddedAtoms(ExtendedState state)
    {
        foreach (FactoredAction group in _generator.GetFactoredApplicableActions(state))
        {
            SchemaEffectModel model = _models[group.Schema];
            IEnumerable<Action> members = model.IsEligible
                ? group.ExpandProjection(SelectionParameters(model))
                : group.Expand();
            foreach (Action action in members) yield return action;
        }
    }

    // At width 1 the goal verdict must stay decidable from the selection too, so parameters of deletes over goal
    // predicates join the add parameters. With positive goals only, a delete can lose a goal atom the parent had.
    private int[] SelectionParameters(SchemaEffectModel model)
    {
        var selection = new SortedSet<int>(model.AddParameters);
        foreach (EffectLiteral delete in model.Deletes)
        {
            if (!DeletesAGoalAtom(delete)) continue;
            foreach (int parameter in delete.Parameters) selection.Add(parameter);
        }

        return selection.ToArray();
    }

    // Conservative: a shared predicate is enough to force the parameters into the selection.
    private bool DeletesAGoalAtom(EffectLiteral delete) => _goalPredicates.Contains(delete.Predicate);

    private IEnumerable<Action> SelectByPairs(ExtendedState state, NoveltyTracker tracker)
    {
        FluentIndex[] unsatisfiedGoals = _goalAtoms.Where(atom => !state.State.IsTrueUnchecked(atom)).ToArray();

        foreach (FactoredAction group in _generator.GetFactoredApplicableActions(state))
        {
            SchemaEffectModel model = _models[group.Schema];
            if (!model.IsEligible || !IsWorthSelecting(group, model))
            {
                foreach (Action action in group.Expand()) yield return action;
                continue;
            }

            var selector = new PairSelector(_context, group, model, tracker, state.State, unsatisfiedGoals, _goalAtoms);
            foreach (Action action in selector.Select()) yield return action;
        }
    }

    /// <summary>
    /// The selector emits at least one member per add-class and at most one more per delete literal, so a group
    /// whose product is no larger than that bound cannot gain from the per-class bookkeeping.
    /// </summary>
    private static bool IsWorthSelecting(FactoredAction group, SchemaEffectModel model)
    {
        long classes = 1;
        foreach (int parameter in model.AddParameters)
        {
            classes *= group.GetCandidates(parameter).Length;
            if (classes >= group.MemberCountUpperBound) return false;
        }

        long emitted = classes * (1 + model.Deletes.Length);
        return emitted < group.MemberCountUpperBound;
    }
}
