namespace Mimir.Core.Grounding;

/// <summary>
/// Projected relations for one chosen state, which a factored generator can adapt to a nearby state instead of
/// rebuilding from scratch.
/// <para>
/// Answering a state means knowing which ground atoms satisfy each precondition, and that costs the size of every
/// predicate's extension. A state a search visits differs from a reference state by the effects of the actions
/// between them, so patching those few rows and putting them back is cheaper whenever the two are close.
/// </para>
/// <para>
/// The reference is fixed when the cache is created and never moves: a call borrows the relations and returns
/// them as it found them. Which state to anchor on is therefore the caller's decision rather than a consequence
/// of call order, and a state too far from the anchor is simply projected afresh.
/// </para>
/// </summary>
public sealed class FactoredProjectionCache
{
    private readonly object _owner;
    private readonly ulong[] _referenceWords;

    internal FactoredProjectionCache(object owner, ulong[] referenceWords, object?[] projections)
    {
        _owner = owner;
        _referenceWords = referenceWords;
        Projections = projections;
    }

    /// <summary>One projection per atom shape; typed loosely so the generator's internals stay internal.</summary>
    internal object?[] Projections { get; }

    /// <summary>Guards against a second call borrowing the relations while the first still has them out.</summary>
    internal bool IsBorrowed { get; set; }

    internal void EnsureUsableBy(object owner)
    {
        if (!ReferenceEquals(_owner, owner))
            throw new InvalidOperationException("The projection cache belongs to a different applicable-action generator.");
        if (IsBorrowed)
            throw new InvalidOperationException(
                "The projection cache is already in use. Finish enumerating the previous result, or use a second cache.");
    }

    /// <summary>
    /// The atoms that differ between the reference state and this one, or false when there are so many that
    /// projecting afresh costs less than patching and unpatching.
    /// </summary>
    internal bool TryDescribeDelta(State state, List<int> added, List<int> removed, int limit)
    {
        added.Clear();
        removed.Clear();

        ReadOnlySpan<ulong> current = state.Bitboard;
        int words = Math.Max(current.Length, _referenceWords.Length);
        for (int word = 0; word < words; word++)
        {
            ulong now = word < current.Length ? current[word] : 0UL;
            ulong reference = word < _referenceWords.Length ? _referenceWords[word] : 0UL;
            if (now == reference) continue;

            Collect(now & ~reference, word, added);
            Collect(reference & ~now, word, removed);
            if (added.Count + removed.Count > limit) return false;
        }

        return true;
    }

    private static void Collect(ulong bits, int word, List<int> destination)
    {
        while (bits != 0)
        {
            destination.Add((word * 64) + System.Numerics.BitOperations.TrailingZeroCount(bits));
            bits &= bits - 1;
        }
    }
}
