using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Engines;

public interface IFactoredApplicableActionGenerator : IApplicableActionGenerator
{
    /// <summary>
    /// The applicable actions grouped into Cartesian products over the schema parameters. The groups are pairwise
    /// disjoint, so expanding all of them yields every applicable action exactly once.
    /// </summary>
    IEnumerable<FactoredAction> GetFactoredApplicableActions(ExtendedState state);
}

/// <summary>
/// A lifted generator that answers each schema's precondition as a conjunctive query and reports the answers as
/// Cartesian products instead of individual bindings. Parameters that the precondition leaves independent stay
/// set-valued, which keeps states with an enormous branching factor representable.
/// </summary>
public sealed partial class FactoredApplicableActionGenerator : IFactoredApplicableActionGenerator
{
    public Problem Problem { get; }
    private readonly SchemaPlan[] _schemaPlans;
    private readonly Dictionary<Predicate, List<Fact>> _staticTrueFacts;
    private readonly HashSet<Predicate> _changingPredicates;
    private int _observedGen0Collections;

    internal FactoredApplicableActionGenerator(Problem problem)
    {
        Problem = problem ?? throw new ArgumentNullException(nameof(problem));
        _schemaPlans = problem.Domain.Actions
            .Select(schema => CompileSchema(problem, schema))
            .ToArray();

        _staticTrueFacts = new Dictionary<Predicate, List<Fact>>(ReferenceEqualityComparer.Instance);
        _changingPredicates = new HashSet<Predicate>(ReferenceEqualityComparer.Instance);
        foreach (FactoredAtom atom in _schemaPlans.SelectMany(plan => plan.Atoms))
        {
            if (atom.Predicate is null) continue;
            if (atom.Kind == FactoredAtomKind.Fluent) _changingPredicates.Add(atom.Predicate);
        }

        _ = problem.InitialState;
        foreach (FactoredAtom atom in _schemaPlans.SelectMany(plan => plan.Atoms))
        {
            if (atom.Kind != FactoredAtomKind.Static || _staticTrueFacts.ContainsKey(atom.Predicate!)) continue;
            _staticTrueFacts[atom.Predicate!] = problem.Context
                .GetFacts(atom.Predicate!)
                .Where(fact => problem.Context.IsTrue(new StaticIndex(((Fact<Static>)fact).LocalIndex)))
                .ToList();
        }

        foreach (SchemaPlan plan in _schemaPlans)
            plan.UnchangingTuples = ProjectUnchangingTuples(plan);

        _observedGen0Collections = GC.CollectionCount(0);
    }

    private List<Fact> StaticTrueFacts(Predicate predicate)
        => _staticTrueFacts.GetValueOrDefault(predicate) ?? new List<Fact>();

    public IEnumerable<FactoredAction> GetFactoredApplicableActions(ExtendedState state)
    {
        ValidateState(state);
        return EnumerateGroups(state);
    }

    private IEnumerable<FactoredAction> EnumerateGroups(ExtendedState state)
    {
        ScavengeCollectedActions();
        var facts = new StateFacts(_staticTrueFacts, state, _changingPredicates);
        foreach (SchemaPlan plan in _schemaPlans)
        {
            foreach (FactoredAction group in SolveSchema(plan, facts))
                yield return group;
        }
    }

    public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state)
    {
        ValidateState(state);
        return EnumerateGroups(state).SelectMany(group => group.Expand());
    }

    public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state, int maxActions)
    {
        ValidateState(state);
        if (maxActions < 0) throw new ArgumentOutOfRangeException(nameof(maxActions));
        return EnumerateGroups(state).SelectMany(group => group.Expand()).Take(maxActions);
    }

    public void CollectApplicableActions(ExtendedState state, List<GroundAction> destination)
    {
        ValidateState(state);
        ArgumentNullException.ThrowIfNull(destination);

        destination.Clear();
        foreach (FactoredAction group in EnumerateGroups(state))
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

    private List<FactoredAction> SolveSchema(SchemaPlan plan, StateFacts facts)
    {
        var groups = new List<FactoredAction>();
        if (plan.IsPermanentlyInapplicable) return groups;

        List<Constant[]>?[] tuples = MaterializeTrueTuples(plan, facts);
        var candidates = new List<Constant>[plan.ParameterCount];
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
            candidates[parameter] = new List<Constant>(plan.Domains[parameter]);

        if (!PropagateAll(plan, tuples, candidates, facts)) return groups;

        Solve(plan, tuples, candidates, facts, groups);
        return groups;
    }

    /// <summary>
    /// Enumerates assignments of a candidate set to every parameter. A parameter stays set-valued unless a positive
    /// precondition still ties it to another set-valued parameter, in which case the generator branches on it.
    /// </summary>
    private void Solve(
        SchemaPlan plan,
        List<Constant[]>?[] tuples,
        List<Constant>[] candidates,
        StateFacts facts,
        List<FactoredAction> groups)
    {
        int branchParameter = ChooseBranchParameter(plan, candidates);
        if (branchParameter < 0)
        {
            groups.Add(BuildGroup(plan, tuples, candidates));
            return;
        }

        // Mirrors the representative-binding rule of the clique generator. Branching reaches a witness only once
        // no atom still couples a witness to an undetermined transition-relevant parameter, because such an atom
        // would itself require branching and the choice above prefers its transition-relevant parameters. The
        // remaining parameters are therefore unaffected by this choice, so one completion stands for all of them.
        bool isWitnessBranch = plan.WitnessParameters[branchParameter];

        var worklist = new Stack<int>();
        int before = groups.Count;
        foreach (Constant value in candidates[branchParameter])
        {
            List<Constant>[] branch = CloneCandidates(candidates);
            branch[branchParameter] = new List<Constant> { value };
            // Semi-join on the parameter just fixed, so deeper branches scan progressively smaller relations.
            List<Constant[]>?[] branchTuples = SemiJoin(plan, tuples, branchParameter, value);
            worklist.Clear();
            worklist.Push(branchParameter);
            if (!Propagate(plan, branchTuples, branch, facts, worklist)) continue;

            Solve(plan, branchTuples, branch, facts, groups);
            if (isWitnessBranch && groups.Count > before) break;
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
        List<Constant[]>?[] tuples,
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
            Constant[][] forbidden = CollectForbiddenTuples(atom, tuples[index]!, sets, membership);
            if (forbidden.Length > 0)
                exclusions.Add(FactoredExclusion.ForbiddenTuples(atom.Parameters, forbidden));
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
    private static Constant[][] CollectForbiddenTuples(
        FactoredAtom atom,
        List<Constant[]> trueTuples,
        Constant[][] sets,
        HashSet<Constant>?[] membership)
    {
        foreach (int parameter in atom.Parameters)
        {
            membership[parameter] ??= new HashSet<Constant>(
                sets[parameter],
                ReferenceEqualityComparer.Instance);
        }

        var forbidden = new List<Constant[]>();
        foreach (Constant[] tuple in trueTuples)
        {
            bool reachable = true;
            for (int column = 0; column < atom.Parameters.Length; column++)
            {
                if (!membership[atom.Parameters[column]]!.Contains(tuple[column]))
                {
                    reachable = false;
                    break;
                }
            }

            if (reachable) forbidden.Add(tuple);
        }

        return forbidden.ToArray();
    }
}
