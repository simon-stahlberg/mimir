using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.PlanSpace;

namespace Mimir.Learning.Tests;

public class SerialPlanEncodingTests
{
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    private static Problem LoadBlocks()
    {
        Domain domain = Domain.FromFile(Path.Combine(ExamplesPath, "blocks_4", "domain.pddl"));
        return Problem.FromText(domain, """
(define (problem b1-on-b3)
  (:domain blocksworld)
  (:objects b1 b2 b3)
  (:init (arm-empty) (clear b1) (on b1 b3) (on-table b3) (clear b2) (on-table b2))
  (:goal (and (on b2 b3) (on-table b1) (not (clear b1)))))
""");
    }

    private static void Insert(SerialPlan plan, params (string Error, string Candidate)[] choices)
    {
        SerialDiagnosis diagnosis = plan.Diagnose();
        var chosen = new int[diagnosis.Errors.Count];
        foreach ((string error, string candidate) in choices)
        {
            int errorIndex = diagnosis.Errors.ToList().FindIndex(value => value.ToString() == error);
            Assert.True(errorIndex >= 0, $"No error {error} among {string.Join(", ", diagnosis.Errors)}");
            chosen[errorIndex] = InsertIndex(diagnosis, diagnosis.Errors[errorIndex], candidate);
        }

        plan.Apply(diagnosis, chosen, []);
    }

    private static int InsertIndex(SerialDiagnosis diagnosis, SerialError error, string candidate)
    {
        int index = error.Repairs.ToList().FindIndex(repair =>
            repair.Kind == SerialRepairKind.Insert && diagnosis.Candidates[repair.CandidateIndex].ToString() == candidate);
        Assert.True(index > 0, $"No insert of {candidate} for {error}.");
        return index;
    }

    // 0:(stack b2 b3) 2:(unstack b1 ?) 1:(putdown b1), judged against a goal without (on-table b1): the stack
    // fails on (clear b3) and (holding b2), the unstack has an unbound parameter, nothing consumes what the
    // putdown produces and b1 is clear in the end.
    private static SerialPlan BuildPlanWithEveryErrorKind(Problem problem)
    {
        SerialPlan plan = SerialPlan.Empty(problem);
        Insert(plan, ("UnmetGoal((on b2 b3))", "(stack b2 b3)"), ("UnmetGoal((on-table b1))", "(putdown b1)"));
        Insert(plan, ("UnsatisfiedPrecondition(step 1, (holding b1))", "(unstack b1 ?)"));
        return plan.WithGoal(problem.Goal.Where(literal => literal.Value.ToString() != "(on-table b1)").ToArray());
    }

    private static IEnumerable<int[]> Rows(IReadOnlyList<int> values, int arity)
    {
        Assert.Equal(0, values.Count % arity);
        for (int offset = 0; offset < values.Count; offset += arity)
            yield return values.Skip(offset).Take(arity).ToArray();
    }

    private static void AssertRow(EncodingContext context, string relation, params int[] expected)
        => Assert.Contains(Rows(context.Relations[relation], expected.Length), row => row.SequenceEqual(expected));

    [Fact]
    public void SerialPlanEncodesStepsSupportsErrorsAndRepairsMatchingTheLayout()
    {
        Problem problem = LoadBlocks();
        SerialPlan plan = BuildPlanWithEveryErrorKind(problem);
        SerialDiagnosis diagnosis = plan.Diagnose();
        Assert.Equal(["0:(stack b2 b3)", "2:(unstack b1 ?)", "1:(putdown b1)"], plan.Steps.Select(step => step.ToString()));
        Assert.Equal(
            [
                "UnsatisfiedPrecondition(step 0, (clear b3))",
                "UnsatisfiedPrecondition(step 0, (holding b2))",
                "UnboundParameter(step 2, parameter 1)",
                "UnmetGoal((not (clear b1)))",
            ],
            diagnosis.Errors.Select(error => error.ToString()));
        Assert.Equal(["-1->2 (clear b1)", "0->2 (arm-empty)", "2->1 (holding b1)", "0->-1 (on b2 b3)"], diagnosis.Supports.Select(support => support.ToString()));
        Assert.Equal([1], diagnosis.UnusedStepIds);

        var context = new EncodingContext();
        context.BeginInstance(problem);
        Dictionary<string, int> objects = problem.AllObjects.ToDictionary(constant => constant.Name, context.GetObjectId);
        SerialEncodingLayout layout = Encoding.EncodeSerialPlan(context, plan, diagnosis, "_s");
        context.EndInstance();

        int initial = layout.InitialNodeId;
        int goal = layout.GoalNodeId;
        int stack = layout.StepNodeIds[0];
        int unstack = layout.StepNodeIds[1];
        int putdown = layout.StepNodeIds[2];
        int[] stackParameters = layout.ParameterNodeIds[0];
        int[] unstackParameters = layout.ParameterNodeIds[1];

        Assert.Equal([initial], context.Relations["serial_initial_s"]);
        Assert.Equal([goal], context.Relations["serial_goal_s"]);
        Assert.Equal(layout.StepNodeIds, context.Relations["serial_step_s"]);
        Assert.Equal(layout.StepNodeIds, context.ActionIndices);
        Assert.Equal([initial, stack, stack, unstack, unstack, putdown, putdown, goal], context.Relations["serial_next_s"]);
        Assert.Equal([stack, goal, unstack, putdown, putdown, goal], context.Relations["serial_purpose_s"]);
        Assert.Equal([putdown], context.Relations["serial_unused_s"]);
        AssertRow(context, "serial_action_stack_s", stack, stackParameters[0], stackParameters[1]);
        AssertRow(context, "serial_action_unstack_s", unstack, unstackParameters[0], unstackParameters[1]);
        AssertRow(context, "serial_action_putdown_s", putdown, layout.ParameterNodeIds[2][0]);
        AssertRow(context, "serial_bound_s", stackParameters[0], objects["b2"]);
        AssertRow(context, "serial_bound_s", stackParameters[1], objects["b3"]);
        AssertRow(context, "serial_bound_s", unstackParameters[0], objects["b1"]);
        Assert.Contains(unstackParameters[1], context.Relations["serial_unbound_s"]);
        Assert.DoesNotContain(unstackParameters[0], context.Relations["serial_unbound_s"]);

        Assert.Equal([initial, unstack, objects["b1"]], context.Relations["serial_support_clear_s"]);
        Assert.Equal([stack, unstack], context.Relations["serial_support_arm-empty_s"]);
        Assert.Equal([unstack, putdown, objects["b1"]], context.Relations["serial_support_holding_s"]);
        Assert.Equal([stack, goal, objects["b2"], objects["b3"]], context.Relations["serial_support_on_s"]);

        Assert.Equal(4, layout.ErrorNodeIds.Length);
        Assert.Equal([layout.ErrorNodeIds[0], stack, objects["b3"]], context.Relations["serial_error_pre_clear_s"]);
        Assert.Equal([layout.ErrorNodeIds[1], stack, objects["b2"]], context.Relations["serial_error_pre_holding_s"]);
        Assert.Equal([layout.ErrorNodeIds[2], unstackParameters[1]], context.Relations["serial_error_unbound_s"]);
        Assert.Equal([layout.ErrorNodeIds[3], objects["b1"]], context.Relations["serial_error_goal_not_clear_s"]);

        SerialError clearB3 = diagnosis.Errors[0];
        Assert.Equal(
            ["NoOp", "Insert(0)", "Insert(1)", "Insert(2)", "Rebind(1, b1)", "Rebind(1, b2)"],
            clearB3.Repairs.Select(repair => repair.ToString()));
        Assert.Equal(
            [layout.RepairNodeIds[0][4], stackParameters[1], objects["b1"], layout.RepairNodeIds[0][5], stackParameters[1], objects["b2"]],
            context.Relations["serial_repair_rebind_s"]);
        int unstackOntoB3 = diagnosis.Candidates.ToList().FindIndex(candidate => candidate.ToString() == "(unstack ? b3)");
        int[] candidateParameters = layout.CandidateParameterNodeIds[unstackOntoB3];
        AssertRow(context, "serial_repair_insert_s", layout.RepairNodeIds[0][InsertIndex(diagnosis, clearB3, "(unstack ? b3)")], layout.CandidateNodeIds[unstackOntoB3]);
        AssertRow(context, "serial_action_unstack_s", layout.CandidateNodeIds[unstackOntoB3], candidateParameters[0], candidateParameters[1]);
        Assert.Contains(candidateParameters[0], context.Relations["serial_unbound_s"]);
        AssertRow(context, "serial_bound_s", candidateParameters[1], objects["b3"]);
        Assert.Equal(diagnosis.Candidates.Count, layout.CandidateNodeIds.Length);
        Assert.Equal(layout.CandidateNodeIds, context.Relations["serial_candidate_s"]);

        int[] expectedBinds =
        [
            layout.RepairNodeIds[2][1], objects["b1"],
            layout.RepairNodeIds[2][2], objects["b2"],
            layout.RepairNodeIds[2][3], objects["b3"],
        ];
        Assert.Equal(expectedBinds, context.Relations["serial_repair_bind_s"]);

        int[] expectedRepairs = layout.RepairNodeIds
            .SelectMany((repairs, errorIndex) => repairs.SelectMany(repair => new[] { repair, layout.ErrorNodeIds[errorIndex] }))
            .ToArray();
        Assert.Equal(expectedRepairs, context.Relations["serial_repairs_s"]);
        Assert.Equal(diagnosis.Errors.Select(error => error.Repairs.Count), layout.RepairNodeIds.Select(repairs => repairs.Length));
        Assert.Equal(layout.RepairNodeIds.Select(repairs => repairs[0]), context.Relations["serial_repair_noop_s"]);

        // (arm-empty) has no arguments, so it is repeated for every object.
        Assert.Equal(objects.Values, context.Relations["serial_final_arm-empty_s"]);
        AssertRow(context, "serial_final_on_s", objects["b2"], objects["b3"]);
        AssertRow(context, "serial_final_clear_s", objects["b1"]);
        AssertRow(context, "serial_final_unknown_clear_s", objects["b3"]);
        AssertRow(context, "serial_final_unknown_on_s", objects["b1"], objects["b3"]);

        int auxiliaryCount = 2
            + layout.ParameterNodeIds.Sum(ids => ids.Length)
            + layout.CandidateNodeIds.Length
            + layout.CandidateParameterNodeIds.Sum(ids => ids.Length)
            + layout.ErrorNodeIds.Length
            + layout.RepairNodeIds.Sum(ids => ids.Length);
        Assert.Equal([3], context.ActionSizes);
        Assert.Equal([auxiliaryCount], context.AuxiliarySizes);
        Assert.Equal([3 + 3 + auxiliaryCount], context.NodeSizes);

        Dictionary<string, int> arities = Encoding.SerialPlanRelations(problem.Domain, "_s").ToDictionary(relation => relation.Name, relation => relation.Arity);
        Assert.All(context.Relations, relation =>
        {
            Assert.True(arities.ContainsKey(relation.Key), $"Relation {relation.Key} is not declared.");
            Assert.NotEmpty(relation.Value);
            Assert.Equal(0, relation.Value.Count % arities[relation.Key]);
        });
    }

    [Fact]
    public void NodesAreAllocatedInLayoutOrder()
    {
        Problem problem = LoadBlocks();
        SerialPlan plan = BuildPlanWithEveryErrorKind(problem);
        var context = new EncodingContext();
        context.BeginInstance(problem);
        SerialEncodingLayout layout = Encoding.EncodeSerialPlan(context, plan, plan.Diagnose());
        context.EndInstance();

        int[] allocated = new[] { layout.InitialNodeId, layout.GoalNodeId }
            .Concat(layout.StepNodeIds)
            .Concat(layout.ParameterNodeIds.SelectMany(ids => ids))
            .Concat(layout.CandidateNodeIds)
            .Concat(layout.CandidateParameterNodeIds.SelectMany(ids => ids))
            .Concat(layout.ErrorNodeIds)
            .Concat(layout.RepairNodeIds.SelectMany(ids => ids))
            .ToArray();
        Assert.Equal(Enumerable.Range(problem.AllObjects.Count, allocated.Length), allocated);
    }

    [Fact]
    public void AnEmptyPlanLinksTheInitialStateToTheGoal()
    {
        Problem problem = LoadBlocks();
        SerialPlan plan = SerialPlan.Empty(problem);
        var context = new EncodingContext();
        context.BeginInstance(problem);
        int b1 = context.GetObjectId(problem.AllObjects.Single(constant => constant.Name == "b1"));
        SerialEncodingLayout layout = Encoding.EncodeSerialPlan(context, plan, plan.Diagnose());
        context.EndInstance();

        Assert.Empty(layout.StepNodeIds);
        Assert.Equal([layout.InitialNodeId, layout.GoalNodeId], context.Relations["serial_next"]);
        Assert.False(context.Relations.ContainsKey("serial_step"));
        Assert.False(context.Relations.ContainsKey("serial_purpose"));
        Assert.Equal([layout.ErrorNodeIds[1], b1], context.Relations["serial_error_goal_on-table"]);
        Assert.Equal([layout.ErrorNodeIds[2], b1], context.Relations["serial_error_goal_not_clear"]);
    }

    [Fact]
    public void AStepWhosePurposeStepWasDeletedHasNoPurpose()
    {
        Problem problem = LoadBlocks();
        SerialPlan plan = BuildPlanWithEveryErrorKind(problem);
        SerialDiagnosis diagnosis = plan.Diagnose();
        plan.Apply(diagnosis, new int[diagnosis.Errors.Count], [1]);
        Assert.Equal([(0, -1), (2, 1)], plan.Steps.Select(step => (step.Id, step.PurposeStepId)));

        var context = new EncodingContext();
        context.BeginInstance(problem);
        SerialEncodingLayout layout = Encoding.EncodeSerialPlan(context, plan, plan.Diagnose());
        context.EndInstance();

        Assert.Equal([layout.StepNodeIds[0], layout.GoalNodeId], context.Relations["serial_purpose"]);
    }

    [Fact]
    public void RelationListCoversFixedSchemaAndFluentRelations()
    {
        Problem problem = LoadBlocks();
        Dictionary<string, int> relations = Encoding.SerialPlanRelations(problem.Domain, "_s").ToDictionary(relation => relation.Name, relation => relation.Arity);

        Assert.Equal(1, relations["serial_initial_s"]);
        Assert.Equal(2, relations["serial_next_s"]);
        Assert.Equal(3, relations["serial_repair_rebind_s"]);
        Assert.Equal(3, relations["serial_action_stack_s"]);
        Assert.Equal(4, relations["serial_support_on_s"]);
        Assert.Equal(4, relations["serial_support_not_on_s"]);
        Assert.Equal(3, relations["serial_error_pre_clear_s"]);
        Assert.Equal(3, relations["serial_error_pre_not_clear_s"]);
        Assert.Equal(2, relations["serial_error_goal_clear_s"]);
        Assert.Equal(2, relations["serial_error_goal_not_clear_s"]);
        Assert.Equal(2, relations["serial_final_on_s"]);
        Assert.Equal(2, relations["serial_final_unknown_on_s"]);
        Assert.Equal(2, relations["serial_support_arm-empty_s"]);
        Assert.Equal(1, relations["serial_error_goal_arm-empty_s"]);
        Assert.Equal(1, relations["serial_final_arm-empty_s"]);
        Assert.Equal(15 + problem.Domain.Actions.Count + 8 * problem.Domain.Fluents.Count, relations.Count);
    }

    [Fact]
    public void EncodingRejectsForeignPlansAndForeignOrStaleDiagnoses()
    {
        Problem problem = LoadBlocks();
        SerialPlan plan = SerialPlan.Empty(problem);
        SerialDiagnosis foreignDiagnosis = plan.Clone().Diagnose();
        var context = new EncodingContext();

        Assert.Throws<InvalidOperationException>(() => Encoding.EncodeSerialPlan(context, plan, plan.Diagnose()));
        context.BeginInstance(problem);
        Assert.Throws<ArgumentException>(() => Encoding.EncodeSerialPlan(context, plan, foreignDiagnosis));
        SerialDiagnosis staleDiagnosis = plan.Diagnose();
        plan.Apply(staleDiagnosis, new int[staleDiagnosis.Errors.Count], []);
        Assert.Throws<ArgumentException>(() => Encoding.EncodeSerialPlan(context, plan, staleDiagnosis));
        context.EndInstance();

        Problem other = Problem.FromFile(problem.Domain, Path.Combine(ExamplesPath, "blocks_4", "p01.pddl"));
        context.BeginInstance(other);
        Assert.Throws<ArgumentException>(() => Encoding.EncodeSerialPlan(context, plan, plan.Diagnose()));
        context.EndInstance();
    }
}
