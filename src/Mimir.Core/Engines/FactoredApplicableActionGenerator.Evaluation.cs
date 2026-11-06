using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public sealed partial class FactoredApplicableActionGenerator
{
    private static readonly List<Constant[]> NoTuples = new();

    /// <summary>
    /// Collects, for every atom, the ground tuples that satisfy it in this state, projected onto the schema
    /// parameters the atom mentions. Equality needs no extension because its truth is decided by reference.
    /// </summary>
    private List<Constant[]>?[] MaterializeTrueTuples(
        SchemaPlan plan,
        ExtendedState state,
        Dictionary<Predicate, List<Fact>> derivedFacts)
    {
        var tuples = new List<Constant[]>?[plan.Atoms.Length];
        for (int index = 0; index < plan.Atoms.Length; index++)
        {
            FactoredAtom atom = plan.Atoms[index];
            tuples[index] = atom.Kind switch
            {
                FactoredAtomKind.Equality => null,
                FactoredAtomKind.FunctionDefined => ProjectFunctionCalls(atom),
                FactoredAtomKind.Derived => ProjectFacts(
                    atom,
                    derivedFacts.GetValueOrDefault(atom.Predicate!) ?? (IReadOnlyList<Fact>)Array.Empty<Fact>(),
                    state: null),
                _ => ProjectFacts(atom, Problem.Context.GetFacts(atom.Predicate!), state),
            };
        }

        return tuples;
    }

    private static List<Constant[]> ProjectFacts(
        FactoredAtom atom,
        IReadOnlyList<Fact> facts,
        ExtendedState? state)
    {
        var tuples = new List<Constant[]>();
        var projected = new Constant[atom.Parameters.Length];
        foreach (Fact fact in facts)
        {
            if (state is not null && !state.IsTrue(fact)) continue;
            if (!atom.TryProject(fact.Arguments, projected)) continue;
            tuples.Add((Constant[])projected.Clone());
        }

        return tuples;
    }

    private List<Constant[]> ProjectFunctionCalls(FactoredAtom atom)
    {
        var tuples = new List<Constant[]>();
        var projected = new Constant[atom.Parameters.Length];
        foreach (GroundFunctionCall call in Problem.Context.GetDefinedFunctionCalls(atom.Function!))
        {
            if (!atom.TryProject(call.Arguments, projected)) continue;
            tuples.Add((Constant[])projected.Clone());
        }

        return tuples;
    }

    /// <summary>
    /// Shrinks the candidate sets until every atom with at most one undetermined parameter is enforced. Returns
    /// false when some parameter runs out of candidates or a fully determined atom is violated.
    /// </summary>
    private static bool Propagate(
        SchemaPlan plan,
        List<Constant[]>?[] tuples,
        List<Constant>[] candidates)
    {
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
        {
            if (candidates[parameter].Count == 0) return false;
        }

        bool changed = true;
        while (changed)
        {
            changed = false;
            for (int index = 0; index < plan.Atoms.Length; index++)
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

                if (undeterminedCount > 1) continue;
                if (undeterminedCount == 0)
                {
                    if (!DeterminedAtomHolds(atom, tuples[index], candidates)) return false;
                    continue;
                }

                int before = candidates[undetermined].Count;
                Restrict(atom, tuples[index], candidates, undetermined);
                if (candidates[undetermined].Count == 0) return false;
                if (candidates[undetermined].Count != before) changed = true;
            }
        }

        return true;
    }

    private static bool DeterminedAtomHolds(
        FactoredAtom atom,
        List<Constant[]>? tuples,
        List<Constant>[] candidates)
    {
        if (atom.Kind == FactoredAtomKind.Equality)
        {
            Constant left = ResolveArgument(atom, 0, candidates);
            Constant right = ResolveArgument(atom, 1, candidates);
            return ReferenceEquals(left, right) == atom.IsPositive;
        }

        var values = new Constant[atom.Parameters.Length];
        for (int column = 0; column < values.Length; column++)
            values[column] = candidates[atom.Parameters[column]][0];

        bool isTrue = false;
        foreach (Constant[] tuple in tuples ?? NoTuples)
        {
            if (!TupleEquals(tuple, values)) continue;
            isTrue = true;
            break;
        }

        return isTrue == atom.IsPositive;
    }

    private static void Restrict(
        FactoredAtom atom,
        List<Constant[]>? tuples,
        List<Constant>[] candidates,
        int parameter)
    {
        List<Constant> remaining = candidates[parameter];
        if (atom.Kind == FactoredAtomKind.Equality)
        {
            RestrictByEquality(atom, candidates, parameter, remaining);
            return;
        }

        int column = Array.IndexOf(atom.Parameters, parameter);
        var supported = new HashSet<Constant>(ReferenceEqualityComparer.Instance);
        foreach (Constant[] tuple in tuples ?? NoTuples)
        {
            bool matches = true;
            for (int other = 0; other < atom.Parameters.Length; other++)
            {
                if (other == column) continue;
                if (!ReferenceEquals(tuple[other], candidates[atom.Parameters[other]][0]))
                {
                    matches = false;
                    break;
                }
            }

            if (matches) supported.Add(tuple[column]);
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

    private static Constant ResolveArgument(FactoredAtom atom, int position, List<Constant>[] candidates)
    {
        int parameter = atom.ArgumentSlots[position];
        return parameter < 0 ? atom.ArgumentConstants[position]! : candidates[parameter][0];
    }

    private static bool TupleEquals(Constant[] left, Constant[] right)
    {
        for (int column = 0; column < left.Length; column++)
        {
            if (!ReferenceEquals(left[column], right[column])) return false;
        }

        return true;
    }
}
