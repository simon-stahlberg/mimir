using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

/// <summary>
/// Delete-relaxed reachability over factored action groups. A group's members all add the same atoms once the
/// parameters absent from its add effects are projected away, so a layer costs a projection per group rather than
/// one step per ground action -- which is what makes this affordable where the number of instances explodes.
/// </summary>
internal sealed class FactoredRelaxedReachability
{
    private readonly IFactoredApplicableActionGenerator _generator;
    private readonly Dictionary<ActionSchema, SchemaEffectModel> _models;

    private FactoredRelaxedReachability(
        IFactoredApplicableActionGenerator generator, Dictionary<ActionSchema, SchemaEffectModel> models)
    {
        _generator = generator;
        _models = models;
    }

    /// <summary>
    /// Null when the problem is one this cannot relax soundly. Ignoring delete effects makes a negative
    /// precondition that holds now hold for ever after, so a generator that still enforces them would call
    /// states unreachable that are not, and the caller would prune a live branch.
    /// </summary>
    public static FactoredRelaxedReachability? TryCreate(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        if (problem.GetApplicableActionGenerator(problem.InitialState) is not IFactoredApplicableActionGenerator generator)
            return null;

        var models = new Dictionary<ActionSchema, SchemaEffectModel>(ReferenceEqualityComparer.Instance);
        foreach (ActionSchema schema in problem.Domain.Actions)
        {
            foreach (Literal<Atom<Fluent>> precondition in schema.FluentPreconditions)
                if (!precondition.IsPositive) return null;

            if (schema.ConditionalEffects.Count > 0 || schema.NumericEffects.Count > 0) return null;
            models[schema] = SchemaEffectModel.Compile(schema);
        }

        return new FactoredRelaxedReachability(generator, models);
    }

    public bool CanReach(ExtendedState state, GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(state);
        ArgumentNullException.ThrowIfNull(goal);

        State current = state.State;
        ExtendedState expanded = state;
        var added = new List<Fact<Fluent>>();
        // A layer's atoms are produced once per action that adds them, so without this the list handed to the
        // next state is mostly repeats.
        var fresh = new HashSet<Fact<Fluent>>();

        while (true)
        {
            if (Holds(goal, expanded)) return true;

            added.Clear();
            fresh.Clear();
            foreach (FactoredAction group in _generator.GetFactoredApplicableActions(expanded))
            {
                SchemaEffectModel model = _models[group.Schema];
                IEnumerable<GroundAction> representatives =
                    model.IsEligible ? group.ExpandProjection(model.AddParameters) : group.Expand();

                foreach (GroundAction action in representatives)
                {
                    foreach (Literal<Fact> literal in action.Effect.Literals)
                    {
                        if (!literal.IsPositive || literal.Value is not Fact<Fluent> fluent) continue;
                        if (!expanded.IsTrue(fluent) && fresh.Add(fluent)) added.Add(fluent);
                    }
                }
            }

            if (added.Count == 0) return false;

            current = current.WithAdditionalFluentFacts(added);
            expanded = current.Expand();
        }
    }

    private static bool Holds(GoalCondition goal, ExtendedState state)
    {
        foreach (Literal<Fact> literal in goal.GoalLiterals)
        {
            // A negative goal literal cannot be judged under a relaxation that never deletes.
            if (!literal.IsPositive) return false;
            if (!state.IsTrue(literal.Value)) return false;
        }

        return true;
    }
}
