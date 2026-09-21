using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public sealed class CrossLayerIntegrationTests
{
    [Fact]
    public void GroundedRpgHeuristics_RejectDerivedConditionalSupport()
    {
        Problem problem = SearchTestHelpers.CreateProblemFromText(
            """
(define (domain derived-conditional-cost)
  (:requirements :strips :adl :conditional-effects :action-costs)
  (:predicates (seed) (ready) (enabled) (goal))
  (:derived (unlocked) (ready))

  (:action distract
    :parameters ()
    :precondition (seed)
    :effect (and (increase (total-cost) 3)))

  (:action finish
    :parameters ()
    :precondition (and (ready) (unlocked))
    :effect (and (when (enabled) (goal)) (increase (total-cost) 2))))
""",
            """
(define (problem derived-conditional-cost-problem)
  (:domain derived-conditional-cost)
  (:init (seed) (ready) (enabled) (= (total-cost) 0))
  (:goal (goal))
  (:metric minimize (total-cost)))
""");

        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Throws<NotSupportedException>(() => new AddHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new FFHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new SetAddHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(generator.Problem));
    }

    [Fact]
    public void AStar_ReopensStateReachedThroughDerivedPreconditionAndReturnsCheaperPlan()
    {
        Problem problem = SearchTestHelpers.CreateProblemFromText(
            """
(define (domain astar-derived-reopen)
  (:requirements :strips :adl :action-costs)
  (:predicates (start) (b) (x) (goal))
  (:derived (reachable) (b))

  (:action start-to-x-expensive
    :parameters ()
    :precondition (start)
    :effect (and (not (start)) (x) (increase (total-cost) 10)))

  (:action start-to-b
    :parameters ()
    :precondition (start)
    :effect (and (not (start)) (b) (increase (total-cost) 1)))

  (:action b-to-x
    :parameters ()
    :precondition (reachable)
    :effect (and (not (b)) (x) (increase (total-cost) 1)))

  (:action x-to-goal
    :parameters ()
    :precondition (x)
    :effect (and (not (x)) (goal) (increase (total-cost) 1))))
""",
            """
(define (problem astar-derived-reopen-problem)
  (:domain astar-derived-reopen)
  (:init (start) (= (total-cost) 0))
  (:goal (goal))
  (:metric minimize (total-cost)))
""");

        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Fact<Fluent> bId = SearchTestHelpers.GetFluentFact(problem, "b");
        Fact<Fluent> xId = SearchTestHelpers.GetFluentFact(problem, "x");
        Fact<Fluent> goalId = SearchTestHelpers.GetFluentFact(problem, "goal");
        int expandedX = 0;

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithHeuristic(new DelegateHeuristic(state =>
            {
                if (state.IsTrue(goalId)) return 100d;
                if (state.IsTrue(bId)) return 20d;
                if (state.IsTrue(xId)) return 0d;
                return 0d;
            }))
            .OnNodeExpanded(node =>
            {
                if (node.State.IsTrue(xId))
                    expandedX++;
            })
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3d, result.PlanCost);
        Assert.Equal(2, expandedX);
        Assert.Equal(new[] { "(start-to-b)", "(b-to-x)", "(x-to-goal)" }, SearchTestHelpers.PlanSteps(result.Plan));
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void GeneratorsCostsAxiomsAndPlannersAgreeOnAdlProblem(ApplicableActionGeneratorType generatorType)
    {
        Problem problem = SearchTestHelpers.CreateProblemFromText(
            """
(define (domain generator-cost-axiom-integration)
  (:requirements :strips :adl :negative-preconditions :conditional-effects :action-costs)
  (:predicates (start) (prepared) (enabled) (junk) (goal))
  (:derived (unlocked) (prepared))

  (:action distract
    :parameters ()
    :precondition (start)
    :effect (and (junk) (increase (total-cost) 1)))

  (:action prepare
    :parameters ()
    :precondition (start)
    :effect (and (not (start)) (prepared) (increase (total-cost) 2)))

  (:action finish
    :parameters ()
    :precondition (and (prepared) (unlocked) (not (junk)))
    :effect (and (when (enabled) (goal)) (increase (total-cost) 3))))
""",
            """
(define (problem generator-cost-axiom-integration-problem)
  (:domain generator-cost-axiom-integration)
  (:init (start) (enabled) (= (total-cost) 0))
  (:goal (goal))
  (:metric minimize (total-cost)))
""", generatorType);

        var cases = new[]
        {
            PlannerFactory.Create("ucs"),
            PlannerFactory.Create("astar", "goal-count")
        };

        foreach (IPlanner planner in cases)
        {
            var result = planner.Solve(problem);

            Assert.True(result.IsSuccess);
            Assert.Equal(5d, result.PlanCost);
            Assert.Equal(new[] { "(prepare)", "(finish)" }, SearchTestHelpers.PlanSteps(result.Plan));
            AssertPlanIsApplicable(problem, result.Plan);
        }
    }

    private static void AssertPlanIsApplicable(Problem problem, IReadOnlyList<Action> plan)
    {
        State state = problem.InitialState;

        foreach (Action action in plan)
        {
            Assert.True(action.IsApplicable(state.Expand()), $"Expected {action} to be applicable.");
            state = state.Expand().Apply(action);
        }

        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(state.Expand()));
    }
}
