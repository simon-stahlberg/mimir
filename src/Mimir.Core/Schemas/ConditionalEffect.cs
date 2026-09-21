namespace Mimir.Core.Schemas;

using Grounding;

/// <summary>
/// A conditional effect in canonical form: a conjunction of literal conditions
/// guarding a single effect literal. Unconditional effects have an empty Conditions list.
/// Quantified variables come from forall wrappers in the original PDDL.
/// </summary>
public class ConditionalEffect
{
    public IReadOnlyList<Variable> QuantifiedVariables { get; }
    public IReadOnlyList<Literal<Atom<Fluent>>> FluentConditions { get; }
    public IReadOnlyList<Literal<Atom<Static>>> StaticConditions { get; }
    public IReadOnlyList<Literal<Atom<Derived>>> DerivedConditions { get; }
    public SchemaCondition Condition => new(StaticConditions.Cast<Literal>().Concat(FluentConditions).Concat(DerivedConditions));
    public ActionEffect Effect => new([EffectLiteral]);
    internal bool IsUnconditional => QuantifiedVariables.Count == 0 && StaticConditions.Count == 0
        && FluentConditions.Count == 0 && DerivedConditions.Count == 0;
    public Literal<Atom<Fluent>> EffectLiteral { get; }

    public ConditionalEffect(
        IReadOnlyList<Variable> quantifiedVariables,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        Literal<Atom<Fluent>> effect)
    {
        ArgumentNullException.ThrowIfNull(effect);

        QuantifiedVariables = CopyCollection(quantifiedVariables, nameof(quantifiedVariables));
        FluentConditions = CopyCollection(fluentConditions, nameof(fluentConditions));
        StaticConditions = CopyCollection(staticConditions, nameof(staticConditions));
        DerivedConditions = CopyCollection(derivedConditions, nameof(derivedConditions));
        EffectLiteral = effect;
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
