using System.Numerics;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Search.Algorithms.IteratedWidth;

internal sealed class NoveltyTracker
{
    private readonly int _k;
    private readonly InstanceContext _context;
    private readonly SparseNoveltyTable _table;
    private int[] _uncoveredBuffer = new int[64];

    /// <summary>Prototype switch, so the cover fast path can be measured against plain triple enumeration.</summary>
    /// <summary>Off only so that tests and measurements can compare the shortcut against plain enumeration.</summary>
    internal bool UseCoverFastPath { get; set; } = true;

    internal long CoverHits;
    internal long CoverMisses;

    // Reused across successors: a search generates millions of them and each would otherwise allocate.
    private int[] _addedBuffer = new int[16];

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

        int addedCount = CollectAddedIndices(state, parent);
        if (addedCount == 0) return false;

        ReadOnlySpan<int> added = _addedBuffer.AsSpan(0, addedCount);
        bool novel = false;
        if (_k >= 1)
            novel |= CheckAtoms(added);

        if (_k == 1) return novel;
        if (_k == 2) return CheckPairsContaining(added, state) || novel;

        int[] ids = CollectSortedIndices(state);
        novel |= CheckPairsContaining(added, ids);

        if (!UseCoverFastPath)
        {
            novel |= CheckTriplesContaining(added, ids);
            return novel;
        }

        // Only atoms whose cover does not already contain this state can contribute a new triple.
        if (_uncoveredBuffer.Length < added.Length) _uncoveredBuffer = new int[added.Length];
        int uncovered = 0;
        foreach (int atom in added)
            if (!_table.CoversAll(atom, ids)) _uncoveredBuffer[uncovered++] = atom;

        if (uncovered == 0)
        {
            CoverHits++;
            return novel;
        }

        CoverMisses++;

        // A cover that does not contain the state still covers every pair drawn from the part that overlaps it,
        // so only triples touching an atom outside the cover have to be registered.
        ReadOnlySpan<int> fresh = _uncoveredBuffer.AsSpan(0, uncovered);
        foreach (int atom in fresh)
        {
            novel |= CheckTriplesOutsideCover(atom, ids);
            _table.SetCover(atom, ids);
        }

        return novel;
    }

    /// <summary>Whether the atom has been true in any state seen so far. Does not register it.</summary>
    internal bool IsAtomSeen(FluentIndex atom) => _table.ContainsAtom(atom.Value);

    /// <summary>Whether the two atoms have been true together in any state seen so far. Does not register them.</summary>
    internal bool IsPairSeen(FluentIndex first, FluentIndex second)
        => _table.ContainsPair(first.Value, second.Value);

    /// <summary>Whether pairing the atom with the state would introduce anything new. Does not register them.</summary>
    internal bool HasUnseenPairWith(FluentIndex atom, State state)
        => _table.HasUnseenPairWith(atom.Value, state.Bitboard);

    private void Validate(State state)
    {
        if (!ReferenceEquals(state.Context, _context))
            throw new InvalidOperationException("State and novelty tracker belong to different problem contexts.");
    }

    private bool CheckPairsContaining(ReadOnlySpan<int> added, int[] ids)
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

    /// <summary>
    /// Pairs each added atom with the state's own, read straight off the bitboard. Materializing the state's
    /// atoms first costs an allocation and a sort per successor, and the pair key is normalized anyway.
    /// </summary>
    private bool CheckPairsContaining(ReadOnlySpan<int> added, State state)
    {
        bool novel = false;
        ReadOnlySpan<ulong> words = state.Bitboard;
        foreach (int left in added)
        {
            if (_table.RegisterPairsWith(left, words)) novel = true;
        }

        return novel;
    }

    private bool CheckTriplesOutsideCover(int atom, int[] ids)
    {
        bool novel = false;
        Span<int> triple = stackalloc int[3];
        for (int i = 0; i < ids.Length; i++)
        {
            if (ids[i] == atom) continue;
            bool leftInside = _table.CoverContains(atom, ids[i]);
            for (int j = i + 1; j < ids.Length; j++)
            {
                if (ids[j] == atom) continue;
                // Both endpoints inside the cover means this triple was registered when the cover was set.
                if (leftInside && _table.CoverContains(atom, ids[j])) continue;

                triple[0] = atom;
                triple[1] = ids[i];
                triple[2] = ids[j];
                triple.Sort();
                if (_table.RegisterTriple(triple[0], triple[1], triple[2]))
                    novel = true;
            }
        }

        return novel;
    }

    private bool CheckTriplesContaining(ReadOnlySpan<int> added, int[] ids)
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
    private int CollectAddedIndices(State state, State parent)
    {
        ReadOnlySpan<ulong> stateWords = state.Bitboard;
        ReadOnlySpan<ulong> parentWords = parent.Bitboard;
        int count = 0;
        for (int word = 0; word < stateWords.Length; word++)
        {
            ulong bits = stateWords[word];
            if (word < parentWords.Length) bits &= ~parentWords[word];
            while (bits != 0)
            {
                if (count == _addedBuffer.Length) Array.Resize(ref _addedBuffer, count * 2);
                _addedBuffer[count++] = (word * 64) + BitOperations.TrailingZeroCount(bits);
                bits &= bits - 1;
            }
        }

        return count;
    }

    private bool CheckAtoms(ReadOnlySpan<int> ids)
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
        private readonly HashSet<(int, int, int)>? _seenTriples;

        // Seen pairs as one bitset per atom, stored both ways round. Costs more memory than a hash set of keys,
        // but a state's atoms arrive as a bitboard, so a whole word of partners is tested with one AND and a
        // state that introduces no pair is dismissed in a handful of them instead of one probe per atom.
        private ulong[]?[] _pairRows;
        // For each atom, the atom set it was last fully triple-registered against. Every pair drawn from that
        // set has been seen with the atom, so a later state inside it can contribute no triple containing it.
        private ulong[]?[] _coverRows = Array.Empty<ulong[]?>();
        private int _pairRowWords;

        public SparseNoveltyTable(int k)
        {
            _pairRows = k >= 2 ? new ulong[]?[64] : Array.Empty<ulong[]?>();
            _pairRowWords = 1;
            _seenTriples = k == 3 ? new HashSet<(int, int, int)>() : null;
        }

        public bool ContainsAtom(int atom) => _seenAtoms.Contains(atom);

        public bool CoversAll(int atom, int[] ids)
        {
            if (atom >= _coverRows.Length || _coverRows[atom] is not { } row) return false;
            foreach (int id in ids)
            {
                int word = id >> 6;
                if (word >= row.Length || (row[word] & (1UL << (id & 63))) == 0) return false;
            }

            return true;
        }

        public bool CoverContains(int atom, int id)
        {
            if (atom >= _coverRows.Length || _coverRows[atom] is not { } row) return false;
            int word = id >> 6;
            return word < row.Length && (row[word] & (1UL << (id & 63))) != 0;
        }

        public void SetCover(int atom, int[] ids)
        {
            if (atom >= _coverRows.Length)
            {
                int length = _coverRows.Length == 0 ? 64 : _coverRows.Length;
                while (atom >= length) length *= 2;
                Array.Resize(ref _coverRows, length);
            }

            int needed = ids.Length == 0 ? 1 : (ids[^1] >> 6) + 1;
            ulong[]? row = _coverRows[atom];
            if (row is null || row.Length < needed) row = new ulong[Math.Max(needed, 4)];
            else Array.Clear(row);

            foreach (int id in ids) row[id >> 6] |= 1UL << (id & 63);
            _coverRows[atom] = row;
        }

        public bool ContainsPair(int first, int second)
        {
            if (first == second) return true;
            if (first >= _pairRows.Length) return false;
            ulong[]? row = _pairRows[first];
            int word = second >> 6;
            return row is not null && word < row.Length && (row[word] & (1UL << (second & 63))) != 0;
        }

        /// <summary>Whether any atom of the state has not yet been paired with <paramref name="atom"/>.</summary>
        public bool HasUnseenPairWith(int atom, ReadOnlySpan<ulong> stateWords)
        {
            if (atom >= _pairRows.Length || _pairRows[atom] is not { } row) return true;

            for (int word = 0; word < stateWords.Length; word++)
            {
                ulong bits = word < row.Length ? stateWords[word] & ~row[word] : stateWords[word];
                if (word == atom >> 6) bits &= ~(1UL << (atom & 63));
                if (bits != 0) return true;
            }

            return false;
        }

        /// <summary>
        /// Registers every pair joining <paramref name="atom"/> to an atom of <paramref name="stateWords"/>, and
        /// reports whether any of them is new.
        /// </summary>
        public bool RegisterPairsWith(int atom, ReadOnlySpan<ulong> stateWords)
        {
            ulong[] row = GetOrCreateRow(atom, stateWords.Length);
            bool novel = false;
            int limit = Math.Min(stateWords.Length, row.Length);
            for (int word = 0; word < limit; word++)
            {
                ulong bits = stateWords[word] & ~row[word];
                if (word == atom >> 6) bits &= ~(1UL << (atom & 63));
                if (bits == 0) continue;

                row[word] |= bits;
                novel = true;
                ulong pending = bits;
                while (pending != 0)
                {
                    int other = (word << 6) + BitOperations.TrailingZeroCount(pending);
                    pending &= pending - 1;
                    GetOrCreateRow(other, (atom >> 6) + 1)[atom >> 6] |= 1UL << (atom & 63);
                }
            }

            return novel;
        }

        private ulong[] GetOrCreateRow(int atom, int words)
        {
            if (atom >= _pairRows.Length)
            {
                int length = _pairRows.Length == 0 ? 64 : _pairRows.Length;
                while (atom >= length) length *= 2;
                Array.Resize(ref _pairRows, length);
            }

            if (words > _pairRowWords) _pairRowWords = words;
            ulong[]? row = _pairRows[atom];
            if (row is null)
            {
                row = new ulong[_pairRowWords];
                _pairRows[atom] = row;
                return row;
            }

            if (row.Length >= words) return row;
            Array.Resize(ref row, Math.Max(words, _pairRowWords));
            _pairRows[atom] = row;
            return row;
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

            return RegisterPairTrusted(first, second);
        }

        // Called once per atom of every successor, so the caller guarantees sorted, distinct, non-negative ids.
        public bool RegisterPairTrusted(int first, int second)
        {
            ulong[] row = GetOrCreateRow(first, (second >> 6) + 1);
            ulong mask = 1UL << (second & 63);
            if ((row[second >> 6] & mask) != 0) return false;

            row[second >> 6] |= mask;
            GetOrCreateRow(second, (first >> 6) + 1)[first >> 6] |= 1UL << (first & 63);
            return true;
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
