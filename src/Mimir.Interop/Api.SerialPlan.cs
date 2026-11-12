using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Learning;
using Mimir.PlanSpace;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

// Serial-plan values cross the boundary as flat int32 arrays. Every array is read with a
// get_*_value_count call followed by a copy_* call into a caller-owned buffer. Record layouts:
//
// * steps:          per step      [id, schemaIndex, isFullyBound, purposeStepId, k, objectIndex | -1 × k]
// * errors:         per error     [kind, stepId, parameterIndex, repairCount,
//                                  per repair: kind, candidateIndex, parameterIndex, objectIndex | -1]
// * candidates:     per candidate [schemaIndex, k, objectIndex | -1 × k]
// * supports:       per support   [producerStepId, consumerStepId]
// * unused steps:   [stepId…]
// * edit result:    [appliedCount, droppedCount, createdCount, deletedCount, dropped…, created…, deleted…]
// * oracle targets: [matchedStepCount, errorCount, deleteCount, repairIndex…, stepIdsToDelete…]
// * layout:         [initialNode, goalNode, stepCount, stepNode…, per step: k, parameterNode × k,
//                    candidateCount, candidateNode…, per candidate: k, parameterNode × k,
//                    errorCount, errorNode…, per error: repairCount, repairNode…]
//
// Schema indices refer to Domain.Actions and object indices to Problem.AllObjects.
public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_empty")]
    public static int SerialPlanEmpty(int problemHandle)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;

        return CreateHandle(() => SerialPlan.Empty(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_empty_with_goal")]
    public static int SerialPlanEmptyWithGoal(int problemHandle, IntPtr goalHandlesPtr, int goalCount)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(goalHandlesPtr, goalCount, "goal");
        if (goal is null)
            return 0;

        return CreateHandle(() => SerialPlan.Empty(problem, goal));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_goal_count")]
    public static int SerialPlanGetGoalCount(int planHandle)
        => ReadValue(planHandle, -1, (SerialPlan plan) => plan.Goal.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_goal_literal")]
    public static int SerialPlanGetGoalLiteral(int planHandle, int goalIndex)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(() => plan.Goal[goalIndex]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_with_goal")]
    public static int SerialPlanWithGoal(int planHandle, IntPtr goalHandlesPtr, int goalCount)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(goalHandlesPtr, goalCount, "goal");
        if (goal is null)
            return 0;

        return CreateHandle(() => plan.WithGoal(goal));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_prune_to_achieved_goal")]
    public static int SerialPlanPruneToAchievedGoal(int planHandle)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.PruneToAchievedGoal);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_clone")]
    public static int SerialPlanClone(int planHandle)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.Clone);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_round")]
    public static int SerialPlanGetRound(int planHandle)
        => ReadValue(planHandle, -1, (SerialPlan plan) => plan.Round);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_is_solution")]
    public static int SerialPlanIsSolution(int planHandle)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return InvalidBoolean;

        return EvaluateRelationship(plan.IsSolution);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_step_value_count")]
    public static int SerialPlanGetStepValueCount(int planHandle)
        => ReadValue(planHandle, -1, (SerialPlan plan) => StepValues(plan).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_copy_steps")]
    public static int SerialPlanCopySteps(int planHandle, IntPtr destination, int capacity)
        => ReadValue(planHandle, -1, (SerialPlan plan) => CopyInt32Values(StepValues(plan), destination, capacity, "steps"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_step_purpose")]
    public static int SerialPlanGetStepPurpose(int planHandle, int stepIndex)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(() => plan.Steps[stepIndex].Purpose);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_diagnose")]
    public static int SerialPlanDiagnose(int planHandle)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.Diagnose);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_apply")]
    public static int SerialPlanApply(
        int planHandle,
        int diagnosisHandle,
        IntPtr repairIndicesPtr,
        int repairCount,
        IntPtr stepIdsPtr,
        int stepIdCount)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;
        SerialDiagnosis? diagnosis = GetLearningHandle<SerialDiagnosis>(diagnosisHandle);
        if (diagnosis is null)
            return 0;
        int[]? repairIndices = ReadLearningInt32Array(repairIndicesPtr, repairCount, "chosenRepairIndex");
        if (repairIndices is null)
            return 0;
        int[]? stepIds = ReadLearningInt32Array(stepIdsPtr, stepIdCount, "stepIdsToDelete");
        if (stepIds is null)
            return 0;

        return CreateHandle(() => plan.Apply(diagnosis, repairIndices, stepIds));
    }

    // The returned list is read with mimir_serial_action_list_get_count and _get_action.
    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_to_actions")]
    public static int SerialPlanToActions(int planHandle)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.ToActions);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_action_list_get_count")]
    public static int SerialActionListGetCount(int listHandle)
        => ReadValue(listHandle, -1, (IReadOnlyList<GroundAction> actions) => actions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_action_list_get_action")]
    public static int SerialActionListGetAction(int listHandle, int index)
    {
        IReadOnlyList<GroundAction>? actions = GetLearningHandle<IReadOnlyList<GroundAction>>(listHandle);
        if (actions is null)
            return 0;

        return CreateHandle(() => actions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_error_value_count")]
    public static int SerialDiagnosisGetErrorValueCount(int diagnosisHandle)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => ErrorValues(diagnosis).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_copy_errors")]
    public static int SerialDiagnosisCopyErrors(int diagnosisHandle, IntPtr destination, int capacity)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => CopyInt32Values(ErrorValues(diagnosis), destination, capacity, "errors"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_error_literal")]
    public static int SerialDiagnosisGetErrorLiteral(int diagnosisHandle, int errorIndex)
    {
        SerialDiagnosis? diagnosis = GetLearningHandle<SerialDiagnosis>(diagnosisHandle);
        if (diagnosis is null)
            return 0;

        return CreateHandle(() => diagnosis.Errors[errorIndex].Literal
            ?? throw new InvalidOperationException($"Error {errorIndex} has no literal."));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_candidate_value_count")]
    public static int SerialDiagnosisGetCandidateValueCount(int diagnosisHandle)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => CandidateValues(diagnosis).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_copy_candidates")]
    public static int SerialDiagnosisCopyCandidates(int diagnosisHandle, IntPtr destination, int capacity)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => CopyInt32Values(CandidateValues(diagnosis), destination, capacity, "candidates"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_support_value_count")]
    public static int SerialDiagnosisGetSupportValueCount(int diagnosisHandle)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => SupportValues(diagnosis).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_copy_supports")]
    public static int SerialDiagnosisCopySupports(int diagnosisHandle, IntPtr destination, int capacity)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => CopyInt32Values(SupportValues(diagnosis), destination, capacity, "supports"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_support_literal")]
    public static int SerialDiagnosisGetSupportLiteral(int diagnosisHandle, int supportIndex)
    {
        SerialDiagnosis? diagnosis = GetLearningHandle<SerialDiagnosis>(diagnosisHandle);
        if (diagnosis is null)
            return 0;

        return CreateHandle(() => diagnosis.Supports[supportIndex].Literal);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_unused_step_count")]
    public static int SerialDiagnosisGetUnusedStepCount(int diagnosisHandle)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => diagnosis.UnusedStepIds.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_copy_unused_steps")]
    public static int SerialDiagnosisCopyUnusedSteps(int diagnosisHandle, IntPtr destination, int capacity)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => CopyInt32Values(diagnosis.UnusedStepIds, destination, capacity, "unused steps"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_edit_result_get_value_count")]
    public static int SerialEditResultGetValueCount(int resultHandle)
        => ReadValue(resultHandle, -1, (SerialEditResult result) => EditResultValues(result).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_edit_result_copy_values")]
    public static int SerialEditResultCopyValues(int resultHandle, IntPtr destination, int capacity)
        => ReadValue(resultHandle, -1, (SerialEditResult result) => CopyInt32Values(EditResultValues(result), destination, capacity, "edit result"));

    // Returns 0 without recording an error when the search finds no plan within its budget.
    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_search")]
    public static int SerialSearchSolve(int problemHandle, int maxExpansions, double timeoutSeconds)
    {
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;

        try
        {
            return ObjectRegistry.Store(SerialSearch.Solve(problem, maxExpansions, TimeSpan.FromSeconds(timeoutSeconds)));
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return 0;
        }
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_oracle_targets")]
    public static int SerialOracleComputeTargets(int currentHandle, int diagnosisHandle, int referenceHandle)
    {
        SerialPlan? current = GetLearningHandle<SerialPlan>(currentHandle);
        if (current is null)
            return 0;
        SerialDiagnosis? diagnosis = GetLearningHandle<SerialDiagnosis>(diagnosisHandle);
        if (diagnosis is null)
            return 0;
        SerialPlan? reference = GetLearningHandle<SerialPlan>(referenceHandle);
        if (reference is null)
            return 0;

        return CreateHandle(() => SerialOracle.Targets(current, diagnosis, reference));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_oracle_targets_get_value_count")]
    public static int SerialOracleTargetsGetValueCount(int targetsHandle)
        => ReadValue(targetsHandle, -1, (SerialOracleTargets targets) => OracleTargetValues(targets).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_oracle_targets_copy_values")]
    public static int SerialOracleTargetsCopyValues(int targetsHandle, IntPtr destination, int capacity)
        => ReadValue(targetsHandle, -1, (SerialOracleTargets targets) => CopyInt32Values(OracleTargetValues(targets), destination, capacity, "oracle targets"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_encode_plan")]
    public static int SerialEncodePlan(int contextHandle, int planHandle, int diagnosisHandle, IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;
        SerialDiagnosis? diagnosis = GetLearningHandle<SerialDiagnosis>(diagnosisHandle);
        if (diagnosis is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return CreateHandle(() => Encoding.EncodeSerialPlan(context, plan, diagnosis, suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_layout_get_value_count")]
    public static int SerialLayoutGetValueCount(int layoutHandle)
        => ReadValue(layoutHandle, -1, (SerialEncodingLayout layout) => LayoutValues(layout).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_layout_copy_values")]
    public static int SerialLayoutCopyValues(int layoutHandle, IntPtr destination, int capacity)
        => ReadValue(layoutHandle, -1, (SerialEncodingLayout layout) => CopyInt32Values(LayoutValues(layout), destination, capacity, "layout"));

    // One "name arity" entry per line.
    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_relations")]
    public static IntPtr SerialRelations(int domainHandle, IntPtr suffixPtr)
    {
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return IntPtr.Zero;

        return ReadValue(domainHandle, IntPtr.Zero, (Domain domain) =>
        {
            IReadOnlyList<(string Name, int Arity)> relations = Encoding.SerialPlanRelations(domain, suffix);
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

    private static List<int> StepValues(SerialPlan plan)
    {
        Dictionary<Constant, int> objectIndices = ObjectIndices(plan.Problem);
        var values = new List<int>();
        foreach (SerialStep step in plan.Steps)
        {
            values.Add(step.Id);
            values.Add(SchemaIndex(plan.Problem.Domain, step.Schema));
            values.Add(step.IsFullyBound ? 1 : 0);
            values.Add(step.PurposeStepId);
            AppendBinding(values, step.Binding, objectIndices);
        }
        return values;
    }

    private static List<int> ErrorValues(SerialDiagnosis diagnosis)
    {
        Dictionary<Constant, int> objectIndices = ObjectIndices(diagnosis.Plan.Problem);
        var values = new List<int>();
        foreach (SerialError error in diagnosis.Errors)
        {
            values.Add((int)error.Kind);
            values.Add(error.StepId);
            values.Add(error.ParameterIndex);
            values.Add(error.Repairs.Count);
            foreach (SerialRepair repair in error.Repairs)
            {
                values.Add((int)repair.Kind);
                values.Add(repair.CandidateIndex);
                values.Add(repair.ParameterIndex);
                values.Add(repair.Object is null ? -1 : objectIndices[repair.Object]);
            }
        }
        return values;
    }

    private static List<int> CandidateValues(SerialDiagnosis diagnosis)
    {
        Problem problem = diagnosis.Plan.Problem;
        Dictionary<Constant, int> objectIndices = ObjectIndices(problem);
        var values = new List<int>();
        foreach (GhostStep candidate in diagnosis.Candidates)
        {
            values.Add(SchemaIndex(problem.Domain, candidate.Schema));
            AppendBinding(values, candidate.Binding, objectIndices);
        }
        return values;
    }

    private static List<int> SupportValues(SerialDiagnosis diagnosis)
    {
        var values = new List<int>(diagnosis.Supports.Count * 2);
        foreach (SerialSupport support in diagnosis.Supports)
        {
            values.Add(support.ProducerStepId);
            values.Add(support.ConsumerStepId);
        }
        return values;
    }

    private static List<int> EditResultValues(SerialEditResult result)
    {
        var values = new List<int>
        {
            result.AppliedCount,
            result.DroppedErrorIndices.Count,
            result.CreatedStepIds.Count,
            result.DeletedStepIds.Count,
        };
        values.AddRange(result.DroppedErrorIndices);
        values.AddRange(result.CreatedStepIds);
        values.AddRange(result.DeletedStepIds);
        return values;
    }

    private static List<int> OracleTargetValues(SerialOracleTargets targets)
    {
        var values = new List<int>
        {
            targets.MatchedStepCount,
            targets.RepairIndexPerError.Count,
            targets.StepIdsToDelete.Count,
        };
        values.AddRange(targets.RepairIndexPerError);
        values.AddRange(targets.StepIdsToDelete);
        return values;
    }

    private static List<int> LayoutValues(SerialEncodingLayout layout)
    {
        var values = new List<int> { layout.InitialNodeId, layout.GoalNodeId };
        AppendCounted(values, layout.StepNodeIds);
        foreach (int[] parameterNodes in layout.ParameterNodeIds) AppendCounted(values, parameterNodes);
        AppendCounted(values, layout.CandidateNodeIds);
        foreach (int[] parameterNodes in layout.CandidateParameterNodeIds) AppendCounted(values, parameterNodes);
        AppendCounted(values, layout.ErrorNodeIds);
        foreach (int[] repairNodes in layout.RepairNodeIds) AppendCounted(values, repairNodes);
        return values;
    }

    private static void AppendCounted(List<int> values, int[] ids)
    {
        values.Add(ids.Length);
        values.AddRange(ids);
    }
}
