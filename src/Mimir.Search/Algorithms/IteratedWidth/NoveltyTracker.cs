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
        if (!ReferenceEquals(state.Context, _context))
            throw new InvalidOperationException("State and novelty tracker belong to different problem contexts.");

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
