namespace Mimir.Core.Grounding;

using Schemas;
using System.Runtime.CompilerServices;

public class State : IEquatable<State>
{
    private readonly ulong[] _bitboard;
    private readonly int _hashCode;

    public InstanceContext Context { get; }
    internal ReadOnlySpan<ulong> Bitboard => _bitboard;

    internal State(InstanceContext context, ulong[] bitboard)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(bitboard);

        Context = context;
        _bitboard = bitboard.ToArray();
        _hashCode = ComputeHashCode(context, _bitboard);
    }

    internal State(InstanceContext context, ulong[] bitboard, bool takeOwnership)
    {
        Context = context;
        _bitboard = bitboard;
        _hashCode = ComputeHashCode(context, _bitboard);
    }

    internal State(InstanceContext context)
    {
        Context = context ?? throw new ArgumentNullException(nameof(context));
        _bitboard = Array.Empty<ulong>();
        _hashCode = ComputeHashCode(Context, _bitboard);
    }

    internal bool IsTrue(FluentIndex factIndex)
    {
        if (!Context.IsValid(factIndex))
        {
            throw new ArgumentOutOfRangeException(
                nameof(factIndex),
                factIndex.Value,
                "The fluent index does not reference a registered fluent fact.");
        }

        return IsTrueUnchecked(factIndex);
    }

    internal bool IsTrueUnchecked(FluentIndex factIndex)
    {
        int arrayIndex = factIndex.Value / 64;
        int bitIndex = factIndex.Value % 64;
        if (arrayIndex >= _bitboard.Length) return false;

        ulong mask = 1UL << bitIndex;
        return (_bitboard[arrayIndex] & mask) != 0;
    }

    public bool IsTrue(Fact<Fluent> fact)
    {
        ArgumentNullException.ThrowIfNull(fact);
        if (!ReferenceEquals(fact.Context, Context))
            throw new ArgumentException("Fact belongs to a different problem.", nameof(fact));
        return IsTrue(new FluentIndex(fact.LocalIndex));
    }

    public ExtendedState Expand() => Context.Expand(this);

    internal bool ContainsAll(OffsetBitboard required)
    {
        for (int i = 0; i < required.WordCount; i++)
        {
            ulong mask = required[i];
            int wordIndex = required.WordOffset + i;
            ulong stateBits = wordIndex < _bitboard.Length ? _bitboard[wordIndex] : 0UL;
            if ((stateBits & mask) != mask)
                return false;
        }

        return true;
    }

    internal bool ContainsNone(OffsetBitboard forbidden)
    {
        for (int i = 0; i < forbidden.WordCount; i++)
        {
            ulong mask = forbidden[i];
            int wordIndex = forbidden.WordOffset + i;
            ulong stateBits = wordIndex < _bitboard.Length ? _bitboard[wordIndex] : 0UL;
            if ((stateBits & mask) != 0)
                return false;
        }

        return true;
    }

    public IEnumerable<Fact<Fluent>> GetTrueFacts()
    {
        for (int i = 0; i < _bitboard.Length; i++)
        {
            ulong word = _bitboard[i];
            if (word == 0) continue;

            for (int b = 0; b < 64; b++)
            {
                if ((word & (1UL << b)) != 0)
                {
                    int localId = i * 64 + b;
                    var fluentIndex = new FluentIndex(localId);
                    if (!Context.IsValid(fluentIndex))
                        throw new InvalidOperationException(
                            $"State contains an unregistered fluent fact index {localId}.");

                    yield return Context.GetFact(fluentIndex);
                }
            }
        }
    }

    internal static ulong[] TrimTrailingZeros(ulong[] bitboard)
    {
        int trimmed = bitboard.Length;
        while (trimmed > 0 && bitboard[trimmed - 1] == 0)
            trimmed--;
        if (trimmed == bitboard.Length)
            return bitboard;
        var result = new ulong[trimmed];
        Array.Copy(bitboard, result, trimmed);
        return result;
    }

    public State WithAdditionalFluentFacts(IEnumerable<Fact<Fluent>> additionalFacts)
    {
        ArgumentNullException.ThrowIfNull(additionalFacts);
        return WithAdditionalFluentFacts(additionalFacts.ToArray());
    }

    internal State WithAdditionalFluentFacts(IReadOnlyList<Fact<Fluent>> additionalFacts)
    {
        ArgumentNullException.ThrowIfNull(additionalFacts);

        if (additionalFacts.Count == 0)
            return this;

        int requiredLength = _bitboard.Length;
        for (int i = 0; i < additionalFacts.Count; i++)
        {
            Fact<Fluent> fact = additionalFacts[i];
            if (fact is null)
                throw new ArgumentException("Additional facts cannot contain null values.", nameof(additionalFacts));
            if (!ReferenceEquals(fact.Context, Context))
                throw new ArgumentException("A fact belongs to a different problem.", nameof(additionalFacts));

            requiredLength = Math.Max(requiredLength, (fact.LocalIndex / 64) + 1);
        }

        var newBitboard = new ulong[requiredLength];
        Array.Copy(_bitboard, newBitboard, _bitboard.Length);

        for (int i = 0; i < additionalFacts.Count; i++)
        {
            Fact<Fluent> fact = additionalFacts[i];
            int arrayIndex = fact.LocalIndex / 64;
            int bitIndex = fact.LocalIndex % 64;
            newBitboard[arrayIndex] |= 1UL << bitIndex;
        }

        return new State(Context, TrimTrailingZeros(newBitboard), takeOwnership: true);
    }

    public bool Equals(State? other)
    {
        if (ReferenceEquals(this, other)) return true;
        if (other is null
            || !ReferenceEquals(other.Context, Context)
            || other._hashCode != _hashCode)
        {
            return false;
        }

        int commonLength = Math.Min(_bitboard.Length, other._bitboard.Length);
        for (int i = 0; i < commonLength; i++)
        {
            if (_bitboard[i] != other._bitboard[i]) return false;
        }

        for (int i = commonLength; i < _bitboard.Length; i++)
        {
            if (_bitboard[i] != 0) return false;
        }

        for (int i = commonLength; i < other._bitboard.Length; i++)
        {
            if (other._bitboard[i] != 0) return false;
        }

        return true;
    }

    public override bool Equals(object? obj) => Equals(obj as State);

    public override int GetHashCode() => _hashCode;

    private static int ComputeHashCode(
        InstanceContext context,
        ReadOnlySpan<ulong> bitboard)
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(context));

        int lastNonZero = bitboard.Length - 1;
        while (lastNonZero >= 0 && bitboard[lastNonZero] == 0)
        {
            lastNonZero--;
        }

        for (int i = 0; i <= lastNonZero; i++)
        {
            hash.Add(bitboard[i]);
        }

        return hash.ToHashCode();
    }
}
