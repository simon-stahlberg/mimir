namespace Mimir.Core.Grounding;

using Schemas;
using System.Runtime.CompilerServices;
using Mimir.Core;

public abstract class GroundConditionalEffectBase
{
    public InstanceContext Context { get; }
    public IReadOnlyList<GroundNumericComparison> NumericConditions { get; }
    internal OffsetBitboard PositiveFluentConditions { get; }
    internal OffsetBitboard NegativeFluentConditions { get; }
    internal OffsetBitboard PositiveStaticConditions { get; }
    internal OffsetBitboard NegativeStaticConditions { get; }

    public IReadOnlyList<Literal<Fact<Derived>>> DerivedConditions { get; }
    public GroundConjunctiveCondition Condition => new(Context.Problem,
        GroundConditionLiterals.Read(Context, PositiveFluentConditions, NegativeFluentConditions,
            PositiveStaticConditions, NegativeStaticConditions, DerivedConditions), NumericConditions);

    private protected GroundConditionalEffectBase(
        InstanceContext context,
        OffsetBitboard positiveFluentConditions,
        OffsetBitboard negativeFluentConditions,
        OffsetBitboard positiveStaticConditions,
        OffsetBitboard negativeStaticConditions,
        IReadOnlyList<Literal<Fact<Derived>>> derivedConditions,
        IReadOnlyList<GroundNumericComparison> numericConditions)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(derivedConditions);
        ArgumentNullException.ThrowIfNull(numericConditions);

        Context = context;
        PositiveFluentConditions = positiveFluentConditions;
        NegativeFluentConditions = negativeFluentConditions;
        PositiveStaticConditions = positiveStaticConditions;
        NegativeStaticConditions = negativeStaticConditions;
        DerivedConditions = Array.AsReadOnly(derivedConditions.ToArray());
        NumericConditions = Array.AsReadOnly(numericConditions.ToArray());
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

    private protected bool ConditionEquals(GroundConditionalEffectBase other)
        => ReferenceEquals(Context, other.Context)
        && PositiveFluentConditions.Equals(other.PositiveFluentConditions)
        && NegativeFluentConditions.Equals(other.NegativeFluentConditions)
        && PositiveStaticConditions.Equals(other.PositiveStaticConditions)
        && NegativeStaticConditions.Equals(other.NegativeStaticConditions)
        && ValueSequence.Equals(DerivedConditions, other.DerivedConditions)
        && ValueSequence.Equals(NumericConditions, other.NumericConditions);

    private protected void AddConditionToHash(ref HashCode hash)
    {
        hash.Add(RuntimeHelpers.GetHashCode(Context));
        hash.Add(PositiveFluentConditions);
        hash.Add(NegativeFluentConditions);
        hash.Add(PositiveStaticConditions);
        hash.Add(NegativeStaticConditions);
        ValueSequence.AddToHash(ref hash, DerivedConditions);
        ValueSequence.AddToHash(ref hash, NumericConditions);
    }
}

public sealed class GroundConditionalEffect : GroundConditionalEffectBase, IEquatable<GroundConditionalEffect>
{
    public Literal<Fact<Fluent>> Effect { get; }

    internal GroundConditionalEffect(
        InstanceContext context,
        OffsetBitboard positiveFluentConditions,
        OffsetBitboard negativeFluentConditions,
        OffsetBitboard positiveStaticConditions,
        OffsetBitboard negativeStaticConditions,
        IReadOnlyList<Literal<Fact<Derived>>> derivedConditions,
        IReadOnlyList<GroundNumericComparison> numericConditions,
        Literal<Fact<Fluent>> effect)
        : base(context, positiveFluentConditions, negativeFluentConditions, positiveStaticConditions,
            negativeStaticConditions, derivedConditions, numericConditions)
    {
        Effect = effect ?? throw new ArgumentNullException(nameof(effect));
    }

    public bool Equals(GroundConditionalEffect? other)
        => other is not null && ConditionEquals(other) && Effect.Equals(other.Effect);

    public override bool Equals(object? obj) => Equals(obj as GroundConditionalEffect);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        AddConditionToHash(ref hash);
        hash.Add(Effect);
        return hash.ToHashCode();
    }
}

public sealed class GroundConditionalNumericEffect : GroundConditionalEffectBase, IEquatable<GroundConditionalNumericEffect>
{
    public GroundNumericUpdate Effect { get; }

    internal GroundConditionalNumericEffect(
        InstanceContext context,
        OffsetBitboard positiveFluentConditions,
        OffsetBitboard negativeFluentConditions,
        OffsetBitboard positiveStaticConditions,
        OffsetBitboard negativeStaticConditions,
        IReadOnlyList<Literal<Fact<Derived>>> derivedConditions,
        IReadOnlyList<GroundNumericComparison> numericConditions,
        GroundNumericUpdate effect)
        : base(context, positiveFluentConditions, negativeFluentConditions, positiveStaticConditions,
            negativeStaticConditions, derivedConditions, numericConditions)
    {
        Effect = effect ?? throw new ArgumentNullException(nameof(effect));
    }

    public bool Equals(GroundConditionalNumericEffect? other)
        => other is not null && ConditionEquals(other) && Effect.Equals(other.Effect);

    public override bool Equals(object? obj) => Equals(obj as GroundConditionalNumericEffect);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        AddConditionToHash(ref hash);
        hash.Add(Effect);
        return hash.ToHashCode();
    }
}
