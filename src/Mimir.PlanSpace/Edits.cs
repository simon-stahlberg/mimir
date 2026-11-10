using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed class EditResult
{
    public int AppliedCount { get; }
    public IReadOnlyList<int> DroppedFlawIndices { get; }
    public IReadOnlyList<int> CreatedStepIds { get; }
    public IReadOnlyList<int> DeletedStepIds { get; }

    internal EditResult(int appliedCount, IReadOnlyList<int> droppedFlawIndices, IReadOnlyList<int> createdStepIds, IReadOnlyList<int> deletedStepIds)
    {
        AppliedCount = appliedCount;
        DroppedFlawIndices = droppedFlawIndices;
        CreatedStepIds = createdStepIds;
        DeletedStepIds = deletedStepIds;
    }
}

public sealed partial class PartialOrderPlan
{
    public EditResult Apply(FlawSet flaws, IReadOnlyList<int> chosenResolverIndex, IReadOnlyList<int> stepIdsToDelete)
    {
        ArgumentNullException.ThrowIfNull(flaws);
        ArgumentNullException.ThrowIfNull(chosenResolverIndex);
        ArgumentNullException.ThrowIfNull(stepIdsToDelete);
        if (!ReferenceEquals(flaws.Plan, this) || flaws.Version != _version)
            throw new InvalidOperationException("The flaw set was not computed from this plan in its current state.");
        if (chosenResolverIndex.Count != flaws.Flaws.Count)
            throw new ArgumentException($"Expected {flaws.Flaws.Count} resolver choices, got {chosenResolverIndex.Count}.", nameof(chosenResolverIndex));

        var chosen = new Resolver[flaws.Flaws.Count];
        for (int i = 0; i < chosen.Length; i++)
        {
            IReadOnlyList<Resolver> resolvers = flaws.Flaws[i].Resolvers;
            if (chosenResolverIndex[i] < 0 || chosenResolverIndex[i] >= resolvers.Count)
                throw new ArgumentOutOfRangeException(nameof(chosenResolverIndex), chosenResolverIndex[i], $"Flaw {i} has {resolvers.Count} resolvers.");
            chosen[i] = resolvers[chosenResolverIndex[i]];
        }

        var deleted = new List<int>();
        foreach (int id in stepIdsToDelete)
        {
            if (GetStep(id).Kind != StepKind.Step)
                throw new ArgumentException($"Step {id} is the {GetStep(id).Kind} step and cannot be deleted.", nameof(stepIdsToDelete));
            if (!deleted.Contains(id)) deleted.Add(id);
        }

        foreach (Resolver resolver in chosen)
        {
            if (resolver.Kind == ResolverKind.DeleteStep && !deleted.Contains(resolver.StepId)) deleted.Add(resolver.StepId);
        }

        int applied = deleted.Count;
        foreach (int id in deleted) RemoveStep(id);
        if (deleted.Count > 0) _ordering.Rebuild(_steps.Select(step => step.Id), _orderings);

        var dropped = new List<int>();
        var created = new List<int>();
        var stepIdByGhostIndex = new Dictionary<int, int>();
        for (int i = 0; i < chosen.Length; i++)
        {
            Resolver resolver = chosen[i];
            if (resolver.Kind != ResolverKind.AdoptGhost && resolver.Kind != ResolverKind.LinkExisting) continue;

            Flaw flaw = flaws.Flaws[i];
            if (deleted.Contains(flaw.StepId) || (resolver.Kind == ResolverKind.LinkExisting && deleted.Contains(resolver.StepId)))
            {
                dropped.Add(i);
                continue;
            }

            int producerId = resolver.StepId;
            if (resolver.Kind == ResolverKind.AdoptGhost && !stepIdByGhostIndex.TryGetValue(resolver.GhostIndex, out producerId))
            {
                GhostStep ghost = flaws.Ghosts[resolver.GhostIndex];
                producerId = CreateStep(ghost.Schema, ghost.CopyBinding()).Id;
                stepIdByGhostIndex.Add(resolver.GhostIndex, producerId);
                created.Add(producerId);
            }

            if (TryAddLink(producerId, flaw.StepId, flaw.Fact!) is null)
            {
                dropped.Add(i);
                continue;
            }

            applied++;
        }

        for (int i = 0; i < chosen.Length; i++)
        {
            if (chosen[i].Kind != ResolverKind.Bind) continue;

            Flaw flaw = flaws.Flaws[i];
            if (deleted.Contains(flaw.StepId))
            {
                dropped.Add(i);
                continue;
            }

            // Bind candidates were filtered with the other parameters unbound, so binds applied
            // earlier in this round can rule this one out.
            PlanStep step = GetStep(flaw.StepId);
            Constant?[] trial = step.Binding.ToArray();
            trial[flaw.ParameterIndex] = chosen[i].Object!;
            if (!StaticFilter.Passes(Problem, step.Schema!, trial))
            {
                dropped.Add(i);
                continue;
            }

            step.Bind(flaw.ParameterIndex, chosen[i].Object!, Problem);
            applied++;
        }

        for (int i = 0; i < chosen.Length; i++)
        {
            if (chosen[i].Kind != ResolverKind.Promote && chosen[i].Kind != ResolverKind.Demote) continue;

            Flaw flaw = flaws.Flaws[i];
            if (deleted.Contains(flaw.StepId) || !_linkById.TryGetValue(flaw.LinkId, out CausalLink? link))
            {
                dropped.Add(i);
                continue;
            }

            bool added = chosen[i].Kind == ResolverKind.Promote
                ? AddOrdering(link.ConsumerId, flaw.StepId, link.Id)
                : AddOrdering(flaw.StepId, link.ProducerId, link.Id);
            if (!added)
            {
                dropped.Add(i);
                continue;
            }

            applied++;
        }

        Round++;
        _version++;
        foreach (Tombstone tombstone in _tombstones) tombstone.Age++;
        _tombstones.RemoveAll(tombstone => tombstone.Age > Options.TombstoneLifespan);
        dropped.Sort();
        return new EditResult(applied, dropped, created, deleted);
    }
}
