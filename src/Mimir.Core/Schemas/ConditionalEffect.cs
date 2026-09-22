namespace Mimir.Core.Schemas;

using Grounding;

public class ConditionalEffect
{
    public IReadOnlyList<Variable> QuantifiedVariables { get; }
    public IReadOnlyList<Literal<Atom<Fluent>>> FluentConditions { get; }
    public IReadOnlyList<Literal<Atom<Static>>> StaticConditions { get; }
    public IReadOnlyList<Literal<Atom<Derived>>> DerivedConditions { get; }
    public IReadOnlyList<NumericComparison> NumericConditions { get; }
    public NumericUpdate? NumericEffect { get; }
    public Literal<Atom<Fluent>>? LiteralEffect { get; }
    public SchemaCondition Condition => new(StaticConditions.Cast<Literal>().Concat(FluentConditions).Concat(DerivedConditions), NumericConditions);
    public ActionEffect Effect => new(LiteralEffect is null ? [] : [LiteralEffect], NumericEffect is null ? [] : [NumericEffect]);
    internal bool IsUnconditional => QuantifiedVariables.Count == 0 && StaticConditions.Count == 0
        && FluentConditions.Count == 0 && DerivedConditions.Count == 0 && NumericConditions.Count == 0;
    // For code paths that only accept propositional effects (e.g. heuristics on non-numeric problems).
    internal Literal<Atom<Fluent>> RequiredLiteralEffect => LiteralEffect ?? throw new InvalidOperationException("This effect is numeric.");

    public ConditionalEffect(
        IReadOnlyList<Variable> quantifiedVariables,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        Literal<Atom<Fluent>>? effect,
        IReadOnlyList<NumericComparison>? numericConditions = null,
        NumericUpdate? numericEffect = null)
    {
        if (effect is null && numericEffect is null)
            throw new ArgumentNullException(nameof(effect), "An effect needs a literal or a numeric update.");
        if (effect is not null && numericEffect is not null)
            throw new ArgumentException("An effect cannot contain both a literal and a numeric update.");

        QuantifiedVariables = CopyCollection(quantifiedVariables, nameof(quantifiedVariables));
        FluentConditions = CopyCollection(fluentConditions, nameof(fluentConditions));
        StaticConditions = CopyCollection(staticConditions, nameof(staticConditions));
        DerivedConditions = CopyCollection(derivedConditions, nameof(derivedConditions));
        LiteralEffect = effect;
        NumericConditions = CopyCollection(numericConditions ?? [], nameof(numericConditions));
        NumericEffect = numericEffect;
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
