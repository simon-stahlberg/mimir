using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace.Tests;

public class SerialPlanTests
{
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

    private static string Describe(SerialPlan plan) => string.Join(" ", plan.Steps);

    private static string[] Errors(SerialDiagnosis diagnosis) => diagnosis.Errors.Select(error => error.ToString()).ToArray();

    private static string[] Supports(SerialDiagnosis diagnosis) => diagnosis.Supports.Select(support => support.ToString()).ToArray();

    private static string[] Candidates(SerialDiagnosis diagnosis, IEnumerable<int> indices)
        => indices.Select(index => diagnosis.Candidates[index].ToString()).ToArray();

    private static string[] Candidates(SerialDiagnosis diagnosis, string error)
        => Candidates(diagnosis, diagnosis.Errors.Single(candidate => candidate.ToString() == error).CandidateIndices);

    private static Constant Named(Problem problem, string name) => problem.AllObjects.Single(candidate => candidate.Name == name);

    // The edits of one round, which change nothing until they are applied.
    private sealed class Edit
    {
        private readonly SerialPlan _plan;
        private readonly List<SerialInsert> _inserts = new();
        private readonly List<int> _deleted = new();
        private readonly List<SerialMove> _moves = new();
        private readonly List<SerialRebind> _rebinds = new();

        public Edit(SerialPlan plan) => _plan = plan;

        // Inserts an action such as "(stack b1 b2)" at the point.
        public Edit Insert(int point, string action)
        {
            string[] words = action.Trim('(', ')').Split(' ');
            ActionSchema schema = _plan.Problem.Domain.Actions.Single(candidate => candidate.Name == words[0]);
            _inserts.Add(new SerialInsert(point, schema, words.Skip(1).Select(name => Named(_plan.Problem, name)).ToArray()));
            return this;
        }

        public Edit Delete(int stepId)
        {
            _deleted.Add(stepId);
            return this;
        }

        public Edit Move(int stepId, int point)
        {
            _moves.Add(new SerialMove(stepId, point));
            return this;
        }

        // Gives one parameter of the step another object.
        public Edit Assign(int stepId, int parameterIndex, string objectName)
        {
            Constant[] binding = _plan.Steps.Single(step => step.Id == stepId).Binding.ToArray();
            binding[parameterIndex] = Named(_plan.Problem, objectName);
            _rebinds.Add(new SerialRebind(stepId, binding));
            return this;
        }

        public void Apply()
            => _plan.Apply(new SerialEdits { Inserts = _inserts, DeletedStepIds = _deleted, Moves = _moves, Rebinds = _rebinds });
    }

    // Inserts the actions one round after the other, each at the given point.
    private static SerialPlan Build(SerialPlan plan, params (int Point, string Action)[] inserts)
    {
        foreach ((int point, string action) in inserts) new Edit(plan).Insert(point, action).Apply();
        return plan;
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

    [Fact]
    public void EmptyPlanHasOneUnmetGoalPerUnsatisfiedGoalLiteral()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b2 b3) (on-table b3))");
        SerialPlan plan = SerialPlan.Empty(problem);

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(["UnmetGoal((on b1 b2))", "UnmetGoal((on b2 b3))"], Errors(diagnosis));
        Assert.All(diagnosis.Errors, error => Assert.Equal((SerialErrorKind.UnmetGoal, -1), (error.Kind, error.StepId)));
        Assert.Equal(["(stack b1 b2)"], Candidates(diagnosis, "UnmetGoal((on b1 b2))"));
        Assert.Equal(["(stack b2 b3)"], Candidates(diagnosis, "UnmetGoal((on b2 b3))"));
        Assert.Equal(["(stack b1 b2)", "(stack b2 b3)"], diagnosis.Candidates.Select(candidate => candidate.ToString()));
        Assert.Equal(["-1->-1 (on-table b3)"], Supports(diagnosis));
        Assert.Empty(diagnosis.UnusedStepIds);
        Assert.Equal(problem.InitialState.GetTrueFacts().ToHashSet(), diagnosis.FinalTrueFacts.ToHashSet());
        Assert.False(diagnosis.IsSolution);
        Assert.False(plan.IsSolution());
        Assert.Throws<InvalidOperationException>(() => plan.ToActions());
    }

    [Fact]
    public void AFailingStepStillAppliesItsEffects()
    {
        SerialPlan plan = Build(SerialPlan.Empty(BlocksOnTable("(and (on b1 b2) (holding b1))")), (0, "(stack b1 b2)"), (1, "(unstack b1 b2)"));
        Assert.Equal("0:(stack b1 b2) 1:(unstack b1 b2)", Describe(plan));

        SerialDiagnosis diagnosis = plan.Diagnose();

        // Nothing holds b1 for the stack, yet the unstack after it finds b1 on b2 and takes it off again.
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (holding b1))", "UnmetGoal((on b1 b2))"], Errors(diagnosis));
        Assert.Equal(
            ["-1->0 (clear b2)", "0->1 (on b1 b2)", "0->1 (clear b1)", "0->1 (arm-empty)", "1->-1 (holding b1)"],
            Supports(diagnosis));
    }

    [Fact]
    public void ACandidateIsBoundWhereTheLiteralItAchievesBindsIt()
    {
        SerialPlan plan = SerialPlan.Empty(HoldingFirstBlock("(and (clear b1) (holding b2))"));

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(["(putdown b1)", "(stack b1 ?)", "(unstack ? b1)"], Candidates(diagnosis, "UnmetGoal((clear b1))"));
        Assert.Equal(["(pickup b2)", "(unstack b2 ?)"], Candidates(diagnosis, "UnmetGoal((holding b2))"));
        Assert.Equal(["b1", "b2", "b3"], plan.ParameterOptions(diagnosis.Candidates[1].Schema, 1).Select(value => value.Name));

        // The free argument is chosen when the candidate is inserted, so every step of a plan is ground.
        new Edit(plan).Insert(0, "(stack b1 b3)").Apply();
        Assert.Equal("0:(stack b1 b3)", Describe(plan));
        Assert.Equal(["UnmetGoal((holding b2))"], Errors(plan.Diagnose()));
    }

    [Fact]
    public void AViolatedNegativePreconditionOffersTheActionsThatDeleteTheFact()
    {
        SerialPlan plan = Build(SerialPlan.Empty(Doors("(open d1)")), (0, "(open-door d1)"));

        SerialDiagnosis diagnosis = plan.Diagnose();
        Assert.Equal(["UnsatisfiedPrecondition(step 0, (not (locked d1)))"], Errors(diagnosis));
        Assert.Equal(["(unlock d1)"], Candidates(diagnosis, "UnsatisfiedPrecondition(step 0, (not (locked d1)))"));
        Assert.Contains("-1->0 (not (open d1))", Supports(diagnosis));

        new Edit(plan).Insert(0, "(unlock d1)").Apply();

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

        new Edit(plan).Insert(0, "(unlock d1)").Apply();

        Assert.Equal(["(unlock d1)"], AssertExecutes(plan));
    }

    [Fact]
    public void AFalseStaticPreconditionIsAnErrorWithoutCandidates()
    {
        // There is no road from l3 to l2, and a truck cannot drive from a location to itself.
        SerialPlan plan = Build(SerialPlan.Empty(Drive("(and (at t1 l2) (at t2 l3))")), (0, "(drive t2 l3 l3)"), (0, "(drive t1 l3 l2)"));
        Assert.Equal("1:(drive t1 l3 l2) 0:(drive t2 l3 l3)", Describe(plan));

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(
            [
                "UnsatisfiedPrecondition(step 1, (at t1 l3))",
                "UnsatisfiedPrecondition(step 1, (road l3 l2))",
                "UnsatisfiedPrecondition(step 0, (at t2 l3))",
                "UnsatisfiedPrecondition(step 0, (road l3 l3))",
                "UnsatisfiedPrecondition(step 0, (not (= l3 l3)))",
            ],
            Errors(diagnosis));
        Assert.Empty(Candidates(diagnosis, "UnsatisfiedPrecondition(step 1, (road l3 l2))"));
        Assert.Empty(Candidates(diagnosis, "UnsatisfiedPrecondition(step 0, (not (= l3 l3)))"));

        // Only other objects repair a static precondition.
        new Edit(plan).Assign(1, 1, "l1").Assign(0, 1, "l2").Apply();
        Assert.Equal(["(drive t1 l1 l2)", "(drive t2 l2 l3)"], AssertExecutes(plan));
    }

    [Fact]
    public void SupportsNameTheLastStepThatSetTheFactAndUnusedStepsProduceNone()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on-table b3))");
        SerialPlan plan = Build(SerialPlan.Empty(problem), (0, "(stack b1 b2)"), (0, "(pickup b1)"));
        Assert.Equal("1:(pickup b1) 0:(stack b1 b2)", Describe(plan));

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(
            ["-1->1 (clear b1)", "-1->1 (on-table b1)", "-1->1 (arm-empty)", "-1->0 (clear b2)", "1->0 (holding b1)", "0->-1 (on b1 b2)", "-1->-1 (on-table b3)"],
            Supports(diagnosis));
        Assert.Empty(diagnosis.UnusedStepIds);
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(plan));

        // Without the goal on (on b1 b2) nothing consumes what the stack produces.
        SerialPlan relabeled = plan.WithGoal([problem.Goal.Single(literal => literal.Value.ToString() == "(on-table b3)")]);
        Assert.Equal([0], relabeled.Diagnose().UnusedStepIds);
        Assert.True(relabeled.IsSolution());
    }

    [Fact]
    public void TheNewPlanIsReadOffPointByPoint()
    {
        SerialPlan plan = Build(SerialPlan.Empty(Gripper(3)), (0, "(drop ball1 roomb left)"), (1, "(drop ball2 roomb left)"), (2, "(drop ball3 roomb right)"));
        Assert.Equal("0:(drop ball1 roomb left) 1:(drop ball2 roomb left) 2:(drop ball3 roomb right)", Describe(plan));

        // In one round: a step enters before the first step and one before the last, the first step gets the
        // other gripper, the second is deleted and the third goes to the front.
        new Edit(plan)
            .Insert(0, "(pick ball1 rooma left)")
            .Insert(2, "(move rooma roomb)")
            .Assign(0, 2, "right")
            .Delete(1)
            .Move(2, 0)
            .Apply();

        // At the first point the inserted step enters before the moved one.
        Assert.Equal("3:(pick ball1 rooma left) 2:(drop ball3 roomb right) 0:(drop ball1 roomb right) 4:(move rooma roomb)", Describe(plan));
    }

    [Fact]
    public void StepsThatEnterTheSamePointKeepTheOrderTheyWereGivenOrHad()
    {
        SerialPlan plan = Build(SerialPlan.Empty(Gripper(3)), (0, "(drop ball1 roomb left)"), (1, "(drop ball2 roomb left)"), (2, "(drop ball3 roomb right)"));

        // Both later steps go to the front, whichever is named first. A step that moves keeps a new binding.
        new Edit(plan).Move(2, 0).Move(1, 0).Assign(1, 2, "right").Apply();
        Assert.Equal("1:(drop ball2 roomb right) 2:(drop ball3 roomb right) 0:(drop ball1 roomb left)", Describe(plan));

        // The points on both sides of a step are where it already is.
        new Edit(plan).Move(1, 1).Move(0, 3).Apply();
        Assert.Equal("1:(drop ball2 roomb right) 2:(drop ball3 roomb right) 0:(drop ball1 roomb left)", Describe(plan));

        // Inserted steps enter in the order they are given.
        new Edit(plan).Insert(3, "(move rooma roomb)").Insert(3, "(move roomb rooma)").Delete(2).Apply();
        Assert.Equal("1:(drop ball2 roomb right) 0:(drop ball1 roomb left) 3:(move rooma roomb) 4:(move roomb rooma)", Describe(plan));
    }

    [Fact]
    public void ApplyRejectsEditsThatDoNotFitThePlanAndLeavesItUnchanged()
    {
        Problem problem = Drive("(at t1 l3)");
        SerialPlan plan = Build(SerialPlan.Empty(problem), (0, "(drive t1 l1 l3)"));

        // A step that does not exist, a point that does not exist.
        Assert.Throws<ArgumentException>(() => new Edit(plan).Delete(7).Apply());
        Assert.Throws<ArgumentOutOfRangeException>(() => new Edit(plan).Move(0, 2).Apply());
        Assert.Throws<ArgumentOutOfRangeException>(() => new Edit(plan).Insert(2, "(drive t1 l1 l2)").Apply());
        // A step leaves its place once.
        Assert.Throws<ArgumentException>(() => new Edit(plan).Delete(0).Move(0, 1).Apply());
        Assert.Throws<ArgumentException>(() => new Edit(plan).Move(0, 0).Move(0, 1).Apply());
        Assert.Throws<ArgumentException>(() => new Edit(plan).Assign(0, 1, "l2").Assign(0, 2, "l2").Apply());
        // A truck where a location belongs, and an argument too few.
        Assert.Throws<ArgumentException>(() => new Edit(plan).Assign(0, 1, "t2").Apply());
        Assert.Throws<ArgumentException>(() => new Edit(plan).Insert(0, "(drive t1 t2 l3)").Apply());
        Assert.Throws<ArgumentException>(() => new Edit(plan).Insert(0, "(drive t1 l1)").Apply());

        Assert.Equal("0:(drive t1 l1 l3)", Describe(plan));
        // The id of a step that was not inserted is not used up.
        Assert.Equal("0:(drive t1 l1 l3) 1:(drive t1 l1 l2)", Describe(Build(plan, (1, "(drive t1 l1 l2)"))));
    }

    [Fact]
    public void AnErrorNamesTheStepThatLastMadeItsLiteralFalse()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b3 b1))");
        // The second pickup needs an empty arm, which the first pickup took; the stack needs b2, which nobody took.
        SerialPlan plan = Build(SerialPlan.Empty(problem), (0, "(pickup b1)"), (1, "(pickup b3)"), (2, "(stack b2 b1)"));

        SerialDiagnosis diagnosis = plan.Diagnose();

        Assert.Equal(
            ["UnsatisfiedPrecondition(step 1, (arm-empty)) by 0", "UnsatisfiedPrecondition(step 2, (clear b1)) by 0", "UnsatisfiedPrecondition(step 2, (holding b2)) by -1"],
            diagnosis.Errors.Where(error => error.Kind == SerialErrorKind.UnsatisfiedPrecondition).Select(error => $"{error} by {error.CulpritStepId}"));
    }

    [Fact]
    public void TwoStepsOfOneSchemaListTheirPreconditionsInTheSameOrder()
    {
        SerialPlan plan = Build(SerialPlan.Empty(Drive("(at t1 l3)")), (0, "(drive t1 l1 l3)"), (1, "(drive t2 l3 l3)"));

        // The fluent preconditions come first, then the static ones.
        Assert.Equal(["(at t1 l1)", "(road l1 l3)", "(not (= l1 l3))"], plan.Steps[0].Preconditions.Select(literal => literal.ToString()));
        Assert.Equal(["(at t2 l3)", "(road l3 l3)", "(not (= l3 l3))"], plan.Steps[1].Preconditions.Select(literal => literal.ToString()));
        Assert.Equal(["(at t1 l3)", "(not (at t1 l1))"], plan.Steps[0].Effects.Select(literal => literal.ToString()).Order());
    }

    [Fact]
    public void PruningKeepsTheGoalLiteralsWhoseSupportingStepsAreExecutable()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on b2 b3) (on-table b3))");
        SerialPlan plan = Build(SerialPlan.Empty(problem), (0, "(stack b1 b2)"), (1, "(stack b2 b3)"), (0, "(pickup b1)"));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2) 1:(stack b2 b3)", Describe(plan));
        // The second stack fails, but its effects are applied, so every goal literal holds at the end.
        Assert.Equal(["UnsatisfiedPrecondition(step 1, (holding b2))"], Errors(plan.Diagnose()));

        SerialPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal(["(on b1 b2)", "(on-table b3)"], pruned.Goal.Select(literal => literal.ToString()));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2)", Describe(pruned));
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(pruned));
        Assert.Equal(3, plan.Steps.Count);

        // Ids continue after those of the plan that was pruned.
        SerialPlan extended = Build(pruned.WithGoal(problem.Goal), (2, "(stack b2 b3)"));
        Assert.Equal("2:(pickup b1) 0:(stack b1 b2) 3:(stack b2 b3)", Describe(extended));
    }

    [Fact]
    public void PruningASolutionKeepsEveryStepAndGoalLiteral()
    {
        Problem problem = BlocksOnTable("(and (on b1 b2) (on-table b3))");
        SerialPlan plan = Build(SerialPlan.Empty(problem), (0, "(stack b1 b2)"), (0, "(pickup b1)"));

        SerialPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal(problem.Goal, pruned.Goal);
        Assert.Equal(Describe(plan), Describe(pruned));
        Assert.Equal(["(pickup b1)", "(stack b1 b2)"], AssertExecutes(pruned));
    }

    [Fact]
    public void PruningAPlanThatAchievesNothingLeavesAnEmptyPlanWithAnEmptyGoal()
    {
        SerialPlan empty = SerialPlan.Empty(BlocksOnTable("(on b1 b2)"));
        SerialPlan failing = Build(empty.Clone(), (0, "(stack b1 b2)"));
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
        SerialPlan plan = SerialPlan.Empty(problem, [problem.Goal.Single(literal => literal.Value.ToString() == "(on b1 b2)")]);
        Build(plan, (0, "(stack b1 b2)"), (0, "(pickup b1)"));
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
    public void NumericPlanningDerivedPredicatesAndConditionalEffectsAreRejectedAtConstruction()
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

        Domain conditional = Domain.FromText("""
(define (domain lamp)
  (:requirements :strips :conditional-effects)
  (:predicates (powered ?r) (lit ?r))
  (:action press :parameters (?r) :effect (when (powered ?r) (lit ?r))))
""");
        Problem conditionalProblem = Problem.FromText(conditional, """
(define (problem lamp-1)
  (:domain lamp)
  (:objects r1)
  (:init (powered r1))
  (:goal (lit r1)))
""");
        Assert.Throws<NotSupportedException>(() => SerialPlan.Empty(conditionalProblem));
    }
}
