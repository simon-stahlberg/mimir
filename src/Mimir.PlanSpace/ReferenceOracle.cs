using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed class OracleTargets
{
    public IReadOnlyList<int> ResolverIndexPerFlaw { get; }
    public IReadOnlyList<int> StepIdsToDelete { get; }
    public int MatchedStepCount { get; }

    internal OracleTargets(IReadOnlyList<int> resolverIndexPerFlaw, IReadOnlyList<int> stepIdsToDelete, int matchedStepCount)
    {
        ResolverIndexPerFlaw = resolverIndexPerFlaw;
        StepIdsToDelete = stepIdsToDelete;
        MatchedStepCount = matchedStepCount;
    }
}

public static class ReferenceOracle
{
    public static OracleTargets Targets(PartialOrderPlan current, FlawSet flaws, PartialOrderPlan reference)
    {
        ArgumentNullException.ThrowIfNull(current);
        ArgumentNullException.ThrowIfNull(flaws);
        ArgumentNullException.ThrowIfNull(reference);
        if (!ReferenceEquals(flaws.Plan, current))
            throw new ArgumentException("The flaw set was not computed from the current plan.", nameof(flaws));
        if (flaws.Version != current.Version)
            throw new ArgumentException("The flaw set was not computed from the current plan in its current state.", nameof(flaws));
        if (!ReferenceEquals(current.Problem, reference.Problem))
            throw new ArgumentException("The reference plan belongs to a different problem.", nameof(reference));

        var matching = new Matching(current, reference);
        var deletions = current.Steps
            .Where(step => step.Kind == StepKind.Step && !matching.IsMatched(step.Id))
            .Select(step => step.Id)
            .ToList();

        int[] openConditionTargets = OpenConditionTargets(flaws, matching, reference);
        var targets = new int[flaws.Flaws.Count];
        for (int i = 0; i < targets.Length; i++)
        {
            Flaw flaw = flaws.Flaws[i];
            targets[i] = flaw.Kind switch
            {
                FlawKind.OpenCondition => openConditionTargets[i],
                FlawKind.Threat => ThreatTarget(flaw, current, matching, reference),
                FlawKind.UnboundParameter => UnboundParameterTarget(flaw, matching, reference),
                _ => throw new NotSupportedException(flaw.Kind.ToString())
            };
        }

        return new OracleTargets(targets, deletions, matching.MatchedStepCount);
    }

    // Apply merges equal ghosts adopted in one round into one step and creates one step per distinct ghost,
    // so each reference step is adopted through one ghost per round, the most bound one among its open
    // conditions, and each ghost stands for one reference step, the earliest; the other open conditions
    // wait for the next round and link to the created step.
    private static int[] OpenConditionTargets(FlawSet flaws, Matching matching, PartialOrderPlan reference)
    {
        var targets = new int[flaws.Flaws.Count];
        var adoptedReferences = new int[flaws.Flaws.Count];
        var ghostByReference = new Dictionary<int, int>();
        for (int i = 0; i < targets.Length; i++)
        {
            Flaw flaw = flaws.Flaws[i];
            adoptedReferences[i] = -1;
            if (flaw.Kind != FlawKind.OpenCondition) continue;
            targets[i] = OpenConditionTarget(flaw, flaws, matching, reference, out adoptedReferences[i]);
            if (adoptedReferences[i] < 0) continue;

            int ghostIndex = flaw.Resolvers[targets[i]].GhostIndex;
            if (ghostByReference.TryGetValue(adoptedReferences[i], out int chosen) && BoundCount(flaws.Ghosts[chosen]) >= BoundCount(flaws.Ghosts[ghostIndex])) continue;
            ghostByReference[adoptedReferences[i]] = ghostIndex;
        }

        var referenceByGhost = new Dictionary<int, int>();
        foreach (int referenceId in ghostByReference.Keys.Order()) referenceByGhost.TryAdd(ghostByReference[referenceId], referenceId);

        for (int i = 0; i < targets.Length; i++)
        {
            if (adoptedReferences[i] < 0) continue;
            int ghostIndex = flaws.Flaws[i].Resolvers[targets[i]].GhostIndex;
            if (ghostByReference[adoptedReferences[i]] != ghostIndex || referenceByGhost[ghostIndex] != adoptedReferences[i]) targets[i] = 0;
        }

        return targets;
    }

    private static int BoundCount(GhostStep ghost) => ghost.Binding.Count(constant => constant is not null);

    // adoptedReference is the reference step the target adopts a ghost for, -1 when it links or does nothing.
    private static int OpenConditionTarget(Flaw flaw, FlawSet flaws, Matching matching, PartialOrderPlan reference, out int adoptedReference)
    {
        adoptedReference = -1;
        if (!matching.TryGetReference(flaw.StepId, out int referenceConsumer)) return 0;

        CausalLink? referenceLink = reference.Links.FirstOrDefault(link => link.ConsumerId == referenceConsumer && link.Fact.Equals(flaw.Fact));
        if (referenceLink is null) return 0;

        if (matching.TryGetCurrent(referenceLink.ProducerId, out int currentProducer))
            return IndexOf(flaw, resolver => resolver.Kind == ResolverKind.LinkExisting && resolver.StepId == currentProducer);

        PlanStep referenceProducer = reference.GetStep(referenceLink.ProducerId);
        int target = IndexOf(flaw, resolver => resolver.Kind == ResolverKind.AdoptGhost
            && IsConsistent(flaws.Ghosts[resolver.GhostIndex], referenceProducer));
        if (target > 0) adoptedReference = referenceProducer.Id;
        return target;
    }

    private static int ThreatTarget(Flaw flaw, PartialOrderPlan current, Matching matching, PartialOrderPlan reference)
    {
        int delete = IndexOf(flaw, resolver => resolver.Kind == ResolverKind.DeleteStep);
        if (!matching.TryGetReference(flaw.StepId, out int referenceThreatener)) return delete;

        // An unmatched producer or consumer is deleted this round, which removes the link and the threat.
        CausalLink link = current.GetLink(flaw.LinkId);
        if (!matching.TryGetReference(link.ProducerId, out int referenceProducer)) return 0;
        if (!matching.TryGetReference(link.ConsumerId, out int referenceConsumer)) return 0;

        int demote = IndexOf(flaw, resolver => resolver.Kind == ResolverKind.Demote);
        if (demote > 0 && reference.IsBefore(referenceThreatener, referenceProducer)) return demote;
        int promote = IndexOf(flaw, resolver => resolver.Kind == ResolverKind.Promote);
        if (promote > 0 && reference.IsBefore(referenceConsumer, referenceThreatener)) return promote;
        // The threatener is ordered incompatibly with its reference counterpart, so its match is wrong.
        return delete;
    }

    private static int UnboundParameterTarget(Flaw flaw, Matching matching, PartialOrderPlan reference)
    {
        if (!matching.TryGetReference(flaw.StepId, out int referenceStep)) return 0;

        Constant? value = reference.GetStep(referenceStep).Binding[flaw.ParameterIndex];
        return IndexOf(flaw, resolver => resolver.Kind == ResolverKind.Bind && ReferenceEquals(resolver.Object, value));
    }

    private static int IndexOf(Flaw flaw, Func<Resolver, bool> predicate)
    {
        for (int i = 1; i < flaw.Resolvers.Count; i++)
        {
            if (predicate(flaw.Resolvers[i])) return i;
        }

        return 0;
    }

    private static bool IsConsistent(GhostStep ghost, PlanStep referenceStep)
    {
        if (!ReferenceEquals(ghost.Schema, referenceStep.Schema)) return false;
        for (int i = 0; i < ghost.Binding.Count; i++)
        {
            if (ghost.Binding[i] is not null && !ReferenceEquals(ghost.Binding[i], referenceStep.Binding[i])) return false;
        }

        return true;
    }

    // Matches current steps to reference steps top-down along causal links (a producer of a matched
    // consumer matches the reference producer of the same fact) and the remaining steps greedily.
    private sealed class Matching
    {
        private readonly PartialOrderPlan _current;
        private readonly PartialOrderPlan _reference;
        private readonly Dictionary<int, int> _referenceByCurrent = new() { [0] = 0, [1] = 1 };
        private readonly Dictionary<int, int> _currentByReference = new() { [0] = 0, [1] = 1 };

        public int MatchedStepCount => _referenceByCurrent.Count - 2;

        public Matching(PartialOrderPlan current, PartialOrderPlan reference)
        {
            _current = current;
            _reference = reference;
            MatchAlongLinks();
            IEnumerable<PlanStep> steps = current.Steps.Where(step => step.Kind == StepKind.Step);
            foreach (PlanStep step in steps.Where(step => step.IsFullyBound).Concat(steps.Where(step => !step.IsFullyBound)))
            {
                if (IsMatched(step.Id) || !TryMatchGreedily(step)) continue;
                MatchAlongLinks();
            }
        }

        private void Match(int currentId, int referenceId)
        {
            _referenceByCurrent.Add(currentId, referenceId);
            _currentByReference.Add(referenceId, currentId);
        }

        private void MatchAlongLinks()
        {
            bool progress = true;
            while (progress)
            {
                progress = false;
                foreach (CausalLink link in _current.Links)
                {
                    if (IsMatched(link.ProducerId) || !TryGetReference(link.ConsumerId, out int referenceConsumer)) continue;
                    CausalLink? referenceLink = _reference.Links.FirstOrDefault(candidate => candidate.ConsumerId == referenceConsumer && candidate.Fact.Equals(link.Fact));
                    if (referenceLink is null || _currentByReference.ContainsKey(referenceLink.ProducerId)) continue;
                    if (!IsConsistent(_reference.GetStep(referenceLink.ProducerId), _current.GetStep(link.ProducerId))) continue;
                    Match(link.ProducerId, referenceLink.ProducerId);
                    progress = true;
                }
            }
        }

        private bool TryMatchGreedily(PlanStep step)
        {
            var produced = _current.Links.Where(link => link.ProducerId == step.Id).Select(link => link.Fact).ToHashSet();
            PlanStep? best = null;
            int bestOverlap = -1;
            foreach (PlanStep candidate in _reference.Steps)
            {
                if (candidate.Kind != StepKind.Step || _currentByReference.ContainsKey(candidate.Id) || !IsConsistent(candidate, step)) continue;
                int overlap = _reference.Links.Count(link => link.ProducerId == candidate.Id && produced.Contains(link.Fact));
                if (overlap <= bestOverlap) continue;
                best = candidate;
                bestOverlap = overlap;
            }

            if (best is null) return false;
            Match(step.Id, best.Id);
            return true;
        }

        private static bool IsConsistent(PlanStep referenceStep, PlanStep step)
        {
            if (!ReferenceEquals(referenceStep.Schema, step.Schema)) return false;
            for (int i = 0; i < step.Binding.Count; i++)
            {
                if (step.Binding[i] is not null && !ReferenceEquals(step.Binding[i], referenceStep.Binding[i])) return false;
            }

            return true;
        }

        public bool IsMatched(int currentId) => _referenceByCurrent.ContainsKey(currentId);
        public bool TryGetReference(int currentId, out int referenceId) => _referenceByCurrent.TryGetValue(currentId, out referenceId);
        public bool TryGetCurrent(int referenceId, out int currentId) => _currentByReference.TryGetValue(referenceId, out currentId);
    }
}
