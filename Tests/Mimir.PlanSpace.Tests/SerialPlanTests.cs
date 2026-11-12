using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit.Abstractions;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace.Tests;

public class SerialPlanTests
{
    private readonly ITestOutputHelper _output;

    public SerialPlanTests(ITestOutputHelper output) => _output = output;

    private static Domain Blocksworld() => Domain.FromText("""
(define (domain blocksworld)
  (:requirements :strips)
  (:predicates (clear ?x) (on-table ?x) (arm-empty) (holding ?x) (on ?x ?y))
  (:action pickup :parameters (?ob)
    :precondition (and (clear ?ob) (on-table ?ob) (arm-empty))
    :effect (and (holding ?ob) (not (clear ?ob)) (not (on-table ?ob)) (not (arm-empty))))
  (:action putdown :parameters (?ob)
    :precondition (holding ?ob)
    :effect (and (clear ?ob) (arm-empty) (on-table ?ob) (not (holding ?ob))))
  (:action stack :parameters (?ob ?underob)
    :precondition (and (clear ?underob) (holding ?ob))
    :effect (and (arm-empty) (clear ?ob) (on ?ob ?underob) (not (clear ?underob)) (not (holding ?ob))))
  (:action unstack :parameters (?ob ?underob)
    :precondition (and (on ?ob ?underob) (clear ?ob) (arm-empty))
    :effect (and (holding ?ob) (clear ?underob) (not (on ?ob ?underob)) (not (clear ?ob)) (not (arm-empty)))))
""");

    private static Problem BlocksOnTable(string goal) => Problem.FromText(Blocksworld(), $"""
(define (problem blocks-on-table)
  (:domain blocksworld)
  (:objects b1 b2 b3)
  (:init (arm-empty) (on-table b1) (on-table b2) (on-table b3) (clear b1) (clear b2) (clear b3))
  (:goal {goal}))
""");

    private static Problem HoldingFirstBlock(string goal) => Problem.FromText(Blocksworld(), $"""
(define (problem holding-b1)
  (:domain blocksworld)
  (:objects b1 b2 b3)
  (:init (holding b1) (on-table b2) (on-table b3) (clear b2) (clear b3))
  (:goal {goal}))
""");

    private static List<List<int>> RandomTowers(int blocks, Random random)
    {
        var towers = new List<List<int>>();
        foreach (int block in Enumerable.Range(1, blocks).OrderBy(_ => random.Next()))
        {
            int tower = random.Next(towers.Count + 1);
            if (tower == towers.Count) towers.Add(new List<int>());
            towers[tower].Add(block);
        }

        return towers;
    }

    private static Problem RandomBlocks(int blocks, int seed)
    {
        var random = new Random(seed);
        var init = new List<string> { "(arm-empty)" };
        foreach (List<int> tower in RandomTowers(blocks, random))
        {
            init.Add($"(on-table b{tower[0]})");
            for (int i = 1; i < tower.Count; i++) init.Add($"(on b{tower[i]} b{tower[i - 1]})");
            init.Add($"(clear b{tower[^1]})");
        }

        var goal = new List<string>();
        foreach (List<int> tower in RandomTowers(blocks, random))
        {
            goal.Add($"(on-table b{tower[0]})");
            for (int i = 1; i < tower.Count; i++) goal.Add($"(on b{tower[i]} b{tower[i - 1]})");
        }

        return Problem.FromText(Blocksworld(), $"""
(define (problem blocks-{blocks}-{seed})
  (:domain blocksworld)
  (:objects {string.Join(" ", Enumerable.Range(1, blocks).Select(i => $"b{i}"))})
  (:init {string.Join(" ", init)})
  (:goal (and {string.Join(" ", goal)})))
""");
    }

    private static Problem Gripper(int balls)
    {
        Domain domain = Domain.FromText("""
(define (domain gripper-strips)
  (:requirements :strips)
  (:constants rooma roomb)
  (:predicates (room ?r) (ball ?b) (gripper ?g) (at-robby ?r) (at ?b ?r) (free ?g) (carry ?o ?g))
  (:action move :parameters (?from ?to)
    :precondition (and (room ?from) (room ?to) (at-robby ?from))
    :effect (and (at-robby ?to) (not (at-robby ?from))))
  (:action pick :parameters (?obj ?room ?gripper)
    :precondition (and (ball ?obj) (room ?room) (gripper ?gripper) (at ?obj ?room) (at-robby ?room) (free ?gripper))
    :effect (and (carry ?obj ?gripper) (not (at ?obj ?room)) (not (free ?gripper))))
  (:action drop :parameters (?obj ?room ?gripper)
    :precondition (and (ball ?obj) (room ?room) (gripper ?gripper) (carry ?obj ?gripper) (at-robby ?room))
    :effect (and (at ?obj ?room) (free ?gripper) (not (carry ?obj ?gripper)))))
""");
        string[] names = Enumerable.Range(1, balls).Select(i => $"ball{i}").ToArray();
        return Problem.FromText(domain, $"""
(define (problem gripper-{balls})
  (:domain gripper-strips)
  (:objects {string.Join(" ", names)} left right)
  (:init (room rooma) (room roomb) (gripper left) (gripper right) (at-robby rooma) (free left) (free right)
         {string.Join(" ", names.Select(name => $"(ball {name}) (at {name} rooma)"))})
  (:goal (and {string.Join(" ", names.Select(name => $"(at {name} roomb)"))})))
""");
    }

    // Roads lead from l1 to l2 and l3 and from l2 and l4 to l3; t1 starts at l1 and t2 at l2.
    private static Problem Drive(string goal)
    {
        Domain domain = Domain.FromText("""
(define (domain drive)
  (:requirements :strips :typing :equality)
  (:types truck location - object)
  (:predicates (at ?t - truck ?l - location) (road ?a ?b - location) (patrolled ?t - truck))
  (:action drive
    :parameters (?t - truck ?from ?to - location)
    :precondition (and (at ?t ?from) (road ?from ?to) (not (= ?from ?to)))
    :effect (and (at ?t ?to) (not (at ?t ?from))))
  (:action patrol
    :parameters (?t - truck ?a ?b - location)
    :precondition (and (at ?t ?a) (road ?a ?b))
    :effect (patrolled ?t)))
""");
        return Problem.FromText(domain, $"""
(define (problem drive-1)
  (:domain drive)
  (:objects t1 t2 - truck l1 l2 l3 l4 - location)
  (:init (at t1 l1) (at t2 l2) (road l1 l2) (road l2 l3) (road l4 l3) (road l1 l3))
  (:goal {goal}))
""");
    }

    private static string Describe(SerialPlan plan) => string.Join(" ", plan.Steps);

    private static string[] Errors(SerialDiagnosis diagnosis) => diagnosis.Errors.Select(error => error.ToString()).ToArray();

    private static string[] Supports(SerialDiagnosis diagnosis) => diagnosis.Supports.Select(support => support.ToString()).ToArray();

    private static string[] Repairs(SerialDiagnosis diagnosis, string error)
        => diagnosis.Errors.Single(candidate => candidate.ToString() == error).Repairs.Select(repair => RepairText(diagnosis, repair)).ToArray();

    private static string RepairText(SerialDiagnosis diagnosis, SerialRepair repair)
        => repair.Kind == SerialRepairKind.Insert ? $"Insert{diagnosis.Candidates[repair.CandidateIndex]}" : repair.ToString();

    // Applies the named repair of each named error and NoOp to every other error.
    private static SerialEditResult Repair(SerialPlan plan, params (string Error, string Repair)[] choices)
    {
        SerialDiagnosis diagnosis = plan.Diagnose();
        var chosen = new int[diagnosis.Errors.Count];
        foreach ((string error, string repair) in choices)
        {
            int errorIndex = Array.IndexOf(Errors(diagnosis), error);
            Assert.True(errorIndex >= 0, $"No error {error} among {string.Join(", ", Errors(diagnosis))}");
            chosen[errorIndex] = Array.IndexOf(Repairs(diagnosis, error), repair);
            Assert.True(chosen[errorIndex] >= 0, $"No repair {repair} among {string.Join(", ", Repairs(diagnosis, error))}");
        }

        return plan.Apply(diagnosis, chosen, []);
    }

    private static string[] AssertExecutes(SerialPlan plan)
    {
        Assert.True(plan.IsSolution(), $"Errors: {string.Join(", ", Errors(plan.Diagnose()))}");
        IReadOnlyList<Action> actions = plan.ToActions();
        ExtendedState state = plan.Problem.InitialState.Expand();
        foreach (Action action in actions)
        {
            Assert.True(action.IsApplicable(state), $"{action} is not applicable.");
            state = state.Apply(action).Expand();
        }

        foreach (Literal<Fact> literal in plan.Goal)
            Assert.True(state.IsTrue(literal.Value) == literal.IsPositive, $"Goal {literal} does not hold.");
        return actions.Select(action => action.ToString()).ToArray();
    }

    private static SerialPlan Solve(Problem problem)
    {
        SerialPlan? plan = SerialSearch.Solve(problem, 200_000, TimeSpan.FromSeconds(10));
        Assert.NotNull(plan);
        return plan;
    }

    private static int RollOut(SerialPlan current, SerialPlan reference)
    {
        int rounds = 0;
        while (!current.IsSolution())
        {
            Assert.True(rounds < 4 * reference.Steps.Count + 10, $"No solution after {rounds} rounds: {Describe(current)}; errors: {string.Join(", ", Errors(current.Diagnose()))}");
            SerialDiagnosis diagnosis = current.Diagnose();
            SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
            current.Apply(diagnosis, targets.RepairIndexPerError, targets.StepIdsToDelete);
            rounds++;
        }

        return rounds;
    }

    private static string[] TargetTexts(SerialDiagnosis diagnosis, SerialOracleTargets targets)
        => diagnosis.Errors.Select((error, index) => RepairText(diagnosis, error.Repairs[targets.RepairIndexPerError[index]])).ToArray();

    // Follows the oracle, except that chooseBind replaces its target for every unbound parameter.
    private static bool TryRollOut(SerialPlan current, SerialPlan reference, int maxRounds, Func<SerialError, int, int> chooseBind)
    {
        for (int round = 0; round < maxRounds && !current.IsSolution(); round++)
        {
            SerialDiagnosis diagnosis = current.Diagnose();
            SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
            int[] chosen = targets.RepairIndexPerError.ToArray();
            for (int i = 0; i < chosen.Length; i++)
            {
                if (diagnosis.Errors[i].Kind == SerialErrorKind.UnboundParameter) chosen[i] = chooseBind(diagnosis.Errors[i], chosen[i]);
            }

            current.Apply(diagnosis, chosen, targets.StepIdsToDelete);
        }

        return current.IsSolution();
    }

    // Follows the oracle, except for a random repair of two errors in five and the deletion of one step in ten.
    private static void EditAtRandom(SerialPlan current, SerialPlan reference, Random random, int rounds)
    {
        for (int round = 0; round < rounds; round++)
        {
            SerialDiagnosis diagnosis = current.Diagnose();
            SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
            int[] chosen = targets.RepairIndexPerError.ToArray();
            for (int i = 0; i < chosen.Length; i++)
            {
                if (random.NextDouble() < 0.4) chosen[i] = random.Next(diagnosis.Errors[i].Repairs.Count);
            }

            int[] deleted = current.Steps.Select(step => step.Id).Where(_ => random.NextDouble() < 0.1).ToArray();
            current.Apply(diagnosis, chosen, deleted);
        }
    }

    private static SerialPlan PickupAndStack(Problem problem)
    {
        SerialPlan plan = SerialPlan.Empty(problem, [problem.Goal.Single(literal => literal.Value.ToString() == "(on b1 b2)")]);
        Repair(plan, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));
        Repair(plan, ("UnsatisfiedPrecondition(step 0, (holding b1))", "Insert(pickup b1)"));
        return plan;
    }

    [Fact]
    public void EmptyPlanHasOneUnmetGoalPerUnsatisfiedGoalLiteral()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b2 b3) (on-table b3))");
        SerialPlan plan = SerialPlan.Empty(problem);

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(["UnmetGoal((on b1 b2))", "UnmetGoal((on b2 b3))"], Errors(diagnosis));
        Assert.All(diagnosis.Errors, error => Assert.Equal((SerialErrorKind.UnmetGoal, -1, -1), (error.Kind, error.StepId, error.ParameterIndex)));
        Assert.Equal(["NoOp", "Insert(stack b1 b2)"], Repairs(diagnosis, "UnmetGoal((on b1 b2))"));
        Assert.Equal(["NoOp", "Insert(stack b2 b3)"], Repairs(diagnosis, "UnmetGoal((on b2 b3))"));
        Assert.Equal(["(stack b1 b2)", "(stack b2 b3)"], diagnosis.Candidates.Select(candidate => candidate.ToString()));
        Assert.Equal(["-1->-1 (on-table b3)"], Supports(diagnosis));
        Assert.Empty(diagnosis.UnusedStepIds);
        Assert.Equal(problem.InitialState.GetTrueFacts().ToHashSet(), diagnosis.FinalTrueFacts.ToHashSet());
        Assert.Empty(diagnosis.FinalUnknownFacts);
        Assert.False(diagnosis.IsSolution);
        Assert.False(plan.IsSolution());
        Assert.Throws<InvalidOperationException>(() => plan.ToActions());
    }

    [Fact]
    public void AFailingStepStillAppliesItsEffects()
    {
        SerialPlan plan = SerialPlan.Empty(BlocksOnTable("(and (on b1 b2) (holding b1))"));
        Repair(plan, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"), ("UnmetGoal((holding b1))", "Insert(unstack b1 ?)"));
        Repair(plan, ("UnboundParameter(step 1, parameter 1)", "Bind(1, b2)"));
        Assert.Equal("0:(stack b1 b2) 1:(unstack b1 b2)", Describe(plan));

        SerialDiagnosis diagnosis = plan.Diagnose();

        // Nothing holds b1 for the stack, yet the unstack after it finds b1 on b2 and takes it off again.
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (holding b1))", "UnmetGoal((on b1 b2))"], Errors(diagnosis));
        Assert.Equal(
            ["-1->0 (clear b2)", "0->1 (on b1 b2)", "0->1 (clear b1)", "0->1 (arm-empty)", "1->-1 (holding b1)"],
            Supports(diagnosis));
    }

    [Fact]
    public void AnUnboundEffectMakesTheFactsItCouldMatchUnknownUntilItIsBound()
    {
        SerialPlan plan = SerialPlan.Empty(HoldingFirstBlock("(and (clear b1) (holding b2))"));
        Repair(plan, ("UnmetGoal((clear b1))", "Insert(stack b1 ?)"), ("UnmetGoal((holding b2))", "Insert(pickup b2)"));
        Assert.Equal("0:(stack b1 ?) 1:(pickup b2)", Describe(plan));

        // The stack may or may not cover b2, so the pickup of b2 is not judged on (clear b2).
        SerialDiagnosis open = plan.Diagnose();
        Assert.Equal(["UnboundParameter(step 0, parameter 1)"], Errors(open));
        Assert.DoesNotContain(open.Supports, support => support.Literal.ToString() == "(clear b2)");
        Assert.Contains("0->-1 (clear b1)", Supports(open));
        Assert.Equal(
            ["(clear b3)", "(on b1 b1)", "(on b1 b2)", "(on b1 b3)"],
            open.FinalUnknownFacts.Select(fact => fact.ToString()).Order());

        SerialPlan covered = plan.Clone();
        Repair(covered, ("UnboundParameter(step 0, parameter 1)", "Bind(1, b2)"));
        Assert.Equal(["UnsatisfiedPrecondition(step 1, (clear b2))"], Errors(covered.Diagnose()));
        Assert.Empty(covered.Diagnose().FinalUnknownFacts);

        Repair(plan, ("UnboundParameter(step 0, parameter 1)", "Bind(1, b3)"));
        Assert.Contains("-1->1 (clear b2)", Supports(plan.Diagnose()));
        Assert.Equal(["(stack b1 b3)", "(pickup b2)"], AssertExecutes(plan));
    }

    private static Problem Doors(string goal)
    {
        Domain domain = Domain.FromText("""
(define (domain doors)
  (:requirements :strips :negative-preconditions)
  (:predicates (locked ?d) (open ?d) (have-key))
  (:action open-door :parameters (?d)
    :precondition (and (not (locked ?d)) (not (open ?d)))
    :effect (open ?d))
  (:action unlock :parameters (?d)
    :precondition (and (locked ?d) (have-key))
    :effect (not (locked ?d))))
""");
        return Problem.FromText(domain, $"""
(define (problem doors-1)
  (:domain doors)
  (:objects d1 d2)
  (:init (locked d1) (have-key))
  (:goal {goal}))
""");
    }

    [Fact]
    public void AViolatedNegativePreconditionOffersTheActionsThatDeleteTheFact()
    {
        SerialPlan plan = SerialPlan.Empty(Doors("(open d1)"));
        Repair(plan, ("UnmetGoal((open d1))", "Insert(open-door d1)"));

        SerialDiagnosis diagnosis = plan.Diagnose();
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (not (locked d1)))"], Errors(diagnosis));
        Assert.Equal(["NoOp", "Insert(unlock d1)", "Rebind(0, d2)"], Repairs(diagnosis, "UnsatisfiedPrecondition(step 0, (not (locked d1)))"));
        Assert.Contains("-1->0 (not (open d1))", Supports(diagnosis));

        Repair(plan, ("UnsatisfiedPrecondition(step 0, (not (locked d1)))", "Insert(unlock d1)"));

        Assert.Contains("1->0 (not (locked d1))", Supports(plan.Diagnose()));
        Assert.Equal(["(unlock d1)", "(open-door d1)"], AssertExecutes(plan));
    }

    [Fact]
    public void AViolatedNegativeGoalLiteralIsAnUnmetGoal()
    {
        SerialPlan plan = SerialPlan.Empty(Doors("(and (not (locked d1)) (not (locked d2)))"));

        SerialDiagnosis diagnosis = plan.Diagnose();
        Assert.Equal(["UnmetGoal((not (locked d1)))"], Errors(diagnosis));
        Assert.Equal(["-1->-1 (not (locked d2))"], Supports(diagnosis));

        Repair(plan, ("UnmetGoal((not (locked d1)))", "Insert(unlock d1)"));

        Assert.Equal(["(unlock d1)"], AssertExecutes(plan));
    }

    [Fact]
    public void AConditionalEffectFiresWhenItsConditionHoldsAndLeavesTheFactUnknownWhenItIsUnknown()
    {
        Domain domain = Domain.FromText("""
(define (domain lamp)
  (:requirements :strips :negative-preconditions :conditional-effects)
  (:predicates (powered ?r) (lit ?r) (pressed))
  (:action switch :parameters (?from ?to)
    :precondition (powered ?from)
    :effect (and (powered ?to) (not (powered ?from))))
  (:action press :parameters (?r)
    :effect (and (pressed) (when (powered ?r) (lit ?r)))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem lamp-1)
  (:domain lamp)
  (:objects r1 r2)
  (:init (powered r2))
  (:goal (and (not (powered r2)) (pressed) (lit r1))))
""");
        SerialPlan plan = SerialPlan.Empty(problem);

        // Only unconditional effects are offered as inserts, so nothing is offered for (lit r1).
        Assert.Equal(["NoOp"], Repairs(plan.Diagnose(), "UnmetGoal((lit r1))"));
        Repair(plan, ("UnmetGoal((not (powered r2)))", "Insert(switch r2 ?)"), ("UnmetGoal((pressed))", "Insert(press ?)"));
        Assert.Equal("0:(switch r2 ?) 1:(press ?)", Describe(plan));

        // A partially bound step with a conditional effect leaves every fact of the predicate it sets open.
        SerialDiagnosis unbound = plan.Diagnose();
        Assert.Equal(["UnboundParameter(step 0, parameter 1)", "UnboundParameter(step 1, parameter 0)"], Errors(unbound));
        Assert.Equal(["(lit r1)", "(lit r2)", "(powered r1)", "(powered r2)"], unbound.FinalUnknownFacts.Select(fact => fact.ToString()).Order());

        // Whether r1 is powered depends on where the switch goes, and with it whether pressing lights r1.
        Repair(plan, ("UnboundParameter(step 1, parameter 0)", "Bind(0, r1)"));
        SerialDiagnosis unknown = plan.Diagnose();
        Assert.Equal(["UnboundParameter(step 0, parameter 1)"], Errors(unknown));
        Assert.Equal(["(lit r1)", "(powered r1)", "(powered r2)"], unknown.FinalUnknownFacts.Select(fact => fact.ToString()).Order());

        SerialPlan unpowered = plan.Clone();
        Repair(unpowered, ("UnboundParameter(step 0, parameter 1)", "Bind(1, r2)"));
        Assert.Equal(["UnmetGoal((not (powered r2)))", "UnmetGoal((lit r1))"], Errors(unpowered.Diagnose()));

        Repair(plan, ("UnboundParameter(step 0, parameter 1)", "Bind(1, r1)"));
        Assert.Contains("1->-1 (lit r1)", Supports(plan.Diagnose()));
        Assert.Equal(["(switch r2 r1)", "(press r1)"], AssertExecutes(plan));
    }

    [Fact]
    public void SupportsNameTheLastStepThatSetTheFactAndUnusedStepsProduceNone()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on-table b3))");
        SerialPlan plan = PickupAndStack(problem);
        Assert.Equal("1:(pickup b1) 0:(stack b1 b2)", Describe(plan));

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(
            ["-1->1 (clear b1)", "-1->1 (on-table b1)", "-1->1 (arm-empty)", "-1->0 (clear b2)", "1->0 (holding b1)", "0->-1 (on b1 b2)"],
            Supports(diagnosis));
        Assert.Empty(diagnosis.UnusedStepIds);
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(plan));

        // Without the goal on (on b1 b2) nothing consumes what the stack produces.
        SerialPlan relabeled = plan.WithGoal([problem.Goal.Single(literal => literal.Value.ToString() == "(on-table b3)")]);
        Assert.Equal([0], relabeled.Diagnose().UnusedStepIds);
        Assert.True(relabeled.IsSolution());
    }

    [Fact]
    public void InsertsGoBeforeTheFlaggedStepAndAtTheEndForGoals()
    {
        SerialPlan plan = SerialPlan.Empty(BlocksOnTable("(and (on b1 b2) (on b1 b3))"));

        SerialEditResult goals = Repair(plan, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"), ("UnmetGoal((on b1 b3))", "Insert(stack b1 b3)"));
        Assert.Equal("0:(stack b1 b2) 1:(stack b1 b3)", Describe(plan));
        Assert.Equal((2, 1), (goals.AppliedCount, plan.Round));
        Assert.Equal([0, 1], goals.CreatedStepIds);
        Assert.Equal([(-1, "(on b1 b2)"), (-1, "(on b1 b3)")], plan.Steps.Select(step => (step.PurposeStepId, step.Purpose.ToString())));

        // The same candidate chosen for errors of different steps creates a step for each of them.
        SerialEditResult pickups = Repair(plan,
            ("UnsatisfiedPrecondition(step 0, (holding b1))", "Insert(pickup b1)"),
            ("UnsatisfiedPrecondition(step 1, (holding b1))", "Insert(pickup b1)"));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2) 3:(pickup b1) 1:(stack b1 b3)", Describe(plan));
        Assert.Equal([2, 3], pickups.CreatedStepIds);
        Assert.Equal([(0, "(holding b1)"), (-1, "(on b1 b2)"), (1, "(holding b1)"), (-1, "(on b1 b3)")], plan.Steps.Select(step => (step.PurposeStepId, step.Purpose.ToString())));
    }

    private static Problem MakeAndUse(string init, string goal)
    {
        Domain domain = Domain.FromText("""
(define (domain make-and-use)
  (:requirements :strips)
  (:predicates (p ?x) (q ?x) (g ?x))
  (:action make :parameters (?x) :effect (and (p ?x) (q ?x)))
  (:action make-p :parameters (?x) :effect (p ?x))
  (:action make-q :parameters (?x) :effect (q ?x))
  (:action use :parameters (?x) :precondition (and (p ?x) (q ?x)) :effect (g ?x)))
""");
        return Problem.FromText(domain, $"""
(define (problem make-and-use-1)
  (:domain make-and-use)
  (:objects a b c)
  (:init {init})
  (:goal {goal}))
""");
    }

    [Fact]
    public void ErrorsOfOneStepShareTheStepOfAnIdenticalCandidateAndKeepErrorOrderOtherwise()
    {
        SerialPlan plan = SerialPlan.Empty(MakeAndUse("", "(and (g a) (g b))"));
        Repair(plan, ("UnmetGoal((g a))", "Insert(use a)"), ("UnmetGoal((g b))", "Insert(use b)"));

        SerialEditResult result = Repair(plan,
            ("UnsatisfiedPrecondition(step 0, (p a))", "Insert(make a)"),
            ("UnsatisfiedPrecondition(step 0, (q a))", "Insert(make a)"),
            ("UnsatisfiedPrecondition(step 1, (p b))", "Insert(make-p b)"),
            ("UnsatisfiedPrecondition(step 1, (q b))", "Insert(make-q b)"));

        Assert.Equal("2:(make a) 0:(use a) 3:(make-p b) 4:(make-q b) 1:(use b)", Describe(plan));
        Assert.Equal(4, result.AppliedCount);
        Assert.Equal([2, 3, 4], result.CreatedStepIds);
        Assert.Empty(result.DroppedErrorIndices);
        Assert.Equal("(p a)", plan.Steps[0].Purpose.ToString());
        Assert.Equal(["(make a)", "(use a)", "(make-p b)", "(make-q b)", "(use b)"], AssertExecutes(plan));
    }

    [Fact]
    public void TwoGoalErrorsShareTheStepOfAnIdenticalCandidate()
    {
        SerialPlan plan = SerialPlan.Empty(HoldingFirstBlock("(and (clear b1) (on-table b1))"));

        SerialEditResult result = Repair(plan, ("UnmetGoal((clear b1))", "Insert(putdown b1)"), ("UnmetGoal((on-table b1))", "Insert(putdown b1)"));

        Assert.Equal([0], result.CreatedStepIds);
        Assert.Equal(["(putdown b1)"], AssertExecutes(plan));
    }

    [Fact]
    public void BindCandidatesRespectTypesAndStaticPreconditions()
    {
        SerialPlan plan = SerialPlan.Empty(Drive("(at t1 l3)"));
        Repair(plan, ("UnmetGoal((at t1 l3))", "Insert(drive t1 ? l3)"));

        // l3 has no road to itself and is excluded by the inequality as well; the trucks are no locations.
        Assert.Equal(
            ["NoOp", "Bind(1, l1)", "Bind(1, l2)", "Bind(1, l4)"],
            Repairs(plan.Diagnose(), "UnboundParameter(step 0, parameter 1)"));
    }

    [Fact]
    public void RebindCandidatesChangeOneParameterSoThatTheFailingLiteralHolds()
    {
        SerialPlan plan = SerialPlan.Empty(Drive("(at t1 l3)"));
        Repair(plan, ("UnmetGoal((at t1 l3))", "Insert(drive t1 ? l3)"));
        SerialPlan fromSecond = plan.Clone();

        // No truck is at l4, and of the locations with a road to l3 only l1 has t1.
        Repair(plan, ("UnboundParameter(step 0, parameter 1)", "Bind(1, l4)"));
        Assert.Equal(
            ["NoOp", "Insert(drive t1 ? l4)", "Rebind(1, l1)"],
            Repairs(plan.Diagnose(), "UnsatisfiedPrecondition(step 0, (at t1 l4))"));
        Repair(plan, ("UnsatisfiedPrecondition(step 0, (at t1 l4))", "Rebind(1, l1)"));
        Assert.Equal(["(drive t1 l1 l3)"], AssertExecutes(plan));

        // t2 is at l2, so the step is executable with t2 instead, which no longer achieves the goal.
        Repair(fromSecond, ("UnboundParameter(step 0, parameter 1)", "Bind(1, l2)"));
        Assert.Equal(
            ["NoOp", "Insert(drive t1 ? l2)", "Rebind(0, t2)", "Rebind(1, l1)"],
            Repairs(fromSecond.Diagnose(), "UnsatisfiedPrecondition(step 0, (at t1 l2))"));
        Repair(fromSecond, ("UnsatisfiedPrecondition(step 0, (at t1 l2))", "Rebind(0, t2)"));
        Assert.Equal("0:(drive t2 l2 l3)", Describe(fromSecond));
        Assert.Equal(["UnmetGoal((at t1 l3))"], Errors(fromSecond.Diagnose()));
    }

    [Fact]
    public void ABindThatConflictsWithAnEarlierOneOfTheSameRoundIsDropped()
    {
        SerialPlan plan = SerialPlan.Empty(Drive("(patrolled t1)"));
        Repair(plan, ("UnmetGoal((patrolled t1))", "Insert(patrol t1 ? ?)"));
        Assert.Contains("Bind(2, l1)", Repairs(plan.Diagnose(), "UnboundParameter(step 0, parameter 2)"));

        // Each bind passes the static preconditions on its own, but there is no road from l3 to l1.
        SerialEditResult result = Repair(plan, ("UnboundParameter(step 0, parameter 1)", "Bind(1, l3)"), ("UnboundParameter(step 0, parameter 2)", "Bind(2, l1)"));

        Assert.Equal("0:(patrol t1 l3 ?)", Describe(plan));
        Assert.Equal(1, result.AppliedCount);
        Assert.Equal([1], result.DroppedErrorIndices);
        Assert.Equal(["NoOp"], Repairs(plan.Diagnose(), "UnboundParameter(step 0, parameter 2)"));
    }

    [Fact]
    public void ASecondRebindOfTheSameParameterInOneRoundIsDropped()
    {
        SerialPlan plan = SerialPlan.Empty(MakeAndUse("(p b) (q b) (p c)", "(g a)"));
        Repair(plan, ("UnmetGoal((g a))", "Insert(use a)"));
        SerialDiagnosis diagnosis = plan.Diagnose();
        Assert.Equal(["NoOp", "Insert(make a)", "Insert(make-p a)", "Rebind(0, b)", "Rebind(0, c)"], Repairs(diagnosis, "UnsatisfiedPrecondition(step 0, (p a))"));
        Assert.Equal(["NoOp", "Insert(make a)", "Insert(make-q a)", "Rebind(0, b)"], Repairs(diagnosis, "UnsatisfiedPrecondition(step 0, (q a))"));

        SerialEditResult result = Repair(plan,
            ("UnsatisfiedPrecondition(step 0, (p a))", "Rebind(0, c)"),
            ("UnsatisfiedPrecondition(step 0, (q a))", "Rebind(0, b)"));

        Assert.Equal("0:(use c)", Describe(plan));
        Assert.Equal(1, result.AppliedCount);
        Assert.Equal([1], result.DroppedErrorIndices);
    }

    [Fact]
    public void RepairsOfADeletedStepAreDropped()
    {
        SerialPlan plan = SerialPlan.Empty(BlocksOnTable("(and (on b1 b2) (holding b3))"));
        Repair(plan, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"), ("UnmetGoal((holding b3))", "Insert(unstack b3 ?)"));
        SerialDiagnosis diagnosis = plan.Diagnose();
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (holding b1))", "UnboundParameter(step 1, parameter 1)"], Errors(diagnosis));

        SerialEditResult result = plan.Apply(diagnosis, [1, 1], [0, 1]);

        Assert.Empty(plan.Steps);
        Assert.Equal(2, result.AppliedCount);
        Assert.Equal([0, 1], result.DroppedErrorIndices);
        Assert.Equal([0, 1], result.DeletedStepIds);
        Assert.Empty(result.CreatedStepIds);

        // Step ids are never reused.
        Repair(plan, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));
        Assert.Equal("2:(stack b1 b2)", Describe(plan));
    }

    [Fact]
    public void ApplyRejectsAStaleDiagnosisAndInvalidChoices()
    {
        SerialPlan plan = SerialPlan.Empty(BlocksOnTable("(on b1 b2)"));
        SerialDiagnosis stale = plan.Diagnose();
        Assert.Throws<InvalidOperationException>(() => plan.Clone().Apply(stale, [1], []));
        Assert.Throws<ArgumentException>(() => plan.Apply(stale, [], []));
        Assert.Throws<ArgumentOutOfRangeException>(() => plan.Apply(stale, [2], []));
        Assert.Throws<ArgumentException>(() => plan.Apply(stale, [0], [7]));
        plan.Apply(stale, [1], []);

        Assert.Throws<InvalidOperationException>(() => plan.Apply(stale, [1], []));
        Assert.Throws<ArgumentException>(() => SerialOracle.Targets(plan, stale, Solve(plan.Problem)));
        Assert.Equal("0:(stack b1 b2)", Describe(plan));
    }

    [Fact]
    public void PruningKeepsTheGoalLiteralsWhoseSupportingStepsAreExecutable()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b2 b3) (on-table b3))");
        SerialPlan plan = SerialPlan.Empty(problem);
        Repair(plan, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"), ("UnmetGoal((on b2 b3))", "Insert(stack b2 b3)"));
        Repair(plan, ("UnsatisfiedPrecondition(step 0, (holding b1))", "Insert(pickup b1)"));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2) 1:(stack b2 b3)", Describe(plan));
        // The second stack fails, but its effects are applied, so every goal literal holds at the end.
        Assert.Equal(["UnsatisfiedPrecondition(step 1, (holding b2))"], Errors(plan.Diagnose()));

        SerialPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal(["(on b1 b2)", "(on-table b3)"], pruned.Goal.Select(literal => literal.ToString()));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2)", Describe(pruned));
        Assert.Equal([(0, "(holding b1)"), (-1, "(on b1 b2)")], pruned.Steps.Select(step => (step.PurposeStepId, step.Purpose.ToString())));
        Assert.Equal(plan.Round, pruned.Round);
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(pruned));
        Assert.Equal(3, plan.Steps.Count);

        // Ids continue after those of the plan that was pruned.
        SerialPlan extended = pruned.WithGoal(problem.Goal);
        Repair(extended, ("UnmetGoal((on b2 b3))", "Insert(stack b2 b3)"));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2) 3:(stack b2 b3)", Describe(extended));
    }

    [Fact]
    public void PruningASolutionKeepsEveryStepAndGoalLiteral()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on-table b3))");
        SerialPlan plan = PickupAndStack(problem).WithGoal(problem.Goal);

        SerialPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal(problem.Goal, pruned.Goal);
        Assert.Equal(Describe(plan), Describe(pruned));
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(pruned));
    }

    [Fact]
    public void PruningAPlanThatAchievesNothingLeavesAnEmptyPlanWithAnEmptyGoal()
    {
        SerialPlan empty = SerialPlan.Empty(BlocksOnTable("(on b1 b2)"));
        SerialPlan failing = empty.Clone();
        Repair(failing, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));
        Assert.Contains("0->-1 (on b1 b2)", Supports(failing.Diagnose()));

        foreach (SerialPlan plan in new[] { empty, failing })
        {
            SerialPlan pruned = plan.PruneToAchievedGoal();

            Assert.Empty(pruned.Goal);
            Assert.Empty(pruned.Steps);
            Assert.Empty(AssertExecutes(pruned));
        }
    }

    [Fact]
    public void WithGoalKeepsTheStepsAndJudgesThemAgainstTheNewGoal()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b2 b3) (on-table b3))");
        SerialPlan plan = PickupAndStack(problem);
        Assert.True(plan.IsSolution());

        SerialPlan relabeled = plan.WithGoal(problem.Goal);

        Assert.Equal(problem.Goal, relabeled.Goal);
        Assert.Equal(Describe(plan), Describe(relabeled));
        Assert.Equal(["UnmetGoal((on b2 b3))"], Errors(relabeled.Diagnose()));
        Assert.True(plan.IsSolution());
        Assert.Throws<ArgumentException>(() => plan.WithGoal(BlocksOnTable("(on b1 b2)").Goal));
    }

    [Fact]
    public void StaticGoalLiteralsAreCheckedAtConstructionAndNeverBecomeErrors()
    {
        SerialPlan plan = SerialPlan.Empty(Drive("(and (road l1 l2) (at t1 l1))"));
        Assert.Empty(AssertExecutes(plan));
        Assert.Equal(["-1->-1 (at t1 l1)"], Supports(plan.Diagnose()));

        Assert.Throws<InvalidOperationException>(() => SerialPlan.Empty(Drive("(road l3 l1)")));
    }

    [Fact]
    public void NumericPlanningAndDerivedPredicatesAreRejectedAtConstruction()
    {
        Domain numeric = Domain.FromText("""
(define (domain counter)
  (:requirements :strips :numeric-fluents)
  (:predicates (done))
  (:functions (count))
  (:action finish :parameters () :precondition (< (count) 3) :effect (and (done) (increase (count) 1))))
""");
        Problem numericProblem = Problem.FromText(numeric, """
(define (problem counter-1)
  (:domain counter)
  (:init (= (count) 0))
  (:goal (done)))
""", ApplicableActionGeneratorType.Grounded);
        Assert.Throws<NotSupportedException>(() => SerialPlan.Empty(numericProblem));

        Domain derived = Domain.FromText("""
(define (domain derived)
  (:requirements :strips :derived-predicates)
  (:predicates (p) (q) (r))
  (:derived (r) (p))
  (:action a :parameters () :precondition (r) :effect (q)))
""");
        Problem derivedProblem = Problem.FromText(derived, """
(define (problem derived-1)
  (:domain derived)
  (:init (p))
  (:goal (q)))
""", ApplicableActionGeneratorType.Grounded);
        Assert.Throws<NotSupportedException>(() => SerialPlan.Empty(derivedProblem));
    }

    public static TheoryData<string, int, int> AcceptanceProblems()
    {
        var problems = new TheoryData<string, int, int>();
        foreach (int balls in new[] { 1, 2, 3, 4 }) problems.Add("gripper", balls, 0);
        foreach (int blocks in new[] { 3, 4, 5 })
        {
            for (int seed = 1; seed <= 5; seed++) problems.Add("blocks", blocks, seed);
        }

        return problems;
    }

    private static Problem AcceptanceProblem(string domain, int size, int seed) => domain == "gripper" ? Gripper(size) : RandomBlocks(size, seed);

    [Theory]
    [MemberData(nameof(AcceptanceProblems))]
    public void SearchSolvesSmallProblemsAndTheOracleRollOutReproducesItsPlan(string domain, int size, int seed)
    {
        Problem problem = AcceptanceProblem(domain, size, seed);
        var watch = Stopwatch.StartNew();
        SerialPlan reference = Solve(problem);
        double seconds = watch.Elapsed.TotalSeconds;
        string[] actions = AssertExecutes(reference);
        Assert.Equal(actions, AssertExecutes(Solve(problem)));

        SerialPlan current = SerialPlan.Empty(problem);
        int rounds = RollOut(current, reference);

        _output.WriteLine($"{domain} {size} seed {seed}: {seconds:F2}s, {actions.Length} steps, {reference.Round} edits, {rounds} oracle rounds");
        Assert.Equal(actions, AssertExecutes(current));
        Assert.Equal(reference.Steps.Select(step => step.Purpose), current.Steps.Select(step => step.Purpose));
    }

    [Fact]
    public void SearchGivesUpWhenItsBudgetIsSpent()
    {
        Problem problem = Gripper(2);

        Assert.Null(SerialSearch.Solve(problem, 3, TimeSpan.FromSeconds(10)));
        Assert.Null(SerialSearch.Solve(problem, int.MaxValue, TimeSpan.Zero));
    }

    [Fact]
    public void OracleDeletesAStepTheReferenceHasNoCounterpartFor()
    {
        Problem problem = BlocksOnTable("(on b1 b2)");
        SerialPlan reference = Solve(problem);
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(reference));
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));
        Repair(current, ("UnsatisfiedPrecondition(step 0, (holding b1))", "Insert(unstack b1 ?)"));
        Assert.Equal("1:(unstack b1 ?) 0:(stack b1 b2)", Describe(current));

        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);

        Assert.Equal(["UnboundParameter(step 1, parameter 1)"], Errors(diagnosis));
        Assert.Equal([0], targets.RepairIndexPerError);
        Assert.Equal([1], targets.StepIdsToDelete);
        Assert.Equal(1, targets.MatchedStepCount);

        RollOut(current, reference);
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleRebindsAParameterThatDiffersFromTheReference()
    {
        Problem problem = Drive("(at t1 l3)");
        SerialPlan reference = Solve(problem);
        Assert.Equal(["(drive t1 l1 l3)"], AssertExecutes(reference));
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((at t1 l3))", "Insert(drive t1 ? l3)"));
        Repair(current, ("UnboundParameter(step 0, parameter 1)", "Bind(1, l4)"));

        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);

        Assert.Equal("Rebind(1, l1)", diagnosis.Errors[0].Repairs[targets.RepairIndexPerError[0]].ToString());
        Assert.Empty(targets.StepIdsToDelete);
        Assert.Equal(1, targets.MatchedStepCount);

        current.Apply(diagnosis, targets.RepairIndexPerError, targets.StepIdsToDelete);
        Assert.Equal(["(drive t1 l1 l3)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleAcceptsABindingThatDiffersFromTheReference()
    {
        Problem problem = Gripper(1);
        SerialPlan reference = Solve(problem);
        Assert.Equal(["(pick ball1 rooma right)", "(move rooma roomb)", "(drop ball1 roomb right)"], AssertExecutes(reference));
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((at ball1 roomb))", "Insert(drop ball1 roomb ?)"));
        Repair(current, ("UnboundParameter(step 0, parameter 2)", "Bind(2, left)"));

        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);

        // Either gripper carries the ball, so the drop stays and the pick of the reference follows its gripper.
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (carry ball1 left))", "UnsatisfiedPrecondition(step 0, (at-robby roomb))"], Errors(diagnosis));
        Assert.Equal(["Insert(pick ball1 ? left)", "Insert(move ? roomb)"], TargetTexts(diagnosis, targets));
        Assert.Empty(targets.StepIdsToDelete);
        Assert.Equal(1, targets.MatchedStepCount);

        RollOut(current, reference);
        Assert.Equal(["(pick ball1 rooma left)", "(move rooma roomb)", "(drop ball1 roomb left)"], AssertExecutes(current));
    }

    [Theory]
    [InlineData(1)]
    [InlineData(2)]
    [InlineData(4)]
    public void OracleRollOutSolvesGripperWithTheOtherGripperAtEveryBind(int balls)
    {
        Problem problem = Gripper(balls);
        SerialPlan reference = Solve(problem);
        SerialPlan current = SerialPlan.Empty(problem);

        bool solved = TryRollOut(current, reference, 4 * reference.Steps.Count, (error, target) =>
        {
            string? gripper = error.Repairs[target].Object?.Name;
            if (gripper != "left" && gripper != "right") return target;
            return error.Repairs.ToList().FindIndex(repair => repair.Object?.Name == (gripper == "left" ? "right" : "left"));
        });

        Assert.True(solved, $"Errors: {string.Join(", ", Errors(current.Diagnose()))}");
        Assert.Equal(
            AssertExecutes(reference).Select(action => action.Replace("left", "LEFT").Replace("right", "left").Replace("LEFT", "right")),
            AssertExecutes(current));
    }

    [Fact]
    public void OracleDeletesAStepThatWasReboundAwayFromItsPurpose()
    {
        Problem problem = Gripper(1);
        SerialPlan reference = Solve(problem);
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((at ball1 roomb))", "Insert(drop ball1 roomb ?)"));
        Repair(current, ("UnboundParameter(step 0, parameter 2)", "Bind(2, right)"), ("UnsatisfiedPrecondition(step 0, (at-robby roomb))", "Rebind(1, rooma)"));
        Assert.Equal("0:(drop ball1 rooma right)", Describe(current));

        SerialOracleTargets targets = SerialOracle.Targets(current, current.Diagnose(), reference);

        // Dropping the ball in rooma no longer puts it in roomb, whatever else is done for the step.
        Assert.Equal([0], targets.StepIdsToDelete);
        Assert.Equal(0, targets.MatchedStepCount);

        RollOut(current, reference);
        Assert.Equal(AssertExecutes(reference), AssertExecutes(current));
    }

    [Fact]
    public void OracleDeletesAStepThatNoRebindBringsToTheWorkingBindingOfTheReference()
    {
        Domain domain = Domain.FromText("""
(define (domain pairs)
  (:requirements :strips)
  (:predicates (linked ?x ?y) (done))
  (:action finish :parameters (?x ?y) :precondition (linked ?x ?y) :effect (done))
  (:action unlink :parameters (?x ?y) :precondition (linked ?x ?y) :effect (not (linked ?x ?y))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem pairs-1)
  (:domain pairs)
  (:objects a b c d)
  (:init (linked a b))
  (:goal (done)))
""");
        SerialPlan reference = Solve(problem);
        Assert.Equal(["(finish a b)"], AssertExecutes(reference));
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((done))", "Insert(finish ? ?)"));
        Repair(current, ("UnboundParameter(step 0, parameter 0)", "Bind(0, c)"), ("UnboundParameter(step 0, parameter 1)", "Bind(1, d)"));

        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);

        // Nothing links c or d to anything, so no single parameter can be changed, while a and b are linked.
        Assert.Equal(["NoOp"], Repairs(diagnosis, "UnsatisfiedPrecondition(step 0, (linked c d))"));
        Assert.Equal([0], targets.RepairIndexPerError);
        Assert.Equal([0], targets.StepIdsToDelete);

        RollOut(current, reference);
        Assert.Equal(["(finish a b)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleDeletesTheStepWhoseBindingLeavesTheStepInsertedForItWithoutAWorkingOne()
    {
        Domain domain = Domain.FromText("""
(define (domain prepare-and-finish)
  (:requirements :strips)
  (:predicates (fresh ?x) (ready ?x) (done))
  (:action prepare :parameters (?x) :precondition (fresh ?x) :effect (and (ready ?x) (not (fresh ?x))))
  (:action finish :parameters (?x) :precondition (ready ?x) :effect (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem prepare-and-finish-1)
  (:domain prepare-and-finish)
  (:objects a b)
  (:init (fresh a))
  (:goal (done)))
""");
        SerialPlan reference = Solve(problem);
        Assert.Equal(["(prepare a)", "(finish a)"], AssertExecutes(reference));
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((done))", "Insert(finish ?)"));
        Repair(current, ("UnboundParameter(step 0, parameter 0)", "Bind(0, b)"));

        // Finishing b is as good as finishing a until b turns out not to be fresh.
        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["Insert(prepare b)"], TargetTexts(diagnosis, targets));
        Assert.Empty(targets.StepIdsToDelete);
        current.Apply(diagnosis, targets.RepairIndexPerError, targets.StepIdsToDelete);

        // Preparing a instead would not make b ready, and a new step for b would be the same one again.
        diagnosis = current.Diagnose();
        targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["NoOp", "Rebind(0, a)"], Repairs(diagnosis, "UnsatisfiedPrecondition(step 1, (fresh b))"));
        Assert.Equal(["NoOp"], TargetTexts(diagnosis, targets));
        Assert.Equal([1, 0], targets.StepIdsToDelete);

        RollOut(current, reference);
        Assert.Equal(["(prepare a)", "(finish a)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleDeletesASettledStepWhoseBindingBreaksWhatTheReferenceGetsFromIt()
    {
        Problem problem = HoldingFirstBlock("(and (arm-empty) (on b1 b2))");
        SerialPlan reference = Solve(problem);
        Assert.Equal(["(stack b1 b2)"], AssertExecutes(reference));
        Assert.Equal("(arm-empty)", reference.Steps.Single().Purpose.ToString());
        SerialPlan current = SerialPlan.Empty(problem);

        // No step of the reference was inserted for (on b1 b2), which its stack achieves along the way.
        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["UnmetGoal((arm-empty))", "UnmetGoal((on b1 b2))"], Errors(diagnosis));
        Assert.Equal(["Insert(stack ? ?)", "NoOp"], TargetTexts(diagnosis, targets));

        Repair(current, ("UnmetGoal((arm-empty))", "Insert(stack ? ?)"));
        Repair(current, ("UnboundParameter(step 0, parameter 0)", "Bind(0, b1)"), ("UnboundParameter(step 0, parameter 1)", "Bind(1, b3)"));
        diagnosis = current.Diagnose();
        targets = SerialOracle.Targets(current, diagnosis, reference);

        // Stacking b1 on b3 empties the arm as well, but then nothing of the reference puts b1 on b2.
        Assert.Equal(["UnmetGoal((on b1 b2))"], Errors(diagnosis));
        Assert.Equal(["NoOp"], TargetTexts(diagnosis, targets));
        Assert.Equal([0], targets.StepIdsToDelete);

        RollOut(current, reference);
        Assert.Equal(["(stack b1 b2)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleAsksForAStepWhosePurposeWasPrunedWhereTheReferenceDependsOnIt()
    {
        Problem problem = Problem.FromText(Blocksworld(), """
(define (problem tower)
  (:domain blocksworld)
  (:objects b1 b2 b3)
  (:init (arm-empty) (on b1 b2) (on-table b2) (on-table b3) (clear b1) (clear b3))
  (:goal (and (on-table b1) (on b3 b2))))
""");
        // The unstack is inserted for the stack, which never gets to hold b3, and the putdown lives off it.
        SerialPlan policy = SerialPlan.Empty(problem);
        Repair(policy, ("UnmetGoal((on b3 b2))", "Insert(stack b3 b2)"));
        Repair(policy, ("UnsatisfiedPrecondition(step 0, (clear b2))", "Insert(unstack ? b2)"), ("UnmetGoal((on-table b1))", "Insert(putdown b1)"));
        Repair(policy, ("UnboundParameter(step 1, parameter 0)", "Bind(0, b1)"));
        SerialPlan reference = policy.PruneToAchievedGoal();
        Assert.Equal("1:(unstack b1 b2) 2:(putdown b1)", Describe(reference));
        Assert.Equal([(0, "(clear b2)"), (-1, "(on-table b1)")], reference.Steps.Select(step => (step.PurposeStepId, step.Purpose.ToString())));

        SerialPlan current = SerialPlan.Empty(problem, reference.Goal);
        Repair(current, ("UnmetGoal((on-table b1))", "Insert(putdown b1)"));
        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (holding b1))"], Errors(diagnosis));
        Assert.Equal(["Insert(unstack b1 ?)"], TargetTexts(diagnosis, targets));

        RollOut(current, reference);
        Assert.Equal(["(unstack b1 b2)", "(putdown b1)"], AssertExecutes(current));
        Assert.Equal(2, SerialOracle.Targets(current, current.Diagnose(), reference).MatchedStepCount);
    }

    [Fact]
    public void OracleAsksForAStepWhosePurposeWasPrunedForTheGoalLiteralItAchieves()
    {
        Problem problem = Gripper(1);
        SerialPlan solved = Solve(problem);
        Literal<Fact> robbyInRoomB = solved.Diagnose().Supports.First(support => support.Literal.ToString() == "(at-robby roomb)").Literal;
        // The move was inserted for the drop, which the relabeled goal has no use for.
        SerialPlan reference = solved.WithGoal([robbyInRoomB]).PruneToAchievedGoal();
        Assert.Equal(["(move rooma roomb)"], AssertExecutes(reference));
        Assert.DoesNotContain(reference.Steps.Single().PurposeStepId, reference.Steps.Select(step => step.Id).Append(-1));

        SerialPlan current = SerialPlan.Empty(problem, reference.Goal);
        SerialDiagnosis diagnosis = current.Diagnose();
        Assert.Equal(["Insert(move ? roomb)"], TargetTexts(diagnosis, SerialOracle.Targets(current, diagnosis, reference)));

        RollOut(current, reference);
        Assert.Equal(["(move rooma roomb)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleWaitsWithAnInsertUntilTheStepsBeforeItInTheReferenceExist()
    {
        Problem problem = Gripper(1);
        SerialPlan reference = Solve(problem);
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((at ball1 roomb))", "Insert(drop ball1 roomb ?)"));

        // The move would be right for (at-robby roomb), but the pick of the reference comes before it and
        // can only be asked for once the gripper is bound; a step inserted later would land after the move.
        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["UnboundParameter(step 0, parameter 2)", "UnsatisfiedPrecondition(step 0, (at-robby roomb))"], Errors(diagnosis));
        Assert.Equal(["Bind(2, right)", "NoOp"], TargetTexts(diagnosis, targets));

        current.Apply(diagnosis, targets.RepairIndexPerError, targets.StepIdsToDelete);
        diagnosis = current.Diagnose();
        targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["Insert(pick ball1 ? right)", "Insert(move ? roomb)"], TargetTexts(diagnosis, targets));
    }

    [Fact]
    public void OracleDeletesAStepThatWasInsertedBeforeAnEarlierStepOfTheReference()
    {
        Problem problem = Gripper(1);
        SerialPlan reference = Solve(problem);
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((at ball1 roomb))", "Insert(drop ball1 roomb ?)"));
        Repair(current, ("UnboundParameter(step 0, parameter 2)", "Bind(2, right)"), ("UnsatisfiedPrecondition(step 0, (at-robby roomb))", "Insert(move ? roomb)"));
        Repair(current, ("UnboundParameter(step 1, parameter 0)", "Bind(0, rooma)"), ("UnsatisfiedPrecondition(step 0, (carry ball1 right))", "Insert(pick ball1 ? right)"));
        Assert.Equal("1:(move rooma roomb) 2:(pick ball1 ? right) 0:(drop ball1 roomb right)", Describe(current));

        // The reference picks before it moves, and a move can only be inserted after the pick once this one is gone.
        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal([1], targets.StepIdsToDelete);
        Assert.Equal(2, targets.MatchedStepCount);

        RollOut(current, reference);
        Assert.Equal(["(pick ball1 rooma right)", "(move rooma roomb)", "(drop ball1 roomb right)"], AssertExecutes(current));
    }

    private static Problem FourBlocksOnTable(string goal) => Problem.FromText(Blocksworld(), $"""
(define (problem four-blocks-on-table)
  (:domain blocksworld)
  (:objects b1 b2 b3 b4)
  (:init (arm-empty) (on-table b1) (on-table b2) (on-table b3) (on-table b4) (clear b1) (clear b2) (clear b3) (clear b4))
  (:goal {goal}))
""");

    // Builds the second tower of the goal first.
    private static SerialPlan SecondTowerFirst(Problem problem)
    {
        SerialPlan reference = SerialPlan.Empty(problem);
        Repair(reference, ("UnmetGoal((on b3 b4))", "Insert(stack b3 b4)"));
        Repair(reference, ("UnsatisfiedPrecondition(step 0, (holding b3))", "Insert(pickup b3)"));
        Repair(reference, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));
        Repair(reference, ("UnsatisfiedPrecondition(step 2, (holding b1))", "Insert(pickup b1)"));
        Assert.Equal("1:(pickup b3) 0:(stack b3 b4) 3:(pickup b1) 2:(stack b1 b2)", Describe(reference));
        return reference;
    }

    [Fact]
    public void OracleAsksForEveryGoalThatCanBeWorkedOnNextWhateverTheOrderOfTheReference()
    {
        Problem problem = FourBlocksOnTable("(and (on b1 b2) (on b3 b4))");
        SerialPlan reference = SecondTowerFirst(problem);
        SerialPlan current = SerialPlan.Empty(problem);

        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["UnmetGoal((on b1 b2))", "UnmetGoal((on b3 b4))"], Errors(diagnosis));
        Assert.Equal(["Insert(stack b1 b2)", "Insert(stack b3 b4)"], TargetTexts(diagnosis, targets));

        RollOut(current, reference);
        Assert.Equal(["(pickup b1)", "(stack b1 b2)", "(pickup b3)", "(stack b3 b4)"], AssertExecutes(current));
    }

    [Fact]
    public void OracleKeepsGoalStepsThatWereInsertedInAnotherOrderThanTheReference()
    {
        Problem problem = FourBlocksOnTable("(and (on b1 b2) (on b3 b4))");
        SerialPlan reference = SecondTowerFirst(problem);
        SerialPlan current = SerialPlan.Empty(problem);
        Repair(current, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));

        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Empty(targets.StepIdsToDelete);
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (holding b1))", "UnmetGoal((on b3 b4))"], Errors(diagnosis));
        Assert.Equal(["Insert(pickup b1)", "Insert(stack b3 b4)"], TargetTexts(diagnosis, targets));
    }

    [Fact]
    public void OracleWaitsWithAGoalWhoseStepsWouldBreakThePlanIfTheyCameNext()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b2 b3))");
        SerialPlan reference = SerialPlan.Empty(problem);
        Repair(reference, ("UnmetGoal((on b2 b3))", "Insert(stack b2 b3)"));
        Repair(reference, ("UnsatisfiedPrecondition(step 0, (holding b2))", "Insert(pickup b2)"));
        Repair(reference, ("UnmetGoal((on b1 b2))", "Insert(stack b1 b2)"));
        Repair(reference, ("UnsatisfiedPrecondition(step 2, (holding b1))", "Insert(pickup b1)"));
        Assert.Equal(["(pickup b2)", "(stack b2 b3)", "(pickup b1)", "(stack b1 b2)"], AssertExecutes(reference));
        SerialPlan current = SerialPlan.Empty(problem);

        // Stacking b1 on b2 first would leave b2 covered when it has to be picked up.
        SerialDiagnosis diagnosis = current.Diagnose();
        SerialOracleTargets targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["UnmetGoal((on b1 b2))", "UnmetGoal((on b2 b3))"], Errors(diagnosis));
        Assert.Equal(["NoOp", "Insert(stack b2 b3)"], TargetTexts(diagnosis, targets));

        // The step for the lower block exists after one round, so the upper block is next.
        current.Apply(diagnosis, targets.RepairIndexPerError, targets.StepIdsToDelete);
        diagnosis = current.Diagnose();
        targets = SerialOracle.Targets(current, diagnosis, reference);
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (holding b2))", "UnmetGoal((on b1 b2))"], Errors(diagnosis));
        Assert.Equal(["Insert(pickup b2)", "Insert(stack b1 b2)"], TargetTexts(diagnosis, targets));
    }

    public static TheoryData<string, int, int> PerturbedProblems() => new()
    {
        { "gripper", 2, 0 }, { "gripper", 3, 0 }, { "blocks", 4, 1 }, { "blocks", 4, 3 }, { "blocks", 5, 1 }, { "blocks", 5, 3 }
    };

    [Theory]
    [MemberData(nameof(PerturbedProblems))]
    public void OracleLeadsBackToASolutionFromPlansEditedAtRandom(string domain, int size, int seed)
    {
        Problem problem = AcceptanceProblem(domain, size, seed);
        SerialPlan reference = Solve(problem);
        for (int trial = 0; trial < 20; trial++)
        {
            SerialPlan current = SerialPlan.Empty(problem);
            EditAtRandom(current, reference, new Random(trial), 6);

            RollOut(current, reference);

            // The objects may differ from the reference where others do as well, and the goals may be worked on
            // in another order; the actions may not differ.
            AssertExecutes(current);
            Assert.Equal(reference.Steps.Select(step => step.Schema.Name).Order(), current.Steps.Select(step => step.Schema.Name).Order());
        }
    }

    public static TheoryData<string, int> RandomBindProblems()
    {
        var problems = new TheoryData<string, int>();
        foreach (int balls in new[] { 1, 2, 3, 4, 5, 6 }) problems.Add("gripper", balls);
        foreach (int blocks in new[] { 3, 4, 5, 6 }) problems.Add("blocks", blocks);
        return problems;
    }

    [Theory]
    [MemberData(nameof(RandomBindProblems))]
    public void OracleRollOutSolvesMostProblemsWhenEveryParameterIsBoundAtRandom(string domain, int size)
    {
        int[] seeds = domain == "gripper" ? [0] : [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        int solved = 0;
        int total = 0;
        foreach (int seed in seeds)
        {
            Problem problem = AcceptanceProblem(domain, size, seed);
            SerialPlan reference = Solve(problem);
            for (int trial = 0; trial < 200 / seeds.Length; trial++)
            {
                var random = new Random(trial);
                SerialPlan current = SerialPlan.Empty(problem);
                total++;
                if (!TryRollOut(current, reference, 4 * reference.Steps.Count, (error, _) => error.Repairs.Count > 1 ? random.Next(1, error.Repairs.Count) : 0))
                {
                    _output.WriteLine($"seed {seed} trial {trial}: {Describe(current)}; errors: {string.Join(", ", Errors(current.Diagnose()))}");
                    continue;
                }

                AssertExecutes(current);
                solved++;
            }
        }

        _output.WriteLine($"{domain} {size}: {solved} of {total} roll-outs solved within four rounds per reference step");
        Assert.True(solved >= 0.95 * total, $"Only {solved} of {total} roll-outs were solved.");
    }

    // The references are what is left of plans edited at random once they are pruned to the goal they achieve.
    [Theory]
    [InlineData("gripper", 3, 0)]
    [InlineData("gripper", 4, 0)]
    [InlineData("blocks", 4, 1)]
    [InlineData("blocks", 5, 2)]
    [InlineData("blocks", 5, 3)]
    [InlineData("blocks", 6, 1)]
    [InlineData("blocks", 6, 3)]
    public void OracleRollOutSolvesMostHindsightReferences(string domain, int size, int seed)
    {
        Problem problem = AcceptanceProblem(domain, size, seed);
        SerialPlan guide = Solve(problem);
        int solved = 0;
        int total = 0;
        int withPrunedPurpose = 0;
        for (int trial = 0; trial < 200; trial++)
        {
            var random = new Random(trial);
            SerialPlan policy = SerialPlan.Empty(problem);
            EditAtRandom(policy, guide, random, random.Next(3, 2 * guide.Steps.Count + 4));
            SerialPlan reference = policy.PruneToAchievedGoal();
            if (reference.Steps.Count == 0) continue;

            total++;
            IEnumerable<int> parents = reference.Steps.Select(step => step.Id).Append(-1);
            if (reference.Steps.Any(step => !parents.Contains(step.PurposeStepId) || (step.PurposeStepId == -1 && !reference.Goal.Contains(step.Purpose)))) withPrunedPurpose++;
            SerialPlan current = SerialPlan.Empty(problem, reference.Goal);
            if (!TryRollOut(current, reference, 4 * reference.Steps.Count + 10, (_, target) => target))
            {
                _output.WriteLine($"trial {trial}: reference {Describe(reference)}; stuck at {Describe(current)}; errors: {string.Join(", ", Errors(current.Diagnose()))}");
                continue;
            }

            AssertExecutes(current);
            solved++;
        }

        _output.WriteLine($"{domain} {size} seed {seed}: {solved} of {total} roll-outs solved, {withPrunedPurpose} references have a step whose purpose was pruned");
        Assert.True(solved >= 0.9 * total, $"Only {solved} of {total} roll-outs were solved.");
    }

    [Fact]
    public void OracleRejectsAReferenceThatIsNotASolution()
    {
        Problem problem = BlocksOnTable("(on b1 b2)");
        SerialPlan current = SerialPlan.Empty(problem);

        Assert.Throws<ArgumentException>(() => SerialOracle.Targets(current, current.Diagnose(), SerialPlan.Empty(problem)));
        Assert.Throws<ArgumentException>(() => SerialOracle.Targets(current, current.Diagnose(), Solve(BlocksOnTable("(on b1 b2)"))));
    }
}
