namespace Mimir.Core.Grounding;

using Schemas;
using System.Runtime.CompilerServices;
using Mimir.Core;

public class GroundConditionalEffect : IEquatable<GroundConditionalEffect>
{
    public InstanceContext Context { get; }
    internal OffsetBitboard PositiveFluentConditions { get; }
    internal OffsetBitboard NegativeFluentConditions { get; }
    internal OffsetBitboard PositiveStaticConditions { get; }
    internal OffsetBitboard NegativeStaticConditions { get; }

    public IReadOnlyList<Literal<Fact<Derived>>> DerivedConditions { get; }
    public GroundConjunctiveCondition Condition => new(Context.Problem,
        GroundConditionLiterals.Read(Context, PositiveFluentConditions, NegativeFluentConditions,
            PositiveStaticConditions, NegativeStaticConditions, DerivedConditions));
    public GroundActionEffect Effect => new([new Literal<Fact>(EffectLiteral.Value, EffectLiteral.Polarity)]);
    public Literal<Fact<Fluent>> EffectLiteral { get; }

    internal GroundConditionalEffect(
        InstanceContext context,
        OffsetBitboard positiveFluentConditions,
        OffsetBitboard negativeFluentConditions,
        OffsetBitboard positiveStaticConditions,
        OffsetBitboard negativeStaticConditions,
        IReadOnlyList<Literal<Fact<Derived>>> derivedConditions,
        Literal<Fact<Fluent>> effect)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(derivedConditions);
        ArgumentNullException.ThrowIfNull(effect);

        Context = context;
        PositiveFluentConditions = positiveFluentConditions;
        NegativeFluentConditions = negativeFluentConditions;
        PositiveStaticConditions = positiveStaticConditions;
        NegativeStaticConditions = negativeStaticConditions;
        DerivedConditions = Array.AsReadOnly(derivedConditions.ToArray());
        EffectLiteral = effect;
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
        && EqualityComparer<Literal<Fact<Fluent>>>.Default.Equals(EffectLiteral, other.EffectLiteral);

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
        hash.Add(EffectLiteral);
        return hash.ToHashCode();
    }
}
