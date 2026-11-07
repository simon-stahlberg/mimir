using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;

namespace Mimir.Search.Algorithms.IteratedWidth;

/// <summary>
/// What a schema adds and deletes, as a function of its arguments. The factored selectors need this to decide
/// which members of a group the novelty test could tell apart, without grounding any of them.
/// </summary>
internal sealed class SchemaEffectModel
{
    /// <summary>False when the added and deleted atoms are not a function of the binding alone.</summary>
    public bool IsEligible { get; }

    /// <summary>Parameters occurring in positive effects; they fix which atoms the action makes true.</summary>
    public int[] AddParameters { get; }

    public EffectLiteral[] Adds { get; }
    public EffectLiteral[] Deletes { get; }

    private SchemaEffectModel(bool isEligible, int[] addParameters, EffectLiteral[] adds, EffectLiteral[] deletes)
    {
        IsEligible = isEligible;
        AddParameters = addParameters;
        Adds = adds;
        Deletes = deletes;
    }

    public static SchemaEffectModel Compile(ActionSchema schema)
    {
        ArgumentNullException.ThrowIfNull(schema);
        SchemaEffectModel ineligible = new(false, [], [], []);
        if (schema.NumericEffects.Count > 0) return ineligible;

        var slots = new Dictionary<Variable, int>(schema.Parameters.Count, ReferenceEqualityComparer.Instance);
        for (int i = 0; i < schema.Parameters.Count; i++)
            slots[schema.Parameters[i]] = i;

        var adds = new List<EffectLiteral>();
        var deletes = new List<EffectLiteral>();
        var addParameters = new SortedSet<int>();
        foreach (ConditionalEffect effect in schema.Effects)
        {
            if (!effect.IsUnconditional) return ineligible;

            var literal = new EffectLiteral(effect.Effect.Value, slots);
            if (effect.Effect.IsPositive)
            {
                adds.Add(literal);
                foreach (int parameter in literal.Parameters) addParameters.Add(parameter);
                continue;
            }

            deletes.Add(literal);
        }

        return new SchemaEffectModel(true, addParameters.ToArray(), adds.ToArray(), deletes.ToArray());
    }
}

/// <summary>One effect literal, with its arguments resolved against the schema's parameter list.</summary>
internal sealed class EffectLiteral
{
    private readonly FluentPredicate _predicate;
    private readonly int[] _slots;
    private readonly Constant?[] _constants;
    private readonly Constant[] _arguments;

    /// <summary>The distinct parameters this literal mentions, ascending.</summary>
    public int[] Parameters { get; }

    public FluentPredicate Predicate => _predicate;

    public EffectLiteral(Atom<Fluent> atom, IReadOnlyDictionary<Variable, int> slots)
    {
        _predicate = atom.Predicate;
        _slots = new int[atom.Arguments.Count];
        _constants = new Constant?[atom.Arguments.Count];
        _arguments = new Constant[atom.Arguments.Count];

        var parameters = new SortedSet<int>();
        for (int position = 0; position < atom.Arguments.Count; position++)
        {
            _slots[position] = -1;
            if (atom.Arguments[position] is Variable variable)
            {
                _slots[position] = slots[variable];
                parameters.Add(_slots[position]);
                continue;
            }

            _constants[position] = (Constant)atom.Arguments[position];
        }

        Parameters = parameters.ToArray();
    }

    /// <summary>
    /// The fluent this literal denotes under the binding, or null when no such fact is registered — which means
    /// it has never been true and so is new to any novelty table.
    /// </summary>
    public FluentIndex? Resolve(InstanceContext context, ReadOnlySpan<Constant?> binding)
    {
        for (int position = 0; position < _slots.Length; position++)
        {
            int parameter = _slots[position];
            Constant? argument = parameter < 0 ? _constants[position] : binding[parameter];
            if (argument is null)
                throw new InvalidOperationException("An effect literal was resolved against an incomplete binding.");
            _arguments[position] = argument;
        }

        FactIndex? factIndex = context.GetFactIndex(_predicate, _arguments);
        if (factIndex is null) return null;
        return new FluentIndex(((Fact<Fluent>)context.GetFact(factIndex.Value)).LocalIndex);
    }
}
