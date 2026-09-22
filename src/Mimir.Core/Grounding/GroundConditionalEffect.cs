namespace Mimir.Core.Grounding;

using Schemas;
using System.Runtime.CompilerServices;
using Mimir.Core;

public class GroundConditionalEffect : IEquatable<GroundConditionalEffect>
{
    public InstanceContext Context { get; }
    public IReadOnlyList<GroundNumericComparison> NumericConditions { get; }
    public GroundNumericUpdate? NumericEffect { get; }
    public Literal<Fact<Fluent>>? LiteralEffect { get; }
    internal OffsetBitboard PositiveFluentConditions { get; }
    internal OffsetBitboard NegativeFluentConditions { get; }
    internal OffsetBitboard PositiveStaticConditions { get; }
    internal OffsetBitboard NegativeStaticConditions { get; }

    public IReadOnlyList<Literal<Fact<Derived>>> DerivedConditions { get; }
    public GroundConjunctiveCondition Condition => new(Context.Problem,
        GroundConditionLiterals.Read(Context, PositiveFluentConditions, NegativeFluentConditions,
            PositiveStaticConditions, NegativeStaticConditions, DerivedConditions), NumericConditions);
    public GroundActionEffect Effect => new(LiteralEffect is null ? [] : [new Literal<Fact>(LiteralEffect.Value, LiteralEffect.Polarity)], NumericEffect is null ? [] : [NumericEffect]);
    // For code paths that only accept propositional effects (e.g. heuristics on non-numeric problems).
    internal Literal<Fact<Fluent>> RequiredLiteralEffect => LiteralEffect ?? throw new InvalidOperationException("This effect is numeric.");

    internal GroundConditionalEffect(
        InstanceContext context,
        OffsetBitboard positiveFluentConditions,
        OffsetBitboard negativeFluentConditions,
        OffsetBitboard positiveStaticConditions,
        OffsetBitboard negativeStaticConditions,
        IReadOnlyList<Literal<Fact<Derived>>> derivedConditions,
        Literal<Fact<Fluent>>? effect,
        IReadOnlyList<GroundNumericComparison>? numericConditions = null,
        GroundNumericUpdate? numericEffect = null)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(derivedConditions);
        if (effect is null && numericEffect is null)
            throw new ArgumentNullException(nameof(effect), "An effect needs a literal or a numeric update.");
        if (effect is not null && numericEffect is not null)
            throw new ArgumentException("An effect cannot contain both a literal and a numeric update.");

        Context = context;
        PositiveFluentConditions = positiveFluentConditions;
        NegativeFluentConditions = negativeFluentConditions;
        PositiveStaticConditions = positiveStaticConditions;
        NegativeStaticConditions = negativeStaticConditions;
        DerivedConditions = Array.AsReadOnly(derivedConditions.ToArray());
        LiteralEffect = effect;
        NumericEffect = numericEffect;
        NumericConditions = numericConditions is null || numericConditions.Count == 0
            ? Array.Empty<GroundNumericComparison>() : Array.AsReadOnly(numericConditions.ToArray());
    }

    public bool IsSatisfied(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        State compactState = state.State;
        if (!ReferenceEquals(compactState.Context, Context))
            throw new InvalidOperationException(
                "Cannot evaluate a conditional effect against a state from a different InstanceContext.");

        if (!BitboardOps.StaticPreconditionHolds(
                PositiveStaticConditions,
                NegativeStaticConditions,
                compactState.Context.StaticBitboardWords))
            return false;

        if (!compactState.ContainsAll(PositiveFluentConditions))
            return false;

        if (!compactState.ContainsNone(NegativeFluentConditions))
            return false;

        for (int i = 0; i < DerivedConditions.Count; i++)
        {
            Literal<Fact<Derived>> literal = DerivedConditions[i];
            if (state.IsDerivedTruePrevalidated(literal.Value)
                != literal.IsPositive)
            {
                return false;
            }
        }

        foreach (GroundNumericComparison comparison in NumericConditions)
            if (!compactState.Holds(comparison)) return false;
        return true;
    }

    public bool Equals(GroundConditionalEffect? other)
        => other is not null
        && ReferenceEquals(Context, other.Context)
        && PositiveFluentConditions.Equals(other.PositiveFluentConditions)
        && NegativeFluentConditions.Equals(other.NegativeFluentConditions)
        && PositiveStaticConditions.Equals(other.PositiveStaticConditions)
        && NegativeStaticConditions.Equals(other.NegativeStaticConditions)
        && ValueSequence.Equals(DerivedConditions, other.DerivedConditions)
        && Equals(LiteralEffect, other.LiteralEffect)
        && Equals(NumericEffect, other.NumericEffect)
        && ValueSequence.Equals(NumericConditions, other.NumericConditions);

    public override bool Equals(object? obj) => Equals(obj as GroundConditionalEffect);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(Context));
        hash.Add(PositiveFluentConditions);
        hash.Add(NegativeFluentConditions);
        hash.Add(PositiveStaticConditions);
        hash.Add(NegativeStaticConditions);
        ValueSequence.AddToHash(ref hash, DerivedConditions);
        hash.Add(LiteralEffect);
        hash.Add(NumericEffect);
        ValueSequence.AddToHash(ref hash, NumericConditions);
        return hash.ToHashCode();
    }
}
