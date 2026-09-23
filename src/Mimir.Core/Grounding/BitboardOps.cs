using System.Numerics;

namespace Mimir.Core.Grounding;

internal readonly struct OffsetBitboard : IEquatable<OffsetBitboard>
{
    private readonly ulong[]? _words;

    public int WordOffset { get; }
    public int WordCount => _words?.Length ?? 0;
    public int EndWordExclusive => WordOffset + WordCount;
    public bool IsEmpty => WordCount == 0;
    public ReadOnlySpan<ulong> Words => _words is null ? ReadOnlySpan<ulong>.Empty : _words;

    public ulong this[int relativeWordIndex]
    {
        get
        {
            if ((uint)relativeWordIndex >= (uint)WordCount)
                throw new ArgumentOutOfRangeException(nameof(relativeWordIndex));

            return _words![relativeWordIndex];
        }
    }

    private OffsetBitboard(int wordOffset, ulong[] words)
    {
        WordOffset = wordOffset;
        _words = words;
    }

    public static OffsetBitboard FromDense(ulong[] words)
    {
        ArgumentNullException.ThrowIfNull(words);

        int firstNonZero = 0;
        while (firstNonZero < words.Length && words[firstNonZero] == 0)
            firstNonZero++;

        if (firstNonZero == words.Length)
            return default;

        int lastNonZero = words.Length - 1;
        while (words[lastNonZero] == 0)
            lastNonZero--;

        int wordCount = lastNonZero - firstNonZero + 1;
        var trimmedWords = new ulong[wordCount];
        Array.Copy(words, firstNonZero, trimmedWords, 0, wordCount);
        return new OffsetBitboard(firstNonZero, trimmedWords);
    }

    public static OffsetBitboard FromSetBitIndices(IReadOnlyList<int> setBitIndices)
    {
        ArgumentNullException.ThrowIfNull(setBitIndices);
        if (setBitIndices.Count == 0)
            return default;

        int firstWord = int.MaxValue;
        int lastWord = -1;
        for (int i = 0; i < setBitIndices.Count; i++)
        {
            int bitIndex = setBitIndices[i];
            if (bitIndex < 0)
                throw new ArgumentOutOfRangeException(
                    nameof(setBitIndices), bitIndex, "Set-bit indices cannot be negative.");

            int wordIndex = bitIndex / 64;
            firstWord = Math.Min(firstWord, wordIndex);
            lastWord = Math.Max(lastWord, wordIndex);
        }

        var words = new ulong[lastWord - firstWord + 1];
        for (int i = 0; i < setBitIndices.Count; i++)
        {
            int bitIndex = setBitIndices[i];
            int relativeWordIndex = bitIndex / 64 - firstWord;
            words[relativeWordIndex] |= 1UL << (bitIndex % 64);
        }

        return new OffsetBitboard(firstWord, words);
    }

    public bool Equals(OffsetBitboard other)
    {
        if (WordOffset != other.WordOffset || WordCount != other.WordCount)
            return false;

        for (int i = 0; i < WordCount; i++)
        {
            if (this[i] != other[i])
                return false;
        }

        return true;
    }

    public override bool Equals(object? obj) => obj is OffsetBitboard other && Equals(other);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(WordOffset);
        hash.Add(WordCount);
        for (int i = 0; i < WordCount; i++)
            hash.Add(this[i]);
        return hash.ToHashCode();
    }

    public static bool operator ==(OffsetBitboard left, OffsetBitboard right) => left.Equals(right);
    public static bool operator !=(OffsetBitboard left, OffsetBitboard right) => !left.Equals(right);
}

internal readonly struct OffsetSetBitEnumerable
{
    private readonly OffsetBitboard _bits;

    public OffsetSetBitEnumerable(OffsetBitboard bits)
    {
        _bits = bits;
    }

    public Enumerator GetEnumerator() => new(_bits);

    internal struct Enumerator
    {
        private readonly OffsetBitboard _bits;
        private int _relativeWordIndex;
        private ulong _remainingWord;

        public int Current { get; private set; }

        public Enumerator(OffsetBitboard bits)
        {
            _bits = bits;
            _relativeWordIndex = -1;
            _remainingWord = 0;
            Current = 0;
        }

        public bool MoveNext()
        {
            while (_remainingWord == 0)
            {
                _relativeWordIndex++;
                if (_relativeWordIndex >= _bits.WordCount)
                    return false;

                _remainingWord = _bits[_relativeWordIndex];
            }

            int bitIndex = BitOperations.TrailingZeroCount(_remainingWord);
            Current = (_bits.WordOffset + _relativeWordIndex) * 64 + bitIndex;
            _remainingWord &= _remainingWord - 1;
            return true;
        }
    }
}

/// <summary>
/// Shared bit-twiddling helpers for the bitboards used throughout grounding,
/// search, and interop.
/// </summary>
internal static class BitboardOps
{
    /// <summary>Count set bits across all words.</summary>
    public static int PopCount(ulong[] bits)
    {
        ArgumentNullException.ThrowIfNull(bits);
        return PopCount(bits.AsSpan());
    }

    public static int PopCount(ReadOnlySpan<ulong> bits)
    {
        int sum = 0;
        for (int i = 0; i < bits.Length; i++)
            sum += BitOperations.PopCount(bits[i]);
        return sum;
    }

    public static int PopCount(OffsetBitboard bits)
    {
        int sum = 0;
        for (int i = 0; i < bits.WordCount; i++)
            sum += BitOperations.PopCount(bits[i]);
        return sum;
    }

    /// <summary>Yield each set-bit local index in ascending order.</summary>
    public static IEnumerable<int> EnumerateSetBits(ulong[] bits)
    {
        ArgumentNullException.ThrowIfNull(bits);
        for (int wordIndex = 0; wordIndex < bits.Length; wordIndex++)
        {
            ulong word = bits[wordIndex];
            while (word != 0)
            {
                int bitIndex = BitOperations.TrailingZeroCount(word);
                yield return wordIndex * 64 + bitIndex;
                word &= word - 1;
            }
        }
    }

    public static OffsetSetBitEnumerable EnumerateSetBits(OffsetBitboard bits)
        => new(bits);

    /// <summary>Eagerly materialize all set-bit indices into an int[].</summary>
    public static int[] DecodeSetBits(ulong[] bits)
    {
        ArgumentNullException.ThrowIfNull(bits);
        var result = new int[PopCount(bits)];
        int k = 0;
        foreach (int idx in EnumerateSetBits(bits)) result[k++] = idx;
        return result;
    }

    public static int[] DecodeSetBits(OffsetBitboard bits)
    {
        var result = new int[PopCount(bits)];
        int resultIndex = 0;
        foreach (int bitIndex in EnumerateSetBits(bits))
            result[resultIndex++] = bitIndex;
        return result;
    }

    /// <summary>
    /// Return the bit position of the <paramref name="n"/>-th set bit (0-based),
    /// or -1 if there are fewer than <c>n + 1</c> set bits.
    /// </summary>
    public static int NthSetBitIndex(ulong[] bits, int n)
    {
        ArgumentNullException.ThrowIfNull(bits);
        return NthSetBitIndex(bits.AsSpan(), n);
    }

    public static int NthSetBitIndex(ReadOnlySpan<ulong> bits, int n)
    {
        if (n < 0) return -1;
        int seen = 0;
        for (int wordIndex = 0; wordIndex < bits.Length; wordIndex++)
        {
            ulong word = bits[wordIndex];
            while (word != 0)
            {
                if (seen == n)
                    return wordIndex * 64 + BitOperations.TrailingZeroCount(word);
                seen++;
                word &= word - 1;
            }
        }
        return -1;
    }

    public static int NthSetBitIndex(OffsetBitboard bits, int n)
    {
        if (n < 0) return -1;

        int seen = 0;
        for (int relativeWordIndex = 0; relativeWordIndex < bits.WordCount; relativeWordIndex++)
        {
            ulong word = bits[relativeWordIndex];
            while (word != 0)
            {
                if (seen == n)
                {
                    int wordIndex = bits.WordOffset + relativeWordIndex;
                    return wordIndex * 64 + BitOperations.TrailingZeroCount(word);
                }

                seen++;
                word &= word - 1;
            }
        }

        return -1;
    }

    /// <summary>
    /// True if <paramref name="positiveStatic"/> is a subset of, and
    /// <paramref name="negativeStatic"/> is disjoint from,
    /// <paramref name="staticBitboard"/>.
    /// </summary>
    public static bool StaticPreconditionHolds(
        OffsetBitboard positiveStatic,
        OffsetBitboard negativeStatic,
        ulong[] staticBitboard)
    {
        ArgumentNullException.ThrowIfNull(staticBitboard);
        return StaticPreconditionHolds(
            positiveStatic,
            negativeStatic,
            staticBitboard.AsSpan());
    }

    public static bool StaticPreconditionHolds(
        OffsetBitboard positiveStatic,
        OffsetBitboard negativeStatic,
        ReadOnlySpan<ulong> staticBitboard)
    {

        for (int i = 0; i < positiveStatic.WordCount; i++)
        {
            ulong required = positiveStatic[i];
            int wordIndex = positiveStatic.WordOffset + i;
            ulong have = wordIndex < staticBitboard.Length ? staticBitboard[wordIndex] : 0UL;
            if ((have & required) != required) return false;
        }
        for (int i = 0; i < negativeStatic.WordCount; i++)
        {
            ulong forbidden = negativeStatic[i];
            int wordIndex = negativeStatic.WordOffset + i;
            ulong have = wordIndex < staticBitboard.Length ? staticBitboard[wordIndex] : 0UL;
            if ((have & forbidden) != 0) return false;
        }
        return true;
    }
}
