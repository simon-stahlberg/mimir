using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Mimir.Search.Space;
using Xunit;
using XunitFact = Xunit.FactAttribute;

namespace Mimir.Search.Tests;

public sealed class H2HeuristicTests
{
    /// <summary>
    /// Both facts are reachable on their own, but every achiever destroys the fact the
    /// other one needs. h^1 cannot see it; h^2 must, which is what the delete guard buys.
    /// </summary>
    [XunitFact]
    public void CompetingAchievers_AreUnreachableAsAPair()
    {
        Problem problem = CreateProblem(
            """
(define (domain competing)
  (:requirements :strips)
  (:predicates (i) (p) (q))
  (:action make-p :parameters () :precondition (i) :effect (and (p) (not (i))))
  (:action make-q :parameters () :precondition (i) :effect (and (q) (not (i)))))
""",
            """
(define (problem competing-problem)
  (:domain competing)
  (:init (i))
  (:goal (and (p) (q))))
""");

        AssertUnsolvable(problem);
        Assert.Equal(1d, MaxValue(problem));
        Assert.Equal(double.PositiveInfinity, H2Value(problem));
    }

    /// <summary>
    /// A negative precondition is only satisfied after the fluent has been deleted, so
    /// h^2 must count the delete. h^max drops negative preconditions and misses it.
    /// </summary>
    [XunitFact]
    public void NegativePrecondition_CountsTheDeleteThatEstablishesIt()
    {
        Problem problem = CreateProblem(
            """
(define (domain negative-precondition)
  (:requirements :strips :negative-preconditions)
  (:predicates (p) (g))
  (:action clear :parameters () :precondition () :effect (not (p)))
  (:action finish :parameters () :precondition (not (p)) :effect (g)))
""",
            NegativePreconditionProblem("negative-precondition"));

        Assert.Equal(2, PlanLength(problem));
        Assert.Equal(1d, MaxValue(problem));
        Assert.Equal(2d, H2Value(problem));
    }

    [XunitFact]
    public void NegativePrecondition_WithoutAnyDeleteIsUnreachable()
    {
        Problem problem = CreateProblem(
            """
(define (domain unestablished-negative-precondition)
  (:requirements :strips :negative-preconditions)
  (:predicates (p) (g))
  (:action finish :parameters () :precondition (not (p)) :effect (g)))
""",
            NegativePreconditionProblem("unestablished-negative-precondition"));

        AssertUnsolvable(problem);
        Assert.Equal(double.PositiveInfinity, H2Value(problem));
    }

    /// <summary>
    /// Deletes are applied before adds, so an action that deletes and adds the same
    /// fluent leaves it true and never establishes its negative literal.
    /// </summary>
    [XunitFact]
    public void FluentAddedAfterItsDelete_NeverEstablishesTheNegativeLiteral()
    {
        Problem problem = CreateProblem(
            """
(define (domain add-after-delete)
  (:requirements :strips :negative-preconditions)
  (:predicates (p) (g))
  (:action touch :parameters () :precondition () :effect (and (not (p)) (p)))
  (:action finish :parameters () :precondition (not (p)) :effect (g)))
""",
            NegativePreconditionProblem("add-after-delete"));

        AssertUnsolvable(problem);
        Assert.Equal(double.PositiveInfinity, H2Value(problem));
    }

    /// <summary>
    /// A conditional delete is a way of establishing a negative literal. Missing it would
    /// make the literal look unreachable and invent a dead end.
    /// </summary>
    [XunitFact]
    public void ConditionalDelete_EstablishesTheNegativeLiteral()
    {
        Problem problem = CreateProblem(
            """
(define (domain conditional-delete)
  (:requirements :strips :negative-preconditions :conditional-effects)
  (:predicates (i) (p) (g))
  (:action maybe :parameters () :precondition (i) :effect (when (i) (not (p))))
  (:action finish :parameters () :precondition (not (p)) :effect (g)))
""",
            """
(define (problem conditional-delete-problem)
  (:domain conditional-delete)
  (:init (i) (p))
  (:goal (g)))
""");

        Assert.Equal(2, PlanLength(problem));
        Assert.Equal(2d, H2Value(problem));
    }

    [XunitFact]
    public void NegativeGoalLiteral_Throws()
    {
        Problem problem = CreateProblem(
            """
(define (domain negative-goal)
  (:requirements :strips)
  (:predicates (p) (g))
  (:action finish :parameters () :precondition (p) :effect (g)))
""",
            """
(define (problem negative-goal-problem)
  (:domain negative-goal)
  (:init (p))
  (:goal (g)))
""");
        GoalCondition goal = SearchTestHelpers.ContradictoryGoal(problem);
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        Assert.Throws<NotSupportedException>(
            () => new H2Heuristic(generator).Evaluate(problem.InitialState.Expand(), goal));
    }

    /// <summary>
    /// The goal costs its own action plus the most expensive pair on the way to it, which
    /// is what separates h^2 from h^max on a task where nothing deletes anything.
    /// </summary>
    [XunitFact]
    public void WeightedPairCost_ExceedsTheMostExpensiveSingleFact()
    {
        Problem problem = CreateWeightedPairProblem();

        Assert.Equal(8d, MaxValue(problem));
        Assert.Equal(10d, H2Value(problem));
    }

    private static Problem CreateWeightedPairProblem()
    {
        DomainBuilder domainBuilder = new DomainBuilder("weighted-pair");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("p");
        predicates.Add("q");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-p");
        action.AddPrecondition("start");
        action.AddEffect("p");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-q");
        action.AddPrecondition("start");
        action.AddEffect("q");
        action.WithCost(3d);
        action.Close();
        action = actions.Add("make-goal");
        action.AddPrecondition("p");
        action.AddPrecondition("q");
        action.AddEffect("goal");
        action.WithCost(5d);
        action.Close();
        actions.Close();

        ProblemBuilder problemBuilder = new ProblemBuilder(domainBuilder.Build(), "weighted-pair-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    /// <summary>
    /// Exhaustive cross-validation: over every reachable state of a small instance, h^2
    /// must stay between h^1 and the true goal distance, and it must prove every dead end
    /// the delete relaxation already proves.
    /// </summary>
    [Theory]
    [InlineData("blocks_3", "p01.pddl")]
    [InlineData("gripper", "p01.pddl")]
    [InlineData("miconic", "p01.pddl")]
    [InlineData("spanner", "p01.pddl")]
    [InlineData("spanner", "p03.pddl")]
    [InlineData("ferry", "p01.pddl")]
    [InlineData("childsnack", "p01.pddl")]
    [InlineData("barman", "p01.pddl")]
    public void EveryReachableState_KeepsH2BetweenH1AndTheTruth(string domainDirectory, string problemFile)
    {
        Problem problem = SearchTestHelpers.LoadProblem(domainDirectory, problemFile);
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace space = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(GoalCondition.FromProblem(problem))
            .WithActionGenerator(generator)
            .Build();
        MaxHeuristic max = new(generator);
        FFHeuristic ff = new(generator);
        H2Heuristic h2 = new(generator);
        PerfectHeuristic perfect = new(space);

        foreach (State state in space.AllStates)
        {
            ExtendedState extendedState = state.Expand();
            double h1Value = max.Evaluate(extendedState).Value;
            double h2Value = h2.Evaluate(extendedState).Value;
            double ffValue = ff.Evaluate(extendedState).Value;
            double perfectValue = perfect.Evaluate(extendedState).Value;

            Assert.True(h1Value <= h2Value, $"h^1 {h1Value} exceeds h^2 {h2Value} in {state}");
            Assert.True(h2Value <= perfectValue, $"h^2 {h2Value} exceeds h* {perfectValue} in {state}");
            Assert.True(
                !double.IsPositiveInfinity(ffValue) || double.IsPositiveInfinity(h2Value),
                $"h^FF proves a dead end that h^2 misses in {state}");
        }
    }

    /// <summary>
    /// The optimised implementation must agree with a naive transcription of the h^2
    /// equations on every reachable state, which the admissibility bounds alone cannot show.
    /// </summary>
    [Theory]
    [InlineData("blocks_3", "p01.pddl")]
    [InlineData("gripper", "p01.pddl")]
    [InlineData("ferry", "p01.pddl")]
    [InlineData("miconic", "p01.pddl")]
    [InlineData("spanner", "p03.pddl")]
    public void EveryReachableState_MatchesTheNaiveReferenceImplementation(
        string domainDirectory,
        string problemFile)
    {
        Problem problem = SearchTestHelpers.LoadProblem(domainDirectory, problemFile);
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace space = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(GoalCondition.FromProblem(problem))
            .WithActionGenerator(generator)
            .Build();
        H2Heuristic h2 = new(generator);
        NaiveH2 naive = new(generator);
        int[] goalFluents = problem.Goal
            .Select(literal => literal.Value is Fact<Fluent> fluent
                ? fluent.LocalIndex
                : throw new InvalidOperationException("The reference comparison needs a positive fluent goal."))
            .ToArray();

        foreach (State state in space.AllStates)
        {
            Assert.Equal(naive.Evaluate(state, goalFluents), h2.Evaluate(state.Expand()).Value);
        }
    }

    private static string NegativePreconditionProblem(string domainName) =>
        $"""
(define (problem {domainName}-problem)
  (:domain {domainName})
  (:init (p))
  (:goal (g)))
""";

    private static Problem CreateProblem(string domainText, string problemText)
        => SearchTestHelpers.CreateProblemFromText(domainText, problemText);

    private static double H2Value(Problem problem)
        => new H2Heuristic(SearchTestHelpers.CreateGroundedGenerator(problem))
            .Evaluate(problem.InitialState.Expand())
            .Value;

    private static double MaxValue(Problem problem)
        => new MaxHeuristic(SearchTestHelpers.CreateGroundedGenerator(problem))
            .Evaluate(problem.InitialState.Expand())
            .Value;

    private static int PlanLength(Problem problem)
    {
        PlanResult result = Solve(problem);
        Assert.True(result.IsSuccess);
        return result.Plan.Count;
    }

    private static void AssertUnsolvable(Problem problem) => Assert.False(Solve(problem).IsSuccess);

    private static PlanResult Solve(Problem problem)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        return new BreadthFirstPlanner((_, _) => generator).Solve(problem);
    }
}
