using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.PlanSpace;

namespace Mimir.Learning;

public sealed class SerialEncodingLayout
{
    public required int InitialNodeId { get; init; }
    public required int GoalNodeId { get; init; }
    public required int[] StepNodeIds { get; init; }
    public required int[][] ParameterNodeIds { get; init; }
    public required int[] CandidateNodeIds { get; init; }
    public required int[][] CandidateParameterNodeIds { get; init; }
    public required int[] ErrorNodeIds { get; init; }
    public required int[][] RepairNodeIds { get; init; }
}

public static partial class Encoding
{
    private static readonly (string Name, int Arity)[] FixedSerialRelations =
    [
        ("serial_initial", 1),
        ("serial_goal", 1),
        ("serial_step", 1),
        ("serial_candidate", 1),
        ("serial_unbound", 1),
        ("serial_unused", 1),
        ("serial_repair_noop", 1),
        ("serial_bound", 2),
        ("serial_next", 2),
        ("serial_purpose", 2),
        ("serial_error_unbound", 2),
        ("serial_repairs", 2),
        ("serial_repair_insert", 2),
        ("serial_repair_bind", 2),
        ("serial_repair_rebind", 3),
    ];

    public static IReadOnlyList<(string Name, int Arity)> SerialPlanRelations(Domain domain, string suffix = "")
    {
        ArgumentNullException.ThrowIfNull(domain);
        ValidateSuffix(suffix);

        var relations = new List<(string Name, int Arity)>();
        foreach ((string name, int arity) in FixedSerialRelations)
            relations.Add(($"{name}{suffix}", arity));
        foreach (ActionSchema schema in domain.Actions)
            relations.Add(($"serial_action_{schema.Name}{suffix}", 1 + schema.Parameters.Count));
        foreach (Mimir.Core.Schemas.Predicate<Fluent> predicate in domain.Fluents)
        {
            int arity = predicate.Parameters.Count;
            // A final fact without arguments is repeated for every object, as in EncodeState.
            int finalArity = Math.Max(1, arity);
            relations.Add(($"serial_support_{predicate.Name}{suffix}", 2 + arity));
            relations.Add(($"serial_support_not_{predicate.Name}{suffix}", 2 + arity));
            relations.Add(($"serial_error_pre_{predicate.Name}{suffix}", 2 + arity));
            relations.Add(($"serial_error_pre_not_{predicate.Name}{suffix}", 2 + arity));
            relations.Add(($"serial_error_goal_{predicate.Name}{suffix}", 1 + arity));
            relations.Add(($"serial_error_goal_not_{predicate.Name}{suffix}", 1 + arity));
            relations.Add(($"serial_final_{predicate.Name}{suffix}", finalArity));
            relations.Add(($"serial_final_unknown_{predicate.Name}{suffix}", finalArity));
        }
        return relations;
    }

    public static SerialEncodingLayout EncodeSerialPlan(
        EncodingContext context,
        SerialPlan plan,
        SerialDiagnosis diagnosis,
        string suffix = "")
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(plan);
        ArgumentNullException.ThrowIfNull(diagnosis);
        ValidateSuffix(suffix);
        if (!ReferenceEquals(context.Problem, plan.Problem))
        {
            throw new ArgumentException(
                "Plan belongs to a different problem than the active encoding instance.",
                nameof(plan));
        }
        if (!ReferenceEquals(diagnosis.Plan, plan) || diagnosis.Version != plan.Version)
            throw new ArgumentException("The diagnosis was not computed from the plan in its current state.", nameof(diagnosis));

        IReadOnlyList<SerialStep> steps = plan.Steps;
        IReadOnlyList<GhostStep> candidates = diagnosis.Candidates;
        IReadOnlyList<SerialError> errors = diagnosis.Errors;

        int nodeCount = 2
            + steps.Count
            + steps.Sum(step => step.Binding.Count)
            + candidates.Count
            + candidates.Sum(candidate => candidate.Binding.Count)
            + errors.Count
            + errors.Sum(error => error.Repairs.Count);
        context.EnsureCanAllocate(nodeCount);

        var layout = new SerialEncodingLayout
        {
            InitialNodeId = context.NewAuxiliaryId(),
            GoalNodeId = context.NewAuxiliaryId(),
            StepNodeIds = steps.Select(_ => context.NewActionId()).ToArray(),
            ParameterNodeIds = steps.Select(step => AllocateAuxiliary(context, step.Binding.Count)).ToArray(),
            CandidateNodeIds = AllocateAuxiliary(context, candidates.Count),
            CandidateParameterNodeIds = candidates.Select(candidate => AllocateAuxiliary(context, candidate.Binding.Count)).ToArray(),
            ErrorNodeIds = AllocateAuxiliary(context, errors.Count),
            RepairNodeIds = errors.Select(error => AllocateAuxiliary(context, error.Repairs.Count)).ToArray(),
        };

        var stepIndexById = new Dictionary<int, int>();
        for (int index = 0; index < steps.Count; index++) stepIndexById.Add(steps[index].Id, index);

        int StepNode(int stepId)
        {
            if (!stepIndexById.TryGetValue(stepId, out int index))
                throw new InvalidOperationException($"Step {stepId} is not part of the plan.");
            return layout.StepNodeIds[index];
        }

        context.GetOrCreateRelation($"serial_initial{suffix}").Add(layout.InitialNodeId);
        context.GetOrCreateRelation($"serial_goal{suffix}").Add(layout.GoalNodeId);

        int previousNode = layout.InitialNodeId;
        for (int index = 0; index < steps.Count; index++)
        {
            SerialStep step = steps[index];
            int node = layout.StepNodeIds[index];
            context.GetOrCreateRelation($"serial_step{suffix}").Add(node);
            AppendSchemaInstance(context, "serial", suffix, node, step.Schema, step.Binding, layout.ParameterNodeIds[index]);
            AppendPair(context, $"serial_next{suffix}", previousNode, node);
            previousNode = node;

            // The step a step was inserted for may have been deleted since, which leaves it without a purpose.
            if (step.PurposeStepId == -1)
                AppendPair(context, $"serial_purpose{suffix}", node, layout.GoalNodeId);
            else if (stepIndexById.TryGetValue(step.PurposeStepId, out int purposeIndex))
                AppendPair(context, $"serial_purpose{suffix}", node, layout.StepNodeIds[purposeIndex]);
        }
        AppendPair(context, $"serial_next{suffix}", previousNode, layout.GoalNodeId);

        foreach (int stepId in diagnosis.UnusedStepIds)
            context.GetOrCreateRelation($"serial_unused{suffix}").Add(StepNode(stepId));

        for (int index = 0; index < candidates.Count; index++)
        {
            int node = layout.CandidateNodeIds[index];
            context.GetOrCreateRelation($"serial_candidate{suffix}").Add(node);
            AppendSchemaInstance(context, "serial", suffix, node, candidates[index].Schema, candidates[index].Binding, layout.CandidateParameterNodeIds[index]);
        }

        foreach (SerialSupport support in diagnosis.Supports)
        {
            List<int> ids = context.GetOrCreateRelation(SerialLiteralRelation("serial_support", support.Literal, suffix));
            ids.Add(support.ProducerStepId == -1 ? layout.InitialNodeId : StepNode(support.ProducerStepId));
            ids.Add(support.ConsumerStepId == -1 ? layout.GoalNodeId : StepNode(support.ConsumerStepId));
            ids.AddRange(GetObjectIds(context, support.Literal.Value.Arguments));
        }

        for (int index = 0; index < errors.Count; index++)
        {
            SerialError error = errors[index];
            int node = layout.ErrorNodeIds[index];
            switch (error.Kind)
            {
                case SerialErrorKind.UnsatisfiedPrecondition:
                    List<int> precondition = context.GetOrCreateRelation(SerialLiteralRelation("serial_error_pre", error.Literal!, suffix));
                    precondition.Add(node);
                    precondition.Add(StepNode(error.StepId));
                    precondition.AddRange(GetObjectIds(context, error.Literal!.Value.Arguments));
                    break;
                case SerialErrorKind.UnmetGoal:
                    AppendFactRow(context, SerialLiteralRelation("serial_error_goal", error.Literal!, suffix), node, error.Literal!.Value);
                    break;
                case SerialErrorKind.UnboundParameter:
                    AppendPair(context, $"serial_error_unbound{suffix}", node, layout.ParameterNodeIds[stepIndexById[error.StepId]][error.ParameterIndex]);
                    break;
                default:
                    throw new NotSupportedException(error.Kind.ToString());
            }

            for (int repairIndex = 0; repairIndex < error.Repairs.Count; repairIndex++)
            {
                SerialRepair repair = error.Repairs[repairIndex];
                int repairNode = layout.RepairNodeIds[index][repairIndex];
                AppendPair(context, $"serial_repairs{suffix}", repairNode, node);

                switch (repair.Kind)
                {
                    case SerialRepairKind.NoOp:
                        context.GetOrCreateRelation($"serial_repair_noop{suffix}").Add(repairNode);
                        break;
                    case SerialRepairKind.Insert:
                        AppendPair(context, $"serial_repair_insert{suffix}", repairNode, layout.CandidateNodeIds[repair.CandidateIndex]);
                        break;
                    case SerialRepairKind.Bind:
                        AppendPair(context, $"serial_repair_bind{suffix}", repairNode, context.GetObjectId(repair.Object!));
                        break;
                    case SerialRepairKind.Rebind:
                        List<int> rebind = context.GetOrCreateRelation($"serial_repair_rebind{suffix}");
                        rebind.Add(repairNode);
                        rebind.Add(layout.ParameterNodeIds[stepIndexById[error.StepId]][repair.ParameterIndex]);
                        rebind.Add(context.GetObjectId(repair.Object!));
                        break;
                    default:
                        throw new NotSupportedException(repair.Kind.ToString());
                }
            }
        }

        foreach (Fact<Fluent> fact in diagnosis.FinalTrueFacts)
            AppendFact(context, fact, $"serial_final_{fact.Predicate.Name}{suffix}");
        foreach (Fact<Fluent> fact in diagnosis.FinalUnknownFacts)
            AppendFact(context, fact, $"serial_final_unknown_{fact.Predicate.Name}{suffix}");

        return layout;
    }

    private static string SerialLiteralRelation(string prefix, Literal<Fact> literal, string suffix)
        => $"{prefix}_{(literal.IsPositive ? "" : "not_")}{literal.Value.Predicate.Name}{suffix}";
}
