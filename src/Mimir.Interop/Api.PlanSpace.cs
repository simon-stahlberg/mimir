using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Learning;
using Mimir.PlanSpace;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

// Plan-space values cross the boundary as flat int32 arrays. Every array is read with a
// get_*_value_count call followed by a copy_* call into a caller-owned buffer. Record layouts:
//
// * steps:          per step   [id, kind, schemaIndex | -1, isFullyBound, k, objectIndex | -1 × k]
// * links:          per link   [id, producerId, consumerId]
// * tombstones:     per entry  [schemaIndex, age, k, objectIndex × k]
// * ordering:       per pair   [before, after]
// * flaws:          per flaw   [kind, stepId, linkId, parameterIndex, resolverCount,
//                               per resolver: kind, stepId, ghostIndex, objectIndex | -1]
// * ghosts:         per ghost  [schemaIndex, k, objectIndex | -1 × k]
// * edit result:    [appliedCount, droppedCount, createdCount, deletedCount, dropped…, created…, deleted…]
// * oracle targets: [matchedStepCount, flawCount, deleteCount, resolverIndex…, stepIdsToDelete…]
// * layout:         [stepCount, stepNode…, per step: k, parameterNode × k,
//                    linkCount, linkNode…, ghostCount, ghostNode…, per ghost: k, parameterNode × k,
//                    flawCount, flawNode…, per flaw: resolverCount, resolverNode…,
//                    tombstoneCount, tombstoneNode…]
//
// Schema indices refer to Domain.Actions and object indices to Problem.AllObjects.
public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_empty")]
    public static int PlanSpacePlanEmpty(int problemHandle, int tombstoneCapacity, int tombstoneLifespan)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;

        return CreateHandle(() => PartialOrderPlan.Empty(
            problem,
            new PlanSpaceOptions(tombstoneCapacity, tombstoneLifespan)));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_from_sequential_plan")]
    public static int PlanSpacePlanFromSequentialPlan(
        int problemHandle,
        IntPtr actionHandlesPtr,
        int actionCount,
        int tombstoneCapacity,
        int tombstoneLifespan)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;
        List<GroundAction>? actions = ReadLearningHandleArray<GroundAction>(actionHandlesPtr, actionCount, "actions");
        if (actions is null)
            return 0;

        return CreateHandle(() => PartialOrderPlan.FromSequentialPlan(
            problem,
            actions,
            new PlanSpaceOptions(tombstoneCapacity, tombstoneLifespan)));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_empty_with_goal")]
    public static int PlanSpacePlanEmptyWithGoal(
        int problemHandle,
        IntPtr goalHandlesPtr,
        int goalCount,
        int tombstoneCapacity,
        int tombstoneLifespan)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(goalHandlesPtr, goalCount, "goal");
        if (goal is null)
            return 0;

        return CreateHandle(() => PartialOrderPlan.Empty(
            problem,
            goal,
            new PlanSpaceOptions(tombstoneCapacity, tombstoneLifespan)));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_from_sequential_plan_with_goal")]
    public static int PlanSpacePlanFromSequentialPlanWithGoal(
        int problemHandle,
        IntPtr actionHandlesPtr,
        int actionCount,
        IntPtr goalHandlesPtr,
        int goalCount,
        int tombstoneCapacity,
        int tombstoneLifespan)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;
        List<GroundAction>? actions = ReadLearningHandleArray<GroundAction>(actionHandlesPtr, actionCount, "actions");
        if (actions is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(goalHandlesPtr, goalCount, "goal");
        if (goal is null)
            return 0;

        return CreateHandle(() => PartialOrderPlan.FromSequentialPlan(
            problem,
            actions,
            goal,
            new PlanSpaceOptions(tombstoneCapacity, tombstoneLifespan)));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_goal_count")]
    public static int PlanSpacePlanGetGoalCount(int planHandle)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => plan.Goal.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_goal_atom")]
    public static int PlanSpacePlanGetGoalAtom(int planHandle, int goalIndex)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(() => plan.Goal[goalIndex].Value);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_with_goal")]
    public static int PlanSpacePlanWithGoal(int planHandle, IntPtr goalHandlesPtr, int goalCount)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(goalHandlesPtr, goalCount, "goal");
        if (goal is null)
            return 0;

        return CreateHandle(() => plan.WithGoal(goal));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_prune_to_achieved_goal")]
    public static int PlanSpacePlanPruneToAchievedGoal(int planHandle)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.PruneToAchievedGoal);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_clone")]
    public static int PlanSpacePlanClone(int planHandle)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.Clone);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_round")]
    public static int PlanSpacePlanGetRound(int planHandle)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => plan.Round);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_is_before")]
    public static int PlanSpacePlanIsBefore(int planHandle, int stepA, int stepB)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return InvalidBoolean;

        return EvaluateRelationship(() => plan.IsBefore(stepA, stepB));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_is_solution")]
    public static int PlanSpacePlanIsSolution(int planHandle)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return InvalidBoolean;

        return EvaluateRelationship(plan.IsSolution);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_step_value_count")]
    public static int PlanSpacePlanGetStepValueCount(int planHandle)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => StepValues(plan).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_copy_steps")]
    public static int PlanSpacePlanCopySteps(int planHandle, IntPtr destination, int capacity)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => CopyInt32Values(StepValues(plan), destination, capacity, "steps"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_link_value_count")]
    public static int PlanSpacePlanGetLinkValueCount(int planHandle)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => LinkValues(plan).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_copy_links")]
    public static int PlanSpacePlanCopyLinks(int planHandle, IntPtr destination, int capacity)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => CopyInt32Values(LinkValues(plan), destination, capacity, "links"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_link_atom")]
    public static int PlanSpacePlanGetLinkAtom(int planHandle, int linkIndex)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(() => plan.Links[linkIndex].Fact);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_tombstone_value_count")]
    public static int PlanSpacePlanGetTombstoneValueCount(int planHandle)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => TombstoneValues(plan).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_copy_tombstones")]
    public static int PlanSpacePlanCopyTombstones(int planHandle, IntPtr destination, int capacity)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => CopyInt32Values(TombstoneValues(plan), destination, capacity, "tombstones"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_get_ordering_value_count")]
    public static int PlanSpacePlanGetOrderingValueCount(int planHandle)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => OrderingValues(plan).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_copy_ordering")]
    public static int PlanSpacePlanCopyOrdering(int planHandle, IntPtr destination, int capacity)
        => ReadValue(planHandle, -1, (PartialOrderPlan plan) => CopyInt32Values(OrderingValues(plan), destination, capacity, "ordering"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_compute_flaws")]
    public static int PlanSpacePlanComputeFlaws(int planHandle)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.ComputeFlaws);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_apply")]
    public static int PlanSpacePlanApply(
        int planHandle,
        int flawsHandle,
        IntPtr resolverIndicesPtr,
        int resolverCount,
        IntPtr stepIdsPtr,
        int stepIdCount)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;
        FlawSet? flaws = GetLearningHandle<FlawSet>(flawsHandle);
        if (flaws is null)
            return 0;
        int[]? resolverIndices = ReadLearningInt32Array(resolverIndicesPtr, resolverCount, "chosenResolverIndex");
        if (resolverIndices is null)
            return 0;
        int[]? stepIds = ReadLearningInt32Array(stepIdsPtr, stepIdCount, "stepIdsToDelete");
        if (stepIds is null)
            return 0;

        return CreateHandle(() => plan.Apply(flaws, resolverIndices, stepIds));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_plan_linearize")]
    public static int PlanSpacePlanLinearize(int planHandle)
    {
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.Linearize);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_action_list_get_count")]
    public static int PlanSpaceActionListGetCount(int listHandle)
        => ReadValue(listHandle, -1, (IReadOnlyList<GroundAction> actions) => actions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_action_list_get_action")]
    public static int PlanSpaceActionListGetAction(int listHandle, int index)
    {
        IReadOnlyList<GroundAction>? actions = GetLearningHandle<IReadOnlyList<GroundAction>>(listHandle);
        if (actions is null)
            return 0;

        return CreateHandle(() => actions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_flaws_get_value_count")]
    public static int PlanSpaceFlawsGetValueCount(int flawsHandle)
        => ReadValue(flawsHandle, -1, (FlawSet flaws) => FlawValues(flaws).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_flaws_copy_flaws")]
    public static int PlanSpaceFlawsCopyFlaws(int flawsHandle, IntPtr destination, int capacity)
        => ReadValue(flawsHandle, -1, (FlawSet flaws) => CopyInt32Values(FlawValues(flaws), destination, capacity, "flaws"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_flaws_get_flaw_atom")]
    public static int PlanSpaceFlawsGetFlawAtom(int flawsHandle, int flawIndex)
    {
        FlawSet? flaws = GetLearningHandle<FlawSet>(flawsHandle);
        if (flaws is null)
            return 0;

        return CreateHandle(() => flaws.Flaws[flawIndex].Fact
            ?? throw new InvalidOperationException($"Flaw {flawIndex} has no atom."));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_flaws_get_ghost_value_count")]
    public static int PlanSpaceFlawsGetGhostValueCount(int flawsHandle)
        => ReadValue(flawsHandle, -1, (FlawSet flaws) => GhostValues(flaws).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_flaws_copy_ghosts")]
    public static int PlanSpaceFlawsCopyGhosts(int flawsHandle, IntPtr destination, int capacity)
        => ReadValue(flawsHandle, -1, (FlawSet flaws) => CopyInt32Values(GhostValues(flaws), destination, capacity, "ghosts"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_edit_result_get_value_count")]
    public static int PlanSpaceEditResultGetValueCount(int resultHandle)
        => ReadValue(resultHandle, -1, (EditResult result) => EditResultValues(result).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_edit_result_copy_values")]
    public static int PlanSpaceEditResultCopyValues(int resultHandle, IntPtr destination, int capacity)
        => ReadValue(resultHandle, -1, (EditResult result) => CopyInt32Values(EditResultValues(result), destination, capacity, "edit result"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_oracle_targets")]
    public static int PlanSpaceOracleTargets(int currentHandle, int flawsHandle, int referenceHandle)
    {
        PartialOrderPlan? current = GetLearningHandle<PartialOrderPlan>(currentHandle);
        if (current is null)
            return 0;
        FlawSet? flaws = GetLearningHandle<FlawSet>(flawsHandle);
        if (flaws is null)
            return 0;
        PartialOrderPlan? reference = GetLearningHandle<PartialOrderPlan>(referenceHandle);
        if (reference is null)
            return 0;

        return CreateHandle(() => ReferenceOracle.Targets(current, flaws, reference));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_oracle_targets_get_value_count")]
    public static int PlanSpaceOracleTargetsGetValueCount(int targetsHandle)
        => ReadValue(targetsHandle, -1, (OracleTargets targets) => OracleTargetValues(targets).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_oracle_targets_copy_values")]
    public static int PlanSpaceOracleTargetsCopyValues(int targetsHandle, IntPtr destination, int capacity)
        => ReadValue(targetsHandle, -1, (OracleTargets targets) => CopyInt32Values(OracleTargetValues(targets), destination, capacity, "oracle targets"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_encode_partial_plan")]
    public static int PlanSpaceEncodePartialPlan(int contextHandle, int planHandle, int flawsHandle, IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        PartialOrderPlan? plan = GetLearningHandle<PartialOrderPlan>(planHandle);
        if (plan is null)
            return 0;
        FlawSet? flaws = GetLearningHandle<FlawSet>(flawsHandle);
        if (flaws is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return CreateHandle(() => Encoding.EncodePartialPlan(context, plan, flaws, suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_layout_get_value_count")]
    public static int PlanSpaceLayoutGetValueCount(int layoutHandle)
        => ReadValue(layoutHandle, -1, (PlanEncodingLayout layout) => LayoutValues(layout).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_layout_copy_values")]
    public static int PlanSpaceLayoutCopyValues(int layoutHandle, IntPtr destination, int capacity)
        => ReadValue(layoutHandle, -1, (PlanEncodingLayout layout) => CopyInt32Values(LayoutValues(layout), destination, capacity, "layout"));

    // One "name arity" entry per line.
    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_space_relations")]
    public static IntPtr PlanSpaceRelations(int domainHandle, IntPtr suffixPtr)
    {
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return IntPtr.Zero;

        return ReadValue(domainHandle, IntPtr.Zero, (Domain domain) =>
        {
            IReadOnlyList<(string Name, int Arity)> relations = Encoding.PartialPlanRelations(domain, suffix);
            return AllocUtf8(string.Join("\n", relations.Select(relation => $"{relation.Name} {relation.Arity}")));
        });
    }

    private static Dictionary<Constant, int> ObjectIndices(Problem problem)
    {
        var indices = new Dictionary<Constant, int>(ReferenceEqualityComparer.Instance);
        for (int index = 0; index < problem.AllObjects.Count; index++)
            indices.Add(problem.AllObjects[index], index);
        return indices;
    }

    private static int SchemaIndex(Domain domain, ActionSchema schema)
    {
        for (int index = 0; index < domain.Actions.Count; index++)
        {
            if (ReferenceEquals(domain.Actions[index], schema)) return index;
        }

        throw new InvalidOperationException($"Action {schema.Name} is not part of the domain.");
    }

    private static void AppendBinding(List<int> values, IReadOnlyList<Constant?> binding, Dictionary<Constant, int> objectIndices)
    {
        values.Add(binding.Count);
        foreach (Constant? constant in binding)
            values.Add(constant is null ? -1 : objectIndices[constant]);
    }

    private static List<int> StepValues(PartialOrderPlan plan)
    {
        Dictionary<Constant, int> objectIndices = ObjectIndices(plan.Problem);
        var values = new List<int>();
        foreach (PlanStep step in plan.Steps)
        {
            values.Add(step.Id);
            values.Add((int)step.Kind);
            values.Add(step.Schema is null ? -1 : SchemaIndex(plan.Problem.Domain, step.Schema));
            values.Add(step.IsFullyBound ? 1 : 0);
            AppendBinding(values, step.Binding, objectIndices);
        }
        return values;
    }

    private static List<int> LinkValues(PartialOrderPlan plan)
    {
        var values = new List<int>(plan.Links.Count * 3);
        foreach (CausalLink link in plan.Links)
        {
            values.Add(link.Id);
            values.Add(link.ProducerId);
            values.Add(link.ConsumerId);
        }
        return values;
    }

    private static List<int> TombstoneValues(PartialOrderPlan plan)
    {
        Dictionary<Constant, int> objectIndices = ObjectIndices(plan.Problem);
        var values = new List<int>();
        foreach (Tombstone tombstone in plan.Tombstones)
        {
            values.Add(SchemaIndex(plan.Problem.Domain, tombstone.Schema));
            values.Add(tombstone.Age);
            AppendBinding(values, tombstone.Binding, objectIndices);
        }
        return values;
    }

    private static List<int> OrderingValues(PartialOrderPlan plan)
    {
        var values = new List<int>();
        foreach ((int before, int after) in plan.OrderingClosure)
        {
            values.Add(before);
            values.Add(after);
        }
        return values;
    }

    private static List<int> FlawValues(FlawSet flaws)
    {
        Dictionary<Constant, int> objectIndices = ObjectIndices(flaws.Plan.Problem);
        var values = new List<int>();
        foreach (Flaw flaw in flaws.Flaws)
        {
            values.Add((int)flaw.Kind);
            values.Add(flaw.StepId);
            values.Add(flaw.LinkId);
            values.Add(flaw.ParameterIndex);
            values.Add(flaw.Resolvers.Count);
            foreach (Resolver resolver in flaw.Resolvers)
            {
                values.Add((int)resolver.Kind);
                values.Add(resolver.StepId);
                values.Add(resolver.GhostIndex);
                values.Add(resolver.Object is null ? -1 : objectIndices[resolver.Object]);
            }
        }
        return values;
    }

    private static List<int> GhostValues(FlawSet flaws)
    {
        Problem problem = flaws.Plan.Problem;
        Dictionary<Constant, int> objectIndices = ObjectIndices(problem);
        var values = new List<int>();
        foreach (GhostStep ghost in flaws.Ghosts)
        {
            values.Add(SchemaIndex(problem.Domain, ghost.Schema));
            AppendBinding(values, ghost.Binding, objectIndices);
        }
        return values;
    }

    private static List<int> EditResultValues(EditResult result)
    {
        var values = new List<int>
        {
            result.AppliedCount,
            result.DroppedFlawIndices.Count,
            result.CreatedStepIds.Count,
            result.DeletedStepIds.Count,
        };
        values.AddRange(result.DroppedFlawIndices);
        values.AddRange(result.CreatedStepIds);
        values.AddRange(result.DeletedStepIds);
        return values;
    }

    private static List<int> OracleTargetValues(OracleTargets targets)
    {
        var values = new List<int>
        {
            targets.MatchedStepCount,
            targets.ResolverIndexPerFlaw.Count,
            targets.StepIdsToDelete.Count,
        };
        values.AddRange(targets.ResolverIndexPerFlaw);
        values.AddRange(targets.StepIdsToDelete);
        return values;
    }

    private static List<int> LayoutValues(PlanEncodingLayout layout)
    {
        var values = new List<int>();
        AppendCounted(values, layout.StepNodeIds);
        foreach (int[] parameterNodes in layout.ParameterNodeIds) AppendCounted(values, parameterNodes);
        AppendCounted(values, layout.LinkNodeIds);
        AppendCounted(values, layout.GhostNodeIds);
        foreach (int[] parameterNodes in layout.GhostParameterNodeIds) AppendCounted(values, parameterNodes);
        AppendCounted(values, layout.FlawNodeIds);
        foreach (int[] resolverNodes in layout.ResolverNodeIds) AppendCounted(values, resolverNodes);
        AppendCounted(values, layout.TombstoneNodeIds);
        return values;
    }

    private static void AppendCounted(List<int> values, int[] ids)
    {
        values.Add(ids.Length);
        values.AddRange(ids);
    }
}
