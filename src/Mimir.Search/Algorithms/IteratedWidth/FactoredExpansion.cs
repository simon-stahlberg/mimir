using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.IteratedWidth;

/// <summary>
/// Picks, out of a factored group, the members IW(1) could possibly keep.
/// <para>
/// A successor is novel at width 1 exactly when it makes some atom true for the first time, and the atoms it
/// makes true are fixed by the parameters occurring in the schema's positive effects. Two members agreeing on
/// those parameters therefore share a novelty verdict, and the first of them generated registers the atoms that
/// prune the rest. Generating one of them is what standard IW(1) keeps anyway.
/// </para>
/// <para>
/// Goal states need the same care: the goal test runs before novelty pruning, so a member that reaches the goal
/// must not be skipped. Parameters occurring in any effect over a goal predicate are therefore selected too,
/// which makes members sharing a selection agree on the goal verdict as well.
/// </para>
/// </summary>
internal sealed class FactoredExpansion
{
    private readonly Dictionary<ActionSchema, int[]?> _selectionsBySchema;

    internal FactoredExpansion(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);

        var goalPredicates = new HashSet<Predicate>(ReferenceEqualityComparer.Instance);
        foreach (Literal<Fact> literal in problem.Goal)
            goalPredicates.Add(literal.Value.Predicate);

        _selectionsBySchema = new Dictionary<ActionSchema, int[]?>(ReferenceEqualityComparer.Instance);
        foreach (ActionSchema schema in problem.Domain.Actions)
            _selectionsBySchema.Add(schema, FindSelection(schema, goalPredicates));
    }

    /// <summary>
    /// The members of the group worth generating, or every member when the schema's effects are not determined
    /// by its arguments alone.
    /// </summary>
    internal IEnumerable<Action> Select(FactoredAction group)
    {
        int[]? selection = _selectionsBySchema[group.Schema];
        return selection is null ? group.Expand() : group.ExpandProjection(selection);
    }

    /// <summary>
    /// The parameters that decide a member's novelty and goal verdicts, or null when a conditional or quantified
    /// effect means the added atoms depend on more than the binding.
    /// </summary>
    private static int[]? FindSelection(ActionSchema schema, HashSet<Predicate> goalPredicates)
    {
        if (schema.NumericEffects.Count > 0) return null;

        var slots = new Dictionary<Variable, int>(schema.Parameters.Count, ReferenceEqualityComparer.Instance);
        for (int i = 0; i < schema.Parameters.Count; i++)
            slots[schema.Parameters[i]] = i;

        var selected = new SortedSet<int>();
        foreach (ConditionalEffect effect in schema.Effects)
        {
            if (!effect.IsUnconditional) return null;
            if (!effect.Effect.IsPositive && !goalPredicates.Contains(effect.Effect.Value.Predicate))
                continue;

            foreach (ITerm argument in effect.Effect.Value.Arguments)
            {
                if (argument is Variable variable) selected.Add(slots[variable]);
            }
        }

        return selected.ToArray();
    }
}
