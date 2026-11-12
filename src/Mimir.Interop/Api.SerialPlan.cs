using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.PlanSpace;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

// Serial-plan values cross the boundary as flat int32 arrays. Every array is read with a
// get_*_value_count call followed by a copy_* call into a caller-owned buffer. Record layouts:
//
// * steps:           per step      [id, schemaIndex, k, objectIndex × k]
// * errors:          per error     [kind, stepId, culpritStepId, candidateCount, candidateIndex…]
// * edits:           [insertCount, per insert: point, schemaIndex, k, objectIndex × k, deleteCount, stepId…,
//                     moveCount, per move: stepId, point, rebindCount, per rebind: stepId, k, objectIndex × k]
// * candidates:      per candidate [schemaIndex, k, objectIndex | -1 × k]
// * supports:        per support   [producerStepId, consumerStepId]
// * unused steps:    [stepId…]
// * options:         [objectIndex…]
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

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_parameter_option_count")]
    public static int SerialPlanGetParameterOptionCount(int planHandle, int schemaIndex, int parameterIndex)
        => ReadValue(planHandle, -1, (SerialPlan plan) => ParameterOptionValues(plan, schemaIndex, parameterIndex).Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_copy_parameter_options")]
    public static int SerialPlanCopyParameterOptions(int planHandle, int schemaIndex, int parameterIndex, IntPtr destination, int capacity)
        => ReadValue(planHandle, -1, (SerialPlan plan) => CopyInt32Values(ParameterOptionValues(plan, schemaIndex, parameterIndex), destination, capacity, "options"));

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_diagnose")]
    public static int SerialPlanDiagnose(int planHandle)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(plan.Diagnose);
    }

    // Returns 1, or 0 with a recorded error when the edits do not fit the plan.
    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_apply")]
    public static int SerialPlanApply(int planHandle, IntPtr editsPtr, int editCount)
    {
        int[]? edits = ReadLearningInt32Array(editsPtr, editCount, "edits");
        if (edits is null)
            return 0;

        return ReadValue(planHandle, 0, (SerialPlan plan) =>
        {
            plan.Apply(ReadEdits(plan.Problem, edits));
            return 1;
        });
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_step_precondition_count")]
    public static int SerialPlanGetStepPreconditionCount(int planHandle, int stepIndex)
        => ReadValue(planHandle, -1, (SerialPlan plan) => plan.Steps[stepIndex].Preconditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_step_precondition")]
    public static int SerialPlanGetStepPrecondition(int planHandle, int stepIndex, int index)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(() => plan.Steps[stepIndex].Preconditions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_step_effect_count")]
    public static int SerialPlanGetStepEffectCount(int planHandle, int stepIndex)
        => ReadValue(planHandle, -1, (SerialPlan plan) => plan.Steps[stepIndex].Effects.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_plan_get_step_effect")]
    public static int SerialPlanGetStepEffect(int planHandle, int stepIndex, int index)
    {
        SerialPlan? plan = GetLearningHandle<SerialPlan>(planHandle);
        if (plan is null)
            return 0;

        return CreateHandle(() => plan.Steps[stepIndex].Effects[index]);
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

        return CreateHandle(() => diagnosis.Errors[errorIndex].Literal);
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

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_final_fact_count")]
    public static int SerialDiagnosisGetFinalFactCount(int diagnosisHandle)
        => ReadValue(diagnosisHandle, -1, (SerialDiagnosis diagnosis) => diagnosis.FinalTrueFacts.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_serial_diagnosis_get_final_fact")]
    public static int SerialDiagnosisGetFinalFact(int diagnosisHandle, int factIndex)
    {
        SerialDiagnosis? diagnosis = GetLearningHandle<SerialDiagnosis>(diagnosisHandle);
        if (diagnosis is null)
            return 0;

        return CreateHandle(() => diagnosis.FinalTrueFacts[factIndex]);
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
            AppendBinding(values, step.Binding, objectIndices);
        }
        return values;
    }

    private static List<int> ParameterOptionValues(SerialPlan plan, int schemaIndex, int parameterIndex)
    {
        Dictionary<Constant, int> objectIndices = ObjectIndices(plan.Problem);
        return plan.ParameterOptions(plan.Problem.Domain.Actions[schemaIndex], parameterIndex).Select(value => objectIndices[value]).ToList();
    }

    private static List<int> ErrorValues(SerialDiagnosis diagnosis)
    {
        var values = new List<int>();
        foreach (SerialError error in diagnosis.Errors)
        {
            values.Add((int)error.Kind);
            values.Add(error.StepId);
            values.Add(error.CulpritStepId);
            values.Add(error.CandidateIndices.Count);
            values.AddRange(error.CandidateIndices);
        }
        return values;
    }

    private static List<int> CandidateValues(SerialDiagnosis diagnosis)
    {
        Problem problem = diagnosis.Problem;
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

    private static SerialEdits ReadEdits(Problem problem, int[] values)
    {
        int position = 0;
        int Next()
        {
            if (position >= values.Length) throw new ArgumentException("The edits end prematurely.");
            return values[position++];
        }

        Constant[] Binding() => Enumerable.Range(0, Next()).Select(_ => problem.AllObjects[Next()]).ToArray();
        // The fields of a record are read in the order they are written.
        var inserts = new SerialInsert[Next()];
        for (int index = 0; index < inserts.Length; index++)
        {
            int point = Next();
            ActionSchema schema = problem.Domain.Actions[Next()];
            inserts[index] = new SerialInsert(point, schema, Binding());
        }

        var deleted = new int[Next()];
        for (int index = 0; index < deleted.Length; index++) deleted[index] = Next();
        var moves = new SerialMove[Next()];
        for (int index = 0; index < moves.Length; index++)
        {
            int stepId = Next();
            moves[index] = new SerialMove(stepId, Next());
        }

        var rebinds = new SerialRebind[Next()];
        for (int index = 0; index < rebinds.Length; index++)
        {
            int stepId = Next();
            rebinds[index] = new SerialRebind(stepId, Binding());
        }

        if (position != values.Length) throw new ArgumentException("The edits have trailing values.");
        return new SerialEdits { Inserts = inserts, DeletedStepIds = deleted, Moves = moves, Rebinds = rebinds };
    }
}
