using System.Numerics;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Search.Algorithms.IteratedWidth;

internal sealed class NoveltyTracker
{
    private readonly int _k;
    private readonly InstanceContext _context;
    private readonly SparseNoveltyTable _table;

    internal NoveltyTracker(int k, InstanceContext context)
    {
        if (k is < 1 or > 3)
            throw new ArgumentOutOfRangeException(nameof(k), "Novelty width must be between 1 and 3.");

        ArgumentNullException.ThrowIfNull(context);

        _k = k;
        _context = context;
        _table = new SparseNoveltyTable(k);
    }

    // Returns true if state introduces at least one new j-tuple for any j ≤ k.
    // Always registers every new tuple found — never short-circuits.
    internal bool IsNovel(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        Validate(state);

        int[] ids = CollectSortedIndices(state);
        bool novel = false;

        if (_k >= 1)
            novel |= CheckAtoms(ids);

        if (_k >= 2)
            novel |= CheckPairs(ids);

        if (_k == 3)
            novel |= CheckTriples(ids);

        return novel;
    }

    /// <summary>
    /// The same verdict as <see cref="IsNovel(State)"/>, reached without re-reading the whole state. Every atom of
    /// a successor except the ones it adds was already true in the parent, and the parent was registered before it
    /// could be expanded, so only tuples containing an added atom can be new.
    /// </summary>
    internal bool IsNovel(State state, State parent)
    {
        ArgumentNullException.ThrowIfNull(state);
        ArgumentNullException.ThrowIfNull(parent);
        Validate(state);
        Validate(parent);

        int[] added = CollectAddedIndices(state, parent);
        if (added.Length == 0) return false;

        bool novel = false;
        if (_k >= 1)
            novel |= CheckAtoms(added);

        if (_k == 1) return novel;

        int[] ids = CollectSortedIndices(state);
        if (_k >= 2)
            novel |= CheckPairsContaining(added, ids);

        if (_k == 3)
            novel |= CheckTriplesContaining(added, ids);

        return novel;
    }

    /// <summary>Whether the atom has been true in any state seen so far. Does not register it.</summary>
    internal bool IsAtomSeen(FluentIndex atom) => _table.ContainsAtom(atom.Value);

    /// <summary>Whether the two atoms have been true together in any state seen so far. Does not register them.</summary>
    internal bool IsPairSeen(FluentIndex first, FluentIndex second)
        => _table.ContainsPair(first.Value, second.Value);

    private void Validate(State state)
    {
        if (!ReferenceEquals(state.Context, _context))
            throw new InvalidOperationException("State and novelty tracker belong to different problem contexts.");
    }

    private bool CheckPairsContaining(int[] added, int[] ids)
    {
        bool novel = false;
        foreach (int left in added)
        {
            foreach (int right in ids)
            {
                if (left == right) continue;
                if (_table.RegisterPair(Math.Min(left, right), Math.Max(left, right)))
                    novel = true;
            }
        }

        return novel;
    }

    private bool CheckTriplesContaining(int[] added, int[] ids)
    {
        bool novel = false;
        Span<int> triple = stackalloc int[3];
        foreach (int first in added)
        {
            for (int i = 0; i < ids.Length - 1; i++)
            {
                if (ids[i] == first) continue;
                for (int j = i + 1; j < ids.Length; j++)
                {
                    if (ids[j] == first) continue;
                    triple[0] = first;
                    triple[1] = ids[i];
                    triple[2] = ids[j];
                    triple.Sort();
                    if (_table.RegisterTriple(triple[0], triple[1], triple[2]))
                        novel = true;
                }
            }
        }

        return novel;
    }

    // The bits set in the successor but not in the parent, ascending. Reading the bitboards rather than the
    // action's add list keeps this exact when an effect both adds and deletes an atom.
    private static int[] CollectAddedIndices(State state, State parent)
    {
        ReadOnlySpan<ulong> stateWords = state.Bitboard;
        ReadOnlySpan<ulong> parentWords = parent.Bitboard;
        var added = new List<int>();
        for (int word = 0; word < stateWords.Length; word++)
        {
            ulong bits = stateWords[word];
            if (word < parentWords.Length) bits &= ~parentWords[word];
            while (bits != 0)
            {
                added.Add((word * 64) + BitOperations.TrailingZeroCount(bits));
                bits &= bits - 1;
            }
        }

        return added.ToArray();
    }

    private bool CheckAtoms(int[] ids)
    {
        bool novel = false;
        foreach (int id in ids)
        {
            if (_table.RegisterAtom(id))
                novel = true;
        }
        return novel;
    }

    private bool CheckPairs(int[] ids)
    {
        bool novel = false;
        for (int i = 0; i < ids.Length - 1; i++)
        for (int j = i + 1; j < ids.Length; j++)
        {
            if (_table.RegisterPair(ids[i], ids[j]))
                novel = true;
        }
        return novel;
    }

    private bool CheckTriples(int[] ids)
    {
        bool novel = false;
        for (int i = 0; i < ids.Length - 2; i++)
        for (int j = i + 1; j < ids.Length - 1; j++)
        for (int l = j + 1; l < ids.Length; l++)
        {
            if (_table.RegisterTriple(ids[i], ids[j], ids[l]))
                novel = true;
        }
        return novel;
    }

    private static int[] CollectSortedIndices(State state)
    {
        var ids = new List<int>();
        foreach (Fact<Fluent> fact in state.GetTrueFacts())
            ids.Add(fact.LocalIndex);

        ids.Sort();
        return ids.ToArray();
    }

    private sealed class SparseNoveltyTable
    {
        private readonly HashSet<int> _seenAtoms = new();
        private readonly HashSet<long>? _seenPairs;
        private readonly HashSet<(int, int, int)>? _seenTriples;

        public SparseNoveltyTable(int k)
        {
            _seenPairs = k >= 2 ? new HashSet<long>() : null;
            _seenTriples = k == 3 ? new HashSet<(int, int, int)>() : null;
        }

        public bool ContainsAtom(int atom) => _seenAtoms.Contains(atom);

        public bool ContainsPair(int first, int second)
        {
            if (first == second) return true;
            (int low, int high) = first < second ? (first, second) : (second, first);
            return _seenPairs!.Contains((long)low << 32 | (uint)high);
        }

        public bool RegisterAtom(int atom)
        {
            ValidateFactId(atom);
            return _seenAtoms.Add(atom);
        }

        public bool RegisterPair(int first, int second)
        {
            ValidateFactId(first);
            ValidateFactId(second);
            if (first >= second)
                throw new InvalidOperationException("Novelty pairs must contain sorted, distinct fact indices.");

            long key = (long)first << 32 | (uint)second;
            return _seenPairs!.Add(key);
        }

        public bool RegisterTriple(int first, int second, int third)
        {
            ValidateFactId(first);
            ValidateFactId(second);
            ValidateFactId(third);
            if (first >= second || second >= third)
                throw new InvalidOperationException("Novelty triples must contain sorted, distinct fact indices.");

            return _seenTriples!.Add((first, second, third));
        }

        private static void ValidateFactId(int factId)
        {
            if (factId < 0)
                throw new InvalidOperationException("A state fact has an invalid negative local index.");
        }
    }
}
