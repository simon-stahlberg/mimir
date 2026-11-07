namespace Mimir.Core.Grounding;

/// <summary>
/// Carries a factored generator's projected relations from one state to the next.
/// <para>
/// Answering a state means knowing which ground atoms satisfy each precondition, and rebuilding that for every
/// state costs the size of each predicate's extension. Successive states a search visits differ by a handful of
/// atoms, so a cache handed back to the generator lets it patch what it already has instead.
/// </para>
/// <para>
/// A cache belongs to whoever created it, is updated in place by the call it is passed to, and may only be used
/// with the generator that created it. Two searches need two caches. Passing none is always correct and simply
/// projects afresh.
/// </para>
/// </summary>
public sealed class FactoredProjectionCache
{
    private readonly object _owner;
    private ulong[] _stateWords = Array.Empty<ulong>();

    internal FactoredProjectionCache(object owner, int projectionCount)
    {
        _owner = owner;
        Projections = new object?[projectionCount];
    }

    /// <summary>Holds one projection per shape; typed loosely so the generator's internals stay internal.</summary>
    internal object?[] Projections { get; }

    /// <summary>False until the first state has been projected, after which patches apply.</summary>
    internal bool IsPrimed { get; private set; }

    internal void EnsureOwnedBy(object owner)
    {
        if (!ReferenceEquals(_owner, owner))
            throw new InvalidOperationException("The projection cache belongs to a different applicable-action generator.");
    }

    /// <summary>The atoms that differ between the cached state and this one, or null when it is worth starting over.</summary>
    internal bool TryDescribeDelta(State state, List<int> added, List<int> removed, int rebuildThreshold)
    {
        added.Clear();
        removed.Clear();
        if (!IsPrimed) return false;

        ReadOnlySpan<ulong> current = state.Bitboard;
        int words = Math.Max(current.Length, _stateWords.Length);
        for (int word = 0; word < words; word++)
        {
            ulong now = word < current.Length ? current[word] : 0UL;
            ulong before = word < _stateWords.Length ? _stateWords[word] : 0UL;
            if (now == before) continue;

            Collect(now & ~before, word, added);
            Collect(before & ~now, word, removed);
            if (added.Count + removed.Count > rebuildThreshold) return false;
        }

        return true;
    }

    internal void Remember(State state)
    {
        ReadOnlySpan<ulong> current = state.Bitboard;
        if (_stateWords.Length < current.Length) _stateWords = new ulong[current.Length];
        else Array.Clear(_stateWords);
        current.CopyTo(_stateWords);
        IsPrimed = true;
    }

    /// <summary>Drops everything, so the next call projects from scratch.</summary>
    public void Clear()
    {
        Array.Clear(Projections);
        Array.Clear(_stateWords);
        IsPrimed = false;
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
