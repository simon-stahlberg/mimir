using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public sealed partial class FactoredApplicableActionGenerator
{
    /// <summary>
    /// The ground tuples satisfying one atom, packed row-major into a single array. A tuple per row would cost
    /// an allocation per true fact per schema per state, which dominated both CPU and GC time.
    /// </summary>
    internal sealed class TupleSet
    {
        public static readonly TupleSet Empty = new(Array.Empty<Constant>(), arity: 0, count: 0);

        private Constant[] _values;
        private int[]? _factByRow;
        private Dictionary<int, int>? _rowByFact;

        public int Arity { get; }
        public int Count { get; private set; }

        public TupleSet(Constant[] values, int arity, int count)
        {
            _values = values;
            Arity = arity;
            Count = count;
        }

        public Constant At(int row, int column) => _values[(row * Arity) + column];

        /// <summary>The rows agreeing with a fixed value, as a plain set; the result is never tracked.</summary>
        public TupleSet Where(int column, Constant value)
        {
            int matches = 0;
            for (int row = 0; row < Count; row++)
            {
                if (ReferenceEquals(At(row, column), value)) matches++;
            }

            if (matches == Count && _rowByFact is null) return this;

            var values = new Constant[matches * Arity];
            int next = 0;
            for (int row = 0; row < Count; row++)
            {
                if (!ReferenceEquals(At(row, column), value)) continue;
                Array.Copy(_values, row * Arity, values, next * Arity, Arity);
                next++;
            }

            return new TupleSet(values, Arity, matches);
        }

        /// <summary>
        /// Lets rows be added and removed as the state changes, at the cost of an index from fact to row. Only a
        /// projection kept across states needs this; one built for a single state is read and dropped.
        /// </summary>
        public void StartTracking(IReadOnlyList<int> facts)
        {
            _factByRow = facts.ToArray();
            _rowByFact = new Dictionary<int, int>(facts.Count);
            for (int row = 0; row < facts.Count; row++) _rowByFact[facts[row]] = row;
        }

        public void Add(int fact, ReadOnlySpan<Constant> row)
        {
            if (_rowByFact!.ContainsKey(fact)) return;

            int required = (Count + 1) * Arity;
            if (required > _values.Length)
                Array.Resize(ref _values, Math.Max(required, Math.Max(_values.Length * 2, 8)));
            if (Count == _factByRow!.Length)
                Array.Resize(ref _factByRow, Math.Max(Count * 2, 8));

            row.CopyTo(_values.AsSpan(Count * Arity, Arity));
            _factByRow[Count] = fact;
            _rowByFact[fact] = Count;
            Count++;
        }

        /// <summary>Swaps the last row into the vacated slot, so removal costs the arity rather than the count.</summary>
        public void Remove(int fact)
        {
            if (!_rowByFact!.Remove(fact, out int row)) return;

            Count--;
            if (row != Count)
            {
                Array.Copy(_values, Count * Arity, _values, row * Arity, Arity);
                int moved = _factByRow![Count];
                _factByRow[row] = moved;
                _rowByFact[moved] = row;
            }
        }
    }

    /// <summary>
    /// The true facts of each predicate the schemas mention, shared by every schema expanded from one state.
    /// Static extensions never change, so they are filtered once per generator; fluent ones are filtered on first
    /// use, because a state usually leaves most schemas inapplicable before their predicates are ever read.
    /// </summary>
    internal sealed class StateFacts
    {
        private readonly Dictionary<Predicate, List<Fact>> _statics;
        private readonly Dictionary<Predicate, List<Fact>> _fluents = new(ReferenceEqualityComparer.Instance);
        private readonly TupleSet?[] _projections;
        private readonly object?[]? _cached;

        public ExtendedState State { get; }

        public StateFacts(
            Dictionary<Predicate, List<Fact>> statics,
            ExtendedState state,
            int projectionCount,
            FactoredProjectionCache? cache)
        {
            _statics = statics;
            State = state;
            _cached = cache?.Projections;
            _projections = projectionCount == 0 ? Array.Empty<TupleSet?>() : new TupleSet?[projectionCount];
        }

        /// <summary>
        /// The projection for an atom's shape in this state. A carried cache supplies it already patched; without
        /// one it is built on first use and shared by the atoms of this state only.
        /// </summary>
        public TupleSet GetProjection(FactoredAtom atom)
        {
            if (_cached is null)
                return _projections[atom.ProjectionId] ??= ProjectFacts(atom, GetTrueFacts(atom.Predicate!));

            if (_cached[atom.ProjectionId] is TupleSet cached) return cached;

            TupleSet built = ProjectTracked(atom, GetTrueFacts(atom.Predicate!));
            _cached[atom.ProjectionId] = built;
            return built;
        }

        public List<Fact> GetTrueFacts(Predicate predicate)
        {
            if (_statics.TryGetValue(predicate, out List<Fact>? statics)) return statics;
            if (_fluents.TryGetValue(predicate, out List<Fact>? fluents)) return fluents;

            fluents = State.State.Context.GetFacts(predicate).Where(State.IsTrue).ToList();
            _fluents.Add(predicate, fluents);
            return fluents;
        }
    }

    /// <summary>
    /// Collects, for every atom, the ground tuples that satisfy it in this state, projected onto the schema
    /// parameters the atom mentions. Equality needs no extension because its truth is decided by reference.
    /// </summary>
    private static TupleSet?[] MaterializeTrueTuples(SchemaPlan plan, StateFacts facts)
    {
        var tuples = (TupleSet?[])plan.UnchangingTuples.Clone();
        for (int index = 0; index < plan.Atoms.Length; index++)
        {
            FactoredAtom atom = plan.Atoms[index];
            if (atom.Kind != FactoredAtomKind.Fluent) continue;
            tuples[index] = facts.GetProjection(atom);
        }

        return tuples;
    }

    /// <summary>Projects the atoms whose extension is fixed for the whole problem, once per generator.</summary>
    private TupleSet?[] ProjectUnchangingTuples(SchemaPlan plan)
    {
        var tuples = new TupleSet?[plan.Atoms.Length];
        for (int index = 0; index < plan.Atoms.Length; index++)
        {
            FactoredAtom atom = plan.Atoms[index];
            tuples[index] = atom.Kind switch
            {
                FactoredAtomKind.Static => ProjectFacts(atom, StaticTrueFacts(atom.Predicate!)),
                FactoredAtomKind.FunctionDefined => ProjectFunctionCalls(atom),
                _ => null,
            };
        }

        return tuples;
    }

    /// <summary>As <see cref="ProjectFacts"/>, but able to take rows in and out as the state changes.</summary>
    private static TupleSet ProjectTracked(FactoredAtom atom, List<Fact> trueFacts)
    {
        int arity = atom.Parameters.Length;
        var values = new Constant[trueFacts.Count * arity];
        var projected = new Constant[arity];
        var facts = new List<int>(trueFacts.Count);
        int count = 0;
        foreach (Fact fact in trueFacts)
        {
            if (!atom.TryProject(fact.Arguments, projected)) continue;
            Array.Copy(projected, 0, values, count * arity, arity);
            facts.Add(((Fact<Fluent>)fact).LocalIndex);
            count++;
        }

        var rows = new TupleSet(values, arity, count);
        rows.StartTracking(facts);
        return rows;
    }

    private static TupleSet ProjectFacts(FactoredAtom atom, List<Fact> trueFacts)
    {
        int arity = atom.Parameters.Length;
        var values = new Constant[trueFacts.Count * arity];
        var projected = new Constant[arity];
        int count = 0;
        foreach (Fact fact in trueFacts)
        {
            if (!atom.TryProject(fact.Arguments, projected)) continue;
            Array.Copy(projected, 0, values, count * arity, arity);
            count++;
        }

        return new TupleSet(values, arity, count);
    }

    private TupleSet ProjectFunctionCalls(FactoredAtom atom)
    {
        int arity = atom.Parameters.Length;
        var rows = new List<Constant>();
        var projected = new Constant[arity];
        int count = 0;
        foreach (GroundFunctionCall call in Problem.Context.GetDefinedFunctionCalls(atom.Function!))
        {
            if (!atom.TryProject(call.Arguments, projected)) continue;
            rows.AddRange(projected);
            count++;
        }

        return new TupleSet(rows.ToArray(), arity, count);
    }

    /// <summary>
    /// Shrinks the candidate sets until every atom with at most one undetermined parameter is enforced. Only the
    /// atoms reachable from a changed parameter are revisited. Returns false when some parameter runs out of
    /// candidates or a fully determined atom is violated.
    /// </summary>
    private static bool Propagate(
        SchemaPlan plan,
        TupleSet?[] tuples,
        List<Constant>[] candidates,
        StateFacts facts,
        Stack<int> worklist)
    {
        while (worklist.Count > 0)
        {
            int changedParameter = worklist.Pop();
            foreach (int index in plan.AtomsByParameter[changedParameter])
            {
                if (!EnforceAtom(plan, tuples, candidates, facts, index, worklist)) return false;
            }
        }

        return true;
    }

    private static bool EnforceAtom(
        SchemaPlan plan,
        TupleSet?[] tuples,
        List<Constant>[] candidates,
        StateFacts facts,
        int index,
        Stack<int> worklist)
    {
        FactoredAtom atom = plan.Atoms[index];
        int undeterminedCount = 0;
        int undetermined = -1;
        foreach (int parameter in atom.Parameters)
        {
            if (candidates[parameter].Count <= 1) continue;
            undeterminedCount++;
            undetermined = parameter;
        }

        if (undeterminedCount > 1) return true;
        if (undeterminedCount == 0)
            return DeterminedAtomHolds(atom, tuples[index], candidates, facts);

        int before = candidates[undetermined].Count;
        Restrict(atom, tuples[index], candidates, facts, undetermined);
        if (candidates[undetermined].Count == 0) return false;
        if (candidates[undetermined].Count != before) worklist.Push(undetermined);
        return true;
    }

    /// <summary>
    /// Restricts every atom that mentions the fixed parameter to the tuples agreeing with it. Atoms that do not
    /// mention it keep their existing list, so a branch copies only the relations it actually narrows.
    /// </summary>
    private static TupleSet?[] SemiJoin(
        SchemaPlan plan,
        TupleSet?[] tuples,
        int parameter,
        Constant value)
    {
        var reduced = (TupleSet?[])tuples.Clone();
        foreach (int index in plan.AtomsByParameter[parameter])
        {
            if (tuples[index] is not { } atomTuples) continue;
            int column = Array.IndexOf(plan.Atoms[index].Parameters, parameter);
            reduced[index] = atomTuples.Where(column, value);
        }

        return reduced;
    }

    /// <summary>Enforces every atom once, which seeds the incremental propagation above.</summary>
    private static bool PropagateAll(
        SchemaPlan plan,
        TupleSet?[] tuples,
        List<Constant>[] candidates,
        StateFacts facts)
    {
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
        {
            if (candidates[parameter].Count == 0) return false;
        }

        foreach (int index in plan.ParameterlessAtoms)
        {
            if (!DeterminedAtomHolds(plan.Atoms[index], tuples[index], candidates, facts)) return false;
        }

        var worklist = new Stack<int>();
        for (int index = 0; index < plan.Atoms.Length; index++)
        {
            if (!EnforceAtom(plan, tuples, candidates, facts, index, worklist)) return false;
        }

        return Propagate(plan, tuples, candidates, facts, worklist);
    }

    private static bool DeterminedAtomHolds(
        FactoredAtom atom,
        TupleSet? tuples,
        List<Constant>[] candidates,
        StateFacts facts)
    {
        if (atom.IsProbed)
            return ProbeDerived(atom, candidates, facts) == atom.IsPositive;

        if (atom.Kind == FactoredAtomKind.Equality)
        {
            Constant left = ResolveArgument(atom, 0, candidates);
            Constant right = ResolveArgument(atom, 1, candidates);
            return ReferenceEquals(left, right) == atom.IsPositive;
        }

        TupleSet rows = tuples ?? TupleSet.Empty;
        bool isTrue = false;
        for (int row = 0; row < rows.Count && !isTrue; row++)
        {
            isTrue = true;
            for (int column = 0; column < rows.Arity; column++)
            {
                if (ReferenceEquals(rows.At(row, column), candidates[atom.Parameters[column]][0])) continue;
                isTrue = false;
                break;
            }
        }

        return isTrue == atom.IsPositive;
    }

    private static void Restrict(
        FactoredAtom atom,
        TupleSet? tuples,
        List<Constant>[] candidates,
        StateFacts facts,
        int parameter)
    {
        List<Constant> remaining = candidates[parameter];
        if (atom.IsProbed)
        {
            RestrictByProbe(atom, candidates, facts, parameter, remaining);
            return;
        }

        if (atom.Kind == FactoredAtomKind.Equality)
        {
            RestrictByEquality(atom, candidates, parameter, remaining);
            return;
        }

        int column = Array.IndexOf(atom.Parameters, parameter);
        TupleSet rows = tuples ?? TupleSet.Empty;
        var supported = new HashSet<Constant>(ReferenceEqualityComparer.Instance);
        for (int row = 0; row < rows.Count; row++)
        {
            bool matches = true;
            for (int other = 0; other < rows.Arity; other++)
            {
                if (other == column) continue;
                if (!ReferenceEquals(rows.At(row, other), candidates[atom.Parameters[other]][0]))
                {
                    matches = false;
                    break;
                }
            }

            if (matches) supported.Add(rows.At(row, column));
        }

        remaining.RemoveAll(candidate => supported.Contains(candidate) != atom.IsPositive);
    }

    private static void RestrictByEquality(
        FactoredAtom atom,
        List<Constant>[] candidates,
        int parameter,
        List<Constant> remaining)
    {
        // Both sides are the same parameter, so the literal is a tautology or a contradiction.
        if (atom.ArgumentSlots[0] == atom.ArgumentSlots[1])
        {
            if (!atom.IsPositive) remaining.Clear();
            return;
        }

        int knownPosition = atom.ArgumentSlots[0] == parameter ? 1 : 0;
        Constant known = ResolveArgument(atom, knownPosition, candidates);
        if (atom.IsPositive)
        {
            remaining.RemoveAll(candidate => !ReferenceEquals(candidate, known));
            return;
        }

        remaining.RemoveAll(candidate => ReferenceEquals(candidate, known));
    }

    /// <summary>Probes one derived atom per candidate, with every other parameter already determined.</summary>
    private static void RestrictByProbe(
        FactoredAtom atom,
        List<Constant>[] candidates,
        StateFacts facts,
        int parameter,
        List<Constant> remaining)
    {
        var original = new List<Constant>(remaining);
        var probe = new List<Constant> { original[0] };
        candidates[parameter] = probe;
        remaining.Clear();
        foreach (Constant candidate in original)
        {
            probe[0] = candidate;
            if (ProbeDerived(atom, candidates, facts) == atom.IsPositive) remaining.Add(candidate);
        }

        candidates[parameter] = remaining;
    }

    private static bool ProbeDerived(FactoredAtom atom, List<Constant>[] candidates, StateFacts facts)
    {
        var arguments = new Constant[atom.ArgumentSlots.Length];
        for (int position = 0; position < arguments.Length; position++)
            arguments[position] = ResolveArgument(atom, position, candidates);

        return facts.State.IsDerivedTruePrevalidated(
            (Mimir.Core.Schemas.Predicate<Derived>)atom.Predicate!,
            arguments);
    }

    private static Constant ResolveArgument(FactoredAtom atom, int position, List<Constant>[] candidates)
    {
        int parameter = atom.ArgumentSlots[position];
        return parameter < 0 ? atom.ArgumentConstants[position]! : candidates[parameter][0];
    }

}
