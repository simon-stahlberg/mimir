using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Engines;

public interface IFactoredApplicableActionGenerator : IApplicableActionGenerator
{
    /// <summary>
    /// The applicable actions grouped into Cartesian products over the schema parameters. The groups are pairwise
    /// disjoint, so expanding all of them yields every applicable action exactly once.
    /// <para>
    /// The sequence is produced lazily and is not a snapshot: a consumer that stops early does not pay for the
    /// groups it never reads, and enumerating the result twice solves the state twice. Materialize it if you need
    /// a stable list. Unlike <see cref="IApplicableActionGenerator.GetApplicableActions(ExtendedState)"/>, this
    /// stays bounded on states whose successors number in the millions.
    /// </para>
    /// </summary>
    IEnumerable<FactoredAction> GetFactoredApplicableActions(ExtendedState state);

    /// <summary>
    /// The same groups, reusing and updating <paramref name="cache"/> instead of projecting every precondition's
    /// extension again. The cache is written to, which is why it is a parameter rather than something the
    /// generator keeps: its lifetime and its sharing are the caller's to decide.
    /// </summary>
    IEnumerable<FactoredAction> GetFactoredApplicableActions(ExtendedState state, FactoredProjectionCache cache);

    /// <summary>
    /// A cache anchored on <paramref name="referenceState"/>, which the overload above adapts to nearby states.
    /// The anchor is fixed here and never moves, so pick a state the search stays close to. It may not be passed
    /// to any other generator.
    /// </summary>
    FactoredProjectionCache CreateProjectionCache(ExtendedState referenceState);
}

/// <summary>
/// A lifted generator that answers each schema's precondition as a conjunctive query and reports the answers as
/// Cartesian products instead of individual bindings. Parameters that the precondition leaves independent stay
/// set-valued, which keeps states with an enormous branching factor representable.
/// <para>
/// It supports a fragment of PDDL: numeric conditions and effects, and cost expressions whose definedness cannot
/// be established ahead of time, throw <see cref="NotSupportedException"/> while the problem is loaded. The
/// <see cref="IApplicableActionGenerator"/> members expand every product, which is correct but unbounded; prefer
/// <see cref="GetFactoredApplicableActions"/> on the instances this generator exists for.
/// </para>
/// </summary>
public sealed partial class FactoredApplicableActionGenerator : IFactoredApplicableActionGenerator
{
    public Problem Problem { get; }
    private readonly SchemaPlan[] _schemaPlans;
    private readonly Dictionary<Predicate, List<Fact>> _staticTrueFacts;
    private int _observedGen0Collections;
    private readonly int _projectionCount;
    private readonly Dictionary<Predicate, List<FactoredAtom>> _atomsByPredicate;

    internal FactoredApplicableActionGenerator(Problem problem)
    {
        Problem = problem ?? throw new ArgumentNullException(nameof(problem));
        _schemaPlans = problem.Domain.Actions
            .Select(schema => CompileSchema(problem, schema))
            .ToArray();

        _staticTrueFacts = new Dictionary<Predicate, List<Fact>>(ReferenceEqualityComparer.Instance);
        _ = problem.InitialState;
        foreach (FactoredAtom atom in _schemaPlans.SelectMany(plan => plan.Atoms))
        {
            if (atom.Kind != FactoredAtomKind.Static || _staticTrueFacts.ContainsKey(atom.Predicate!)) continue;
            _staticTrueFacts[atom.Predicate!] = problem.Context
                .GetFacts(atom.Predicate!)
                .Where(fact => problem.Context.IsTrue(new StaticIndex(((Fact<Static>)fact).LocalIndex)))
                .ToList();
        }

        _projectionCount = AssignProjectionIds(_schemaPlans);
        _atomsByPredicate = new Dictionary<Predicate, List<FactoredAtom>>(ReferenceEqualityComparer.Instance);
        foreach (FactoredAtom atom in _schemaPlans.SelectMany(plan => plan.Atoms))
        {
            if (atom.Kind != FactoredAtomKind.Fluent) continue;
            if (!_atomsByPredicate.TryGetValue(atom.Predicate!, out List<FactoredAtom>? atoms))
            {
                atoms = new List<FactoredAtom>();
                _atomsByPredicate.Add(atom.Predicate!, atoms);
            }

            // One atom per shape is enough: the cache holds one projection for all of them.
            if (atoms.All(existing => existing.ProjectionId != atom.ProjectionId)) atoms.Add(atom);
        }
        foreach (SchemaPlan plan in _schemaPlans)
            plan.UnchangingTuples = ProjectUnchangingTuples(plan);

        _observedGen0Collections = GC.CollectionCount(0);
    }

    // Atoms reading the same predicate through the same argument pattern share one projection per state.
    private static int AssignProjectionIds(SchemaPlan[] plans)
    {
        var identifiers = new Dictionary<(Predicate, string), int>();
        foreach (FactoredAtom atom in plans.SelectMany(plan => plan.Atoms))
        {
            if (atom.Kind != FactoredAtomKind.Fluent) continue;

            (Predicate, string) key = (atom.Predicate!, atom.ProjectionShape());
            if (!identifiers.TryGetValue(key, out int identifier))
            {
                identifier = identifiers.Count;
                identifiers.Add(key, identifier);
            }

            atom.ProjectionId = identifier;
        }

        return identifiers.Count;
    }

    private void Patch(FactoredProjectionCache cache, List<int> toAdd, List<int> toRemove)
    {
        foreach (int fact in toRemove) PatchRow(cache, fact, add: false);
        foreach (int fact in toAdd) PatchRow(cache, fact, add: true);
    }

    private void PatchRow(FactoredProjectionCache cache, int fact, bool add)
    {
        Fact<Fluent> changed = Problem.Context.GetFact(new FluentIndex(fact));
        if (!_atomsByPredicate.TryGetValue(changed.Predicate, out List<FactoredAtom>? atoms)) return;

        foreach (FactoredAtom atom in atoms)
        {
            if (cache.Projections[atom.ProjectionId] is not TupleSet rows) continue;
            if (!add)
            {
                rows.Remove(fact);
                continue;
            }

            var projected = new Constant[atom.Parameters.Length];
            if (atom.TryProject(changed.Arguments, projected)) rows.Add(fact, projected);
        }
    }

    private List<Fact> StaticTrueFacts(Predicate predicate)
        => _staticTrueFacts.GetValueOrDefault(predicate) ?? EmptyFacts;

    private static readonly List<Fact> EmptyFacts = new();

    public IEnumerable<FactoredAction> GetFactoredApplicableActions(ExtendedState state)
    {
        ValidateState(state);
        return EnumerateGroups(state, cache: null);
    }

    public IEnumerable<FactoredAction> GetFactoredApplicableActions(
        ExtendedState state,
        FactoredProjectionCache cache)
    {
        ValidateState(state);
        ArgumentNullException.ThrowIfNull(cache);
        cache.EnsureUsableBy(this);
        return EnumerateGroups(state, cache);
    }

    public FactoredProjectionCache CreateProjectionCache(ExtendedState referenceState)
    {
        ValidateState(referenceState);

        var facts = new StateFacts(_staticTrueFacts, referenceState, _projectionCount, cache: null);
        var projections = new object?[_projectionCount];
        foreach (FactoredAtom atom in _atomsByPredicate.Values.SelectMany(atoms => atoms))
            projections[atom.ProjectionId] = ProjectTracked(atom, facts.GetTrueFacts(atom.Predicate!));

        ReadOnlySpan<ulong> words = referenceState.State.Bitboard;
        return new FactoredProjectionCache(this, words.ToArray(), projections);
    }

    private IEnumerable<FactoredAction> EnumerateGroups(ExtendedState state, FactoredProjectionCache? cache)
    {
        ScavengeCollectedActions();

        // Patching costs the delta twice, since the relations have to be put back; past that, projecting is cheaper.
        var added = new List<int>();
        var removed = new List<int>();
        bool borrowed = cache is not null
            && cache.TryDescribeDelta(state.State, added, removed, Math.Max(16, state.State.Bitboard.Length * 4));

        var facts = new StateFacts(_staticTrueFacts, state, _projectionCount, borrowed ? cache : null);
        if (!borrowed)
        {
            foreach (SchemaPlan plan in _schemaPlans)
            {
                foreach (FactoredAction group in SolveSchema(plan, facts))
                    yield return group;
            }

            yield break;
        }

        cache!.IsBorrowed = true;
        Patch(cache, added, removed);
        try
        {
            foreach (SchemaPlan plan in _schemaPlans)
            {
                foreach (FactoredAction group in SolveSchema(plan, facts))
                    yield return group;
            }
        }
        finally
        {
            // The inverse delta restores the anchor, so the next call starts from the same place as this one.
            Patch(cache, removed, added);
            cache.IsBorrowed = false;
        }
    }

    private static IEnumerable<FactoredAction> SolveSchema(SchemaPlan plan, StateFacts facts)
    {
        if (plan.IsPermanentlyInapplicable) yield break;

        TupleSet?[] tuples = MaterializeTrueTuples(plan, facts);
        var candidates = new List<Constant>[plan.ParameterCount];
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
            candidates[parameter] = new List<Constant>(plan.Domains[parameter]);

        if (!PropagateAll(plan, tuples, candidates, facts)) yield break;

        foreach (FactoredAction group in Solve(plan, tuples, candidates, facts))
            yield return group;
    }

    public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state)
    {
        ValidateState(state);
        return EnumerateGroups(state, cache: null).SelectMany(group => group.Expand());
    }

    public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state, int maxActions)
    {
        ValidateState(state);
        if (maxActions < 0) throw new ArgumentOutOfRangeException(nameof(maxActions));
        return EnumerateGroups(state, cache: null).SelectMany(group => group.Expand()).Take(maxActions);
    }

    public void CollectApplicableActions(ExtendedState state, List<GroundAction> destination)
    {
        ValidateState(state);
        ArgumentNullException.ThrowIfNull(destination);

        destination.Clear();
        foreach (FactoredAction group in EnumerateGroups(state, cache: null))
            destination.AddRange(group.Expand());
    }

    // Walking the weak caches on every expansion costs more than it reclaims, so wait for a collection.
    private void ScavengeCollectedActions()
    {
        Problem.Context.ScavengeActionComponentCaches();
        int gen0Collections = GC.CollectionCount(0);
        if (gen0Collections == _observedGen0Collections) return;

        _observedGen0Collections = gen0Collections;
        foreach (SchemaPlan plan in _schemaPlans)
            plan.Actions.Scavenge();
    }

    private void ValidateState(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.State.Context, Problem.Context))
            throw new InvalidOperationException("State and applicable-action generator belong to different problem contexts.");
    }

    /// <summary>
    /// Enumerates assignments of a candidate set to every parameter. A parameter stays set-valued unless a positive
    /// precondition still ties it to another set-valued parameter, in which case the generator branches on it.
    /// </summary>
    private static IEnumerable<FactoredAction> Solve(
        SchemaPlan plan,
        TupleSet?[] tuples,
        List<Constant>[] candidates,
        StateFacts facts)
    {
        int branchParameter = ChooseBranchParameter(plan, candidates);
        if (branchParameter < 0)
        {
            yield return BuildGroup(plan, tuples, candidates);
            yield break;
        }

        // Mirrors the representative-binding rule of the clique generator. Branching reaches a witness only once
        // no atom still couples a witness to an undetermined transition-relevant parameter, because such an atom
        // would itself require branching and the choice above prefers its transition-relevant parameters. The
        // remaining parameters are therefore unaffected by this choice, so one completion stands for all of them.
        bool isWitnessBranch = plan.WitnessParameters[branchParameter];

        var worklist = new Stack<int>();
        foreach (Constant value in candidates[branchParameter])
        {
            List<Constant>[] branch = CloneCandidates(candidates);
            branch[branchParameter] = new List<Constant> { value };
            // Semi-join on the parameter just fixed, so deeper branches scan progressively smaller relations.
            TupleSet?[] branchTuples = SemiJoin(plan, tuples, branchParameter, value);
            worklist.Clear();
            worklist.Push(branchParameter);
            if (!Propagate(plan, branchTuples, branch, facts, worklist)) continue;

            bool emitted = false;
            foreach (FactoredAction group in Solve(plan, branchTuples, branch, facts))
            {
                emitted = true;
                yield return group;
            }

            if (isWitnessBranch && emitted) yield break;
        }
    }

    private static int ChooseBranchParameter(SchemaPlan plan, List<Constant>[] candidates)
    {
        int chosen = -1;
        foreach (FactoredAtom atom in plan.Atoms)
        {
            if (!RequiresBranching(plan, atom, candidates)) continue;
            foreach (int parameter in atom.Parameters)
            {
                if (candidates[parameter].Count <= 1) continue;
                if (chosen < 0 || IsBetterBranchParameter(plan, candidates, parameter, chosen))
                    chosen = parameter;
            }
        }

        return chosen;
    }

    // Branching on a transition-relevant parameter first keeps the witness collapse above sound.
    private static bool IsBetterBranchParameter(
        SchemaPlan plan,
        List<Constant>[] candidates,
        int parameter,
        int incumbent)
    {
        if (plan.WitnessParameters[parameter] != plan.WitnessParameters[incumbent])
            return !plan.WitnessParameters[parameter];

        return candidates[parameter].Count < candidates[incumbent].Count;
    }

    /// <summary>
    /// A positive atom over several undetermined parameters is a relation, not a product, so its parameters must be
    /// branched on. A negative atom normally becomes an exclusion instead, but one that mentions a witness is
    /// branched on too, because collapsing a witness it constrains could drop otherwise applicable bindings.
    /// </summary>
    private static bool RequiresBranching(SchemaPlan plan, FactoredAtom atom, List<Constant>[] candidates)
    {
        int undetermined = 0;
        bool mentionsWitness = false;
        foreach (int parameter in atom.Parameters)
        {
            if (candidates[parameter].Count <= 1) continue;
            undetermined++;
            mentionsWitness |= plan.WitnessParameters[parameter];
        }

        if (undetermined < 2) return false;
        return atom.IsPositive || atom.IsProbed || mentionsWitness;
    }

    private static List<Constant>[] CloneCandidates(List<Constant>[] candidates)
    {
        var clone = new List<Constant>[candidates.Length];
        for (int parameter = 0; parameter < candidates.Length; parameter++)
            clone[parameter] = new List<Constant>(candidates[parameter]);
        return clone;
    }

    private static FactoredAction BuildGroup(
        SchemaPlan plan,
        TupleSet?[] tuples,
        List<Constant>[] candidates)
    {
        var sets = new Constant[plan.ParameterCount][];
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
        {
            // A witness changes no part of the transition, so the group carries one representative of it.
            sets[parameter] = plan.WitnessParameters[parameter] && candidates[parameter].Count > 1
                ? [candidates[parameter][0]]
                : candidates[parameter].ToArray();
        }

        var exclusions = new List<FactoredExclusion>();
        HashSet<Constant>?[]? membership = null;
        for (int index = 0; index < plan.Atoms.Length; index++)
        {
            FactoredAtom atom = plan.Atoms[index];
            if (atom.IsPositive || atom.IsProbed) continue;

            int undetermined = 0;
            int first = -1;
            int second = -1;
            foreach (int parameter in atom.Parameters)
            {
                if (sets[parameter].Length <= 1) continue;
                undetermined++;
                if (first < 0) first = parameter;
                else if (second < 0) second = parameter;
            }

            if (undetermined < 2) continue;

            if (atom.Kind == FactoredAtomKind.Equality)
            {
                exclusions.Add(FactoredExclusion.Distinctness(first, second));
                continue;
            }

            membership ??= new HashSet<Constant>?[plan.ParameterCount];
            Constant[] forbidden = CollectForbiddenCombinations(atom, tuples[index]!, sets, membership);
            if (forbidden.Length > 0)
                exclusions.Add(FactoredExclusion.ForbiddenCombinations(atom.Parameters, forbidden));
        }

        return new FactoredAction(
            plan.GroundingPlan,
            plan.Actions,
            sets,
            plan.WitnessParameters,
            exclusions);
    }

    /// <summary>
    /// Keeps only the tuples a member of this group could actually take, so that expansion tests a short list.
    /// The membership sets are built once per group and shared by its atoms.
    /// </summary>
    private static Constant[] CollectForbiddenCombinations(
        FactoredAtom atom,
        TupleSet trueTuples,
        Constant[][] sets,
        HashSet<Constant>?[] membership)
    {
        foreach (int parameter in atom.Parameters)
        {
            membership[parameter] ??= new HashSet<Constant>(
                sets[parameter],
                ReferenceEqualityComparer.Instance);
        }

        var forbidden = new List<Constant>();
        for (int row = 0; row < trueTuples.Count; row++)
        {
            bool reachable = true;
            for (int column = 0; column < trueTuples.Arity; column++)
            {
                if (!membership[atom.Parameters[column]]!.Contains(trueTuples.At(row, column)))
                {
                    reachable = false;
                    break;
                }
            }

            if (!reachable) continue;
            for (int column = 0; column < trueTuples.Arity; column++)
                forbidden.Add(trueTuples.At(row, column));
        }

        return forbidden.ToArray();
    }
}
