using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class ActionSchema
{
    public string Name { get; }
    public IReadOnlyList<Variable> Parameters { get; }

    public IReadOnlyList<Literal<Atom<Fluent>>> FluentPreconditions { get; }
    public IReadOnlyList<Literal<Atom<Static>>> StaticPreconditions { get; }
    public IReadOnlyList<Literal<Atom<Derived>>> DerivedPreconditions { get; }

    internal IReadOnlyList<ConditionalEffect> Effects { get; }
    public SchemaCondition Precondition => new(StaticPreconditions.Cast<Literal>().Concat(FluentPreconditions).Concat(DerivedPreconditions));
    public ActionEffect Effect => new(Effects.Where(effect => effect.IsUnconditional).Select(effect => effect.EffectLiteral));
    public IReadOnlyList<ConditionalEffect> ConditionalEffects => Array.AsReadOnly(Effects.Where(effect => !effect.IsUnconditional).ToArray());
    public NumericExpression CostExpression { get; }

    public ActionSchema(
        string name,
        IReadOnlyList<Variable> parameters,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentPreconditions,
        IReadOnlyList<Literal<Atom<Static>>> staticPreconditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedPreconditions,
        IReadOnlyList<ConditionalEffect> effects,
        NumericExpression costExpression)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentNullException.ThrowIfNull(costExpression);

        Name = name;
        Parameters = CopyCollection(parameters, nameof(parameters));
        FluentPreconditions = CopyCollection(fluentPreconditions, nameof(fluentPreconditions));
        StaticPreconditions = CopyCollection(staticPreconditions, nameof(staticPreconditions));
        DerivedPreconditions = CopyCollection(derivedPreconditions, nameof(derivedPreconditions));
        Effects = CopyCollection(effects, nameof(effects));
        CostExpression = costExpression;
    }

    private static IReadOnlyList<T> CopyCollection<T>(IReadOnlyList<T> values, string parameterName)
        where T : class
    {
        ArgumentNullException.ThrowIfNull(values, parameterName);

        T[] copy = values.ToArray();
        if (copy.Any(value => value is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return Array.AsReadOnly(copy);
    }

    public override string ToString()
    {
        if (Parameters.Count == 0) return $"({Name})";
        return $"({Name} {string.Join(" ", Parameters)})";
    }
}
