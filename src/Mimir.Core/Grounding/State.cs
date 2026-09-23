namespace Mimir.Core.Grounding;

using Schemas;
using System.Runtime.CompilerServices;

public class State : IEquatable<State>
{
    public bool Value(Fact atom) => Expand().IsTrue(atom);
    public double Value(NumericExpression expression) => NumericEvaluation.Evaluate(this, expression);
    public bool Holds(Fact atom) => Value(atom);
    public bool Holds(Literal<Fact> literal) => Value(literal.Value) == literal.IsPositive;
    public bool Holds(GroundConjunctiveCondition condition)
    {
        ArgumentNullException.ThrowIfNull(condition);
        if (!ReferenceEquals(condition.Problem, Context.Problem))
            throw new ArgumentException("Condition belongs to a different problem.");
        return condition.Literals.All(Holds) && condition.NumericConditions.All(Holds);
    }

    public bool Holds(NumericComparison comparison)
    {
        ArgumentNullException.ThrowIfNull(comparison);
        return NumericEvaluation.Compare(comparison.Operator, Value(comparison.Left), Value(comparison.Right));
    }

    private readonly ulong[] _bitboard;
    private readonly double[] _numericValues;
    private readonly int _hashCode;

    public InstanceContext Context { get; }
    internal ReadOnlySpan<ulong> Bitboard => _bitboard;
    internal ReadOnlySpan<double> NumericValues => _numericValues;

    internal State(InstanceContext context, ulong[] bitboard,
        double[] numericValues, bool takeOwnership = false)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(bitboard);
        ArgumentNullException.ThrowIfNull(numericValues);
        if (numericValues.Length != context.NumericStateSize)
            throw new ArgumentException($"State requires {context.NumericStateSize} numeric values, received {numericValues.Length}.", nameof(numericValues));

        Context = context;
        _bitboard = takeOwnership ? bitboard : bitboard.ToArray();
        // States of problems without changing numeric fluents share one empty array instead of each owning one.
        _numericValues = numericValues.Length == 0 ? Array.Empty<double>() : takeOwnership ? numericValues : numericValues.ToArray();
        for (int index = 0; index < _numericValues.Length; index++)
            _numericValues[index] = NumericEvaluation.Quantize(_numericValues[index]);
        _hashCode = ComputeHashCode(context, _bitboard, _numericValues);
    }

    internal State(State source, double[] numericValues)
    {
        ArgumentNullException.ThrowIfNull(source);
        ArgumentNullException.ThrowIfNull(numericValues);
        if (numericValues.Length != source.Context.NumericStateSize)
            throw new ArgumentException($"State requires {source.Context.NumericStateSize} numeric values, received {numericValues.Length}.", nameof(numericValues));
        Context = source.Context;
        _bitboard = source._bitboard;
        _numericValues = numericValues;
        for (int index = 0; index < _numericValues.Length; index++)
            _numericValues[index] = NumericEvaluation.Quantize(_numericValues[index]);
        _hashCode = ComputeHashCode(Context, _bitboard, _numericValues);
    }

    internal State(State source, ulong[] bitboard, bool takeOwnership)
    {
        ArgumentNullException.ThrowIfNull(source);
        ArgumentNullException.ThrowIfNull(bitboard);
        Context = source.Context;
        _bitboard = takeOwnership ? bitboard : bitboard.ToArray();
        _numericValues = source._numericValues;
        _hashCode = ComputeHashCode(Context, _bitboard, _numericValues);
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

        return new State(this, TrimTrailingZeros(newBitboard), takeOwnership: true);
    }

    public bool Equals(State? other)
    {
        if (ReferenceEquals(this, other)) return true;
        if (other is null
            || !ReferenceEquals(other.Context, Context)
            || other._hashCode != _hashCode
            || !_numericValues.AsSpan().SequenceEqual(other._numericValues))
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
        ReadOnlySpan<ulong> bitboard,
        ReadOnlySpan<double> numericValues)
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

        foreach (double value in numericValues) hash.Add(value);
        return hash.ToHashCode();
    }
}
