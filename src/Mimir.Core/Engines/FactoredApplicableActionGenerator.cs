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

    internal FactoredApplicableActionGenerator(Problem problem)
    {
        Problem = problem ?? throw new ArgumentNullException(nameof(problem));
        _schemaPlans = problem.Domain.Actions
            .Select(schema => CompileSchema(problem, schema))
            .ToArray();
        _ = problem.InitialState;
    }

    public IEnumerable<FactoredAction> GetFactoredApplicableActions(ExtendedState state)
    {
        ValidateState(state);
        return EnumerateGroups(state);
    }

    private IEnumerable<FactoredAction> EnumerateGroups(ExtendedState state)
    {
        Dictionary<Predicate, List<Fact>> derivedFacts = GroupTrueDerivedFacts(state);
        foreach (SchemaPlan plan in _schemaPlans)
        {
            foreach (FactoredAction group in SolveSchema(plan, state, derivedFacts))
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

    private void ValidateState(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.State.Context, Problem.Context))
            throw new InvalidOperationException("State and applicable-action generator belong to different problem contexts.");
    }

    private static Dictionary<Predicate, List<Fact>> GroupTrueDerivedFacts(ExtendedState state)
    {
        var grouped = new Dictionary<Predicate, List<Fact>>(ReferenceEqualityComparer.Instance);
        foreach (Fact<Derived> fact in state.GetTrueDerivedFacts())
        {
            if (!grouped.TryGetValue(fact.Predicate, out List<Fact>? facts))
            {
                facts = new List<Fact>();
                grouped.Add(fact.Predicate, facts);
            }

            facts.Add(fact);
        }

        return grouped;
    }

    private List<FactoredAction> SolveSchema(
        SchemaPlan plan,
        ExtendedState state,
        Dictionary<Predicate, List<Fact>> derivedFacts)
    {
        var groups = new List<FactoredAction>();
        if (plan.IsPermanentlyInapplicable) return groups;

        List<Constant[]>?[] tuples = MaterializeTrueTuples(plan, state, derivedFacts);
        var candidates = new List<Constant>[plan.ParameterCount];
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
            candidates[parameter] = new List<Constant>(plan.Domains[parameter]);

        if (!Propagate(plan, tuples, candidates)) return groups;

        foreach (List<Constant>[] solution in Solve(plan, tuples, candidates))
            groups.Add(BuildGroup(plan, tuples, solution));

        return groups;
    }

    /// <summary>
    /// Enumerates assignments of a candidate set to every parameter. A parameter stays set-valued unless a positive
    /// precondition still ties it to another set-valued parameter, in which case the generator branches on it.
    /// </summary>
    private List<List<Constant>[]> Solve(
        SchemaPlan plan,
        List<Constant[]>?[] tuples,
        List<Constant>[] candidates)
    {
        var results = new List<List<Constant>[]>();
        int branchParameter = ChooseBranchParameter(plan, candidates);
        if (branchParameter < 0)
        {
            results.Add(CollapseWitnesses(plan, candidates));
            return results;
        }

        // Mirrors the representative-binding rule of the clique generator. Branching reaches a witness only once
        // no atom still couples a witness to an undetermined transition-relevant parameter, because such an atom
        // would itself require branching and the choice above prefers its transition-relevant parameters. The
        // remaining parameters are therefore unaffected by this choice, so one completion stands for all of them.
        bool isWitnessBranch = plan.WitnessParameters[branchParameter];

        foreach (Constant value in candidates[branchParameter])
        {
            List<Constant>[] branch = CloneCandidates(candidates);
            branch[branchParameter] = new List<Constant> { value };
            if (!Propagate(plan, tuples, branch)) continue;

            results.AddRange(Solve(plan, tuples, branch));
            if (isWitnessBranch && results.Count > 0) break;
        }

        return results;
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
        return atom.IsPositive || mentionsWitness;
    }

    private static List<Constant>[] CloneCandidates(List<Constant>[] candidates)
    {
        var clone = new List<Constant>[candidates.Length];
        for (int parameter = 0; parameter < candidates.Length; parameter++)
            clone[parameter] = new List<Constant>(candidates[parameter]);
        return clone;
    }

    private static List<Constant>[] CollapseWitnesses(SchemaPlan plan, List<Constant>[] candidates)
    {
        List<Constant>[] collapsed = CloneCandidates(candidates);
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
        {
            if (!plan.WitnessParameters[parameter] || collapsed[parameter].Count <= 1) continue;
            collapsed[parameter] = new List<Constant> { collapsed[parameter][0] };
        }

        return collapsed;
    }

    private static FactoredAction BuildGroup(
        SchemaPlan plan,
        List<Constant[]>?[] tuples,
        List<Constant>[] candidates)
    {
        var sets = new Constant[plan.ParameterCount][];
        for (int parameter = 0; parameter < plan.ParameterCount; parameter++)
            sets[parameter] = candidates[parameter].ToArray();

        var exclusions = new List<FactoredExclusion>();
        for (int index = 0; index < plan.Atoms.Length; index++)
        {
            FactoredAtom atom = plan.Atoms[index];
            if (atom.IsPositive) continue;

            int[] undetermined = atom.Parameters.Where(parameter => candidates[parameter].Count > 1).ToArray();
            if (undetermined.Length < 2) continue;

            if (atom.Kind == FactoredAtomKind.Equality)
            {
                exclusions.Add(FactoredExclusion.Distinctness(undetermined[0], undetermined[1]));
                continue;
            }

            Constant[][] forbidden = CollectForbiddenTuples(atom, tuples[index]!, candidates);
            if (forbidden.Length > 0)
                exclusions.Add(FactoredExclusion.ForbiddenTuples(atom.Parameters, forbidden));
        }

        return new FactoredAction(
            plan.GroundingPlan,
            plan.GroundingPlan.Context.Problem,
            sets,
            plan.WitnessParameters,
            exclusions);
    }

    private static Constant[][] CollectForbiddenTuples(
        FactoredAtom atom,
        List<Constant[]> trueTuples,
        List<Constant>[] candidates)
    {
        var forbidden = new List<Constant[]>();
        foreach (Constant[] tuple in trueTuples)
        {
            bool reachable = true;
            for (int column = 0; column < atom.Parameters.Length; column++)
            {
                if (!candidates[atom.Parameters[column]].Contains(tuple[column]))
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
