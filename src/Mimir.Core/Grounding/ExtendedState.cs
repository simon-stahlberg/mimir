namespace Mimir.Core.Grounding;

using Engines;
using Schemas;

public sealed class ExtendedState : IEquatable<ExtendedState>
{
    private readonly DerivedPredicateEvaluation? _derivedEvaluation;

    public State State { get; }

    internal ExtendedState(
        State state,
        DerivedPredicateEvaluation? derivedEvaluation)
    {
        State = state ?? throw new ArgumentNullException(nameof(state));
        _derivedEvaluation = derivedEvaluation;
    }

    public bool IsTrue(Fact fact)
    {
        ArgumentNullException.ThrowIfNull(fact);
        if (!ReferenceEquals(fact.Context, State.Context))
            throw new ArgumentException(nameof(fact));

        return fact switch
        {
            Fact<Fluent> fluent => State.IsTrue(fluent),
            Fact<Static> equality
                when State.Context.Problem.Domain.IsEqualityPredicate(equality.Predicate)
                => ReferenceEquals(equality.Arguments[0], equality.Arguments[1]),
            Fact<Static> stat => State.Context.IsTrue(new StaticIndex(stat.LocalIndex)),
            Fact<Derived> derived => IsDerivedTruePrevalidated(derived),
            _ => throw new NotSupportedException(fact.GetType().Name)
        };
    }

    internal bool IsDerivedTruePrevalidated(Fact<Derived> fact)
        => State.Context.IsDerivedTruePrevalidated(
            fact,
            _derivedEvaluation);

    internal bool IsStateDependentDerivedTruePrevalidated(Fact<Derived> fact)
        => State.Context.IsStateDependentDerivedTruePrevalidated(
            fact,
            _derivedEvaluation);

    internal bool IsStateDependentDerivedTruePrevalidated(
        Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments)
        => State.Context.IsStateDependentDerivedTruePrevalidated(
            predicate,
            arguments,
            _derivedEvaluation);

    public bool Equals(ExtendedState? other)
        => other is not null && State.Equals(other.State);

    public override bool Equals(object? obj)
        => obj is ExtendedState other && Equals(other);

    public override int GetHashCode() => State.GetHashCode();

    public State Apply(Action action)
    {
        ArgumentNullException.ThrowIfNull(action);
        if (!ReferenceEquals(action.Context, State.Context))
            throw new InvalidOperationException(nameof(action));

        int length = Math.Max(
            State.Bitboard.Length,
            action.AddEffects.EndWordExclusive);
        List<GroundConditionalEffect>? triggeredEffects = null;
        foreach (GroundConditionalEffect effect in action.ConditionalEffects)
        {
            if (!effect.IsSatisfied(this))
                continue;

            triggeredEffects ??= new List<GroundConditionalEffect>();
            triggeredEffects.Add(effect);
            if (effect.Effect.Polarity == Polarity.Positive)
            {
                length = Math.Max(
                    length,
                    effect.Effect.Value.LocalIndex / 64 + 1);
            }
        }

        var result = new ulong[length];
        State.Bitboard.CopyTo(result);

        ApplyDeletes(action, triggeredEffects, result);
        ApplyAdds(action, triggeredEffects, result);
        return new State(
            State.Context,
            State.TrimTrailingZeros(result),
            takeOwnership: true);
    }

    private static void ApplyDeletes(
        Action action,
        IReadOnlyList<GroundConditionalEffect>? triggeredEffects,
        ulong[] result)
    {
        for (int i = 0; i < action.DeleteEffects.WordCount; i++)
        {
            int wordIndex = action.DeleteEffects.WordOffset + i;
            if (wordIndex >= result.Length)
                break;
            result[wordIndex] &= ~action.DeleteEffects[i];
        }

        if (triggeredEffects is null)
            return;

        for (int i = 0; i < triggeredEffects.Count; i++)
        {
            GroundConditionalEffect effect = triggeredEffects[i];
            if (effect.Effect.Polarity != Polarity.Negative)
                continue;

            int localIndex = effect.Effect.Value.LocalIndex;
            int wordIndex = localIndex / 64;
            if (wordIndex < result.Length)
                result[wordIndex] &= ~(1UL << (localIndex % 64));
        }
    }

    private static void ApplyAdds(
        Action action,
        IReadOnlyList<GroundConditionalEffect>? triggeredEffects,
        ulong[] result)
    {
        for (int i = 0; i < action.AddEffects.WordCount; i++)
        {
            int wordIndex = action.AddEffects.WordOffset + i;
            result[wordIndex] |= action.AddEffects[i];
        }

        if (triggeredEffects is null)
            return;

        for (int i = 0; i < triggeredEffects.Count; i++)
        {
            GroundConditionalEffect effect = triggeredEffects[i];
            if (effect.Effect.Polarity != Polarity.Positive)
                continue;

            int localIndex = effect.Effect.Value.LocalIndex;
            result[localIndex / 64] |= 1UL << (localIndex % 64);
        }
    }
}
