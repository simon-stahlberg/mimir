namespace Mimir.Core.Schemas;

using Grounding;

public abstract class ConditionalEffectBase
{
    public IReadOnlyList<Variable> QuantifiedVariables { get; }
    public IReadOnlyList<Literal<Atom<Fluent>>> FluentConditions { get; }
    public IReadOnlyList<Literal<Atom<Static>>> StaticConditions { get; }
    public IReadOnlyList<Literal<Atom<Derived>>> DerivedConditions { get; }
    public IReadOnlyList<NumericComparison> NumericConditions { get; }
    public SchemaCondition Condition => new(StaticConditions.Cast<Literal>().Concat(FluentConditions).Concat(DerivedConditions), NumericConditions);
    internal bool IsUnconditional => QuantifiedVariables.Count == 0 && StaticConditions.Count == 0
        && FluentConditions.Count == 0 && DerivedConditions.Count == 0 && NumericConditions.Count == 0;

    private protected ConditionalEffectBase(
        IReadOnlyList<Variable> quantifiedVariables,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        IReadOnlyList<NumericComparison> numericConditions)
    {
        QuantifiedVariables = CopyCollection(quantifiedVariables, nameof(quantifiedVariables));
        FluentConditions = CopyCollection(fluentConditions, nameof(fluentConditions));
        StaticConditions = CopyCollection(staticConditions, nameof(staticConditions));
        DerivedConditions = CopyCollection(derivedConditions, nameof(derivedConditions));
        NumericConditions = CopyCollection(numericConditions, nameof(numericConditions));
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
}

public sealed class ConditionalEffect : ConditionalEffectBase
{
    public Literal<Atom<Fluent>> Effect { get; }

    public ConditionalEffect(
        IReadOnlyList<Variable> quantifiedVariables,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        IReadOnlyList<NumericComparison> numericConditions,
        Literal<Atom<Fluent>> effect)
        : base(quantifiedVariables, fluentConditions, staticConditions, derivedConditions, numericConditions)
    {
        Effect = effect ?? throw new ArgumentNullException(nameof(effect));
    }
}

public sealed class ConditionalNumericEffect : ConditionalEffectBase
{
    public NumericUpdate Effect { get; }

    public ConditionalNumericEffect(
        IReadOnlyList<Variable> quantifiedVariables,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        IReadOnlyList<NumericComparison> numericConditions,
        NumericUpdate effect)
        : base(quantifiedVariables, fluentConditions, staticConditions, derivedConditions, numericConditions)
    {
        Effect = effect ?? throw new ArgumentNullException(nameof(effect));
    }
}
