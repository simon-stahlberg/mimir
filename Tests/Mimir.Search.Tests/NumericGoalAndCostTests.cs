using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class NumericGoalAndCostTests
{
    private static Problem CreateFuelProblem(ApplicableActionGeneratorType generatorType)
    {
        Domain domain = Domain.FromText("""
(define (domain d) (:requirements :strips :numeric-fluents :action-costs)
 (:functions (fuel) (rate) (total-cost))
 (:action step :parameters () :precondition (>= (fuel) 1)
  :effect (and (decrease (fuel) 1) (increase (total-cost) (rate)))))
""");
        return Problem.FromText(domain, """
(define (problem p) (:domain d) (:init (= (fuel) 2) (= (rate) 1.5) (= (total-cost) 0))
 (:goal (= (fuel) 0)) (:metric minimize (total-cost)))
""", generatorType);
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void NumericGoalsAreEvaluatedAgainstStates(ApplicableActionGeneratorType generatorType)
    {
        Problem problem = CreateFuelProblem(generatorType);
        Action action = problem.GroundAction(problem.Domain.Actions[0]);
        GoalCondition goal = GoalCondition.FromProblem(problem);
        State initial = problem.InitialState;
        State last = initial.Expand().Apply(action).Expand().Apply(action);

        Assert.False(goal.IsSatisfied(initial.Expand()));
        Assert.Equal(1, goal.CountUnsatisfiedGoals(initial.Expand()));
        Assert.True(goal.IsSatisfied(last.Expand()));
        Assert.Equal(0, goal.CountUnsatisfiedGoals(last.Expand()));
        Assert.Equal(1, new GoalCountHeuristic(problem).Evaluate(initial.Expand()).Value);
        Assert.NotEqual(goal, GoalCondition.Always(problem));
        Assert.Equal(goal, GoalCondition.FromLiterals(problem, [], problem.NumericGoals));
        Assert.Equal(goal.GetHashCode(), GoalCondition.FromProblem(problem).GetHashCode());
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void StaticNumericCostsSurvivePlanReporting(ApplicableActionGeneratorType generatorType)
    {
        Problem problem = CreateFuelProblem(generatorType);
        Action action = problem.GroundAction(problem.Domain.Actions[0]);
        Assert.Equal(1.5, action.Cost);

        SearchResult result = SearchResult.Success([action, action], new SearchStatistics(2, 3, TimeSpan.Zero, 2));
        Assert.Equal(3, PlannerExecution.ToPlanResult(result, TimeSpan.Zero, TimeSpan.Zero).PlanCost);
        IPlanner[] planners = [new UniformCostPlanner(), new BreadthFirstPlanner()];
        foreach (IPlanner planner in planners)
        {
            PlanResult solved = planner.Solve(problem);
            Assert.True(solved.IsSuccess);
            Assert.Equal(3, solved.PlanCost);
            Assert.Equal(2, solved.PlanLength);
        }
    }

    [Fact]
    public void PropositionalOnlyComponentsRejectNumericProblems()
    {
        Problem problem = CreateFuelProblem(ApplicableActionGeneratorType.Grounded);
        var generator = (GroundedApplicableActionGenerator)problem.GetApplicableActionGenerator(problem.InitialState);
        Assert.Throws<NotSupportedException>(() => new LiftedFfHeuristic(problem));
        Assert.Throws<NotSupportedException>(() => new IteratedWidthPlanner(1).Solve(problem));
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(problem));
    }

    [Fact]
    public void StateDependentActionCostsAreRejectedAtDomainLoad()
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Domain.FromText("""
(define (domain d) (:requirements :strips :numeric-fluents :action-costs)
 (:functions (fuel) (total-cost))
 (:action step :parameters () :precondition (>= (fuel) 1)
  :effect (and (decrease (fuel) 1) (increase (total-cost) (fuel)))))
"""));
        NotSupportedException inner = Assert.IsType<NotSupportedException>(exception.InnerException);
        Assert.Contains("Action 'step'", inner.Message);
        Assert.Contains("'fuel'", inner.Message);
    }

    [Fact]
    public void NumericConditionsHiddenInDerivedPredicatesStillRejectNumericHeuristics()
    {
        Domain domain = Domain.FromText("""
(define (domain derived-numeric) (:requirements :adl :derived-predicates :numeric-fluents)
 (:predicates (available) (done)) (:functions (fuel))
 (:derived (available) (not (<= (fuel) 0)))
 (:action act :parameters () :precondition (available) :effect (done)))
""");
        Problem problem = Problem.FromText(domain,
            "(define (problem p) (:domain derived-numeric) (:init (= (fuel) 1)) (:goal (done)))",
            ApplicableActionGeneratorType.Grounded);
        Assert.True(problem.HasNumericPlanning);
        Assert.Equal(1, new GoalCountHeuristic(problem).Evaluate(problem.InitialState.Expand()).Value);
        Assert.Throws<NotSupportedException>(() => new LiftedFfHeuristic(problem));
        var generator = (GroundedApplicableActionGenerator)problem.GetApplicableActionGenerator(problem.InitialState);
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(problem));
        Assert.True(new BreadthFirstPlanner().Solve(problem).IsSuccess);
    }

    [Fact]
    public void PublicGoalExpressionsAllowConjunctiveComparisonsAndValidateOwnership()
    {
        Domain domain = Domain.FromText("(define (domain goals) (:requirements :numeric-fluents) (:functions (fuel)))");
        const string text = "(define (problem p) (:domain goals) (:init (= (fuel) 2)) (:goal (>= (fuel) 1)))";
        Problem problem = Problem.FromText(domain, text);
        NumericComparison comparison = problem.FunctionCall("fuel").GreaterThanOrEqual(1);
        GoalCondition goal = GoalCondition.FromExpression(problem, new GroundedAnd([comparison]));
        Assert.Equal(GoalCondition.FromProblem(problem), goal);
        Assert.True(goal.IsSatisfied(problem.InitialState.Expand()));
        Assert.Throws<ArgumentException>(() => GoalCondition.FromExpression(problem, new GroundedOr([comparison])));
        Assert.Throws<ArgumentException>(() => GoalCondition.FromExpression(problem, new GroundedNot(comparison)));
        Problem other = Problem.FromText(domain, text);
        Assert.Throws<ArgumentException>(() => GoalCondition.FromExpression(other, comparison));
    }

    [Fact]
    public void GoalEqualityIgnoresComparisonOrder()
    {
        Domain domain = Domain.FromText("(define (domain goals) (:requirements :numeric-fluents) (:functions (fuel) (load)))");
        Problem problem = Problem.FromText(domain,
            "(define (problem p) (:domain goals) (:init (= (fuel) 2) (= (load) 1)) (:goal (and (>= (fuel) 1) (<= (load) 3))))");
        GroundNumericComparison[] comparisons = problem.NumericGoals.ToArray();
        GoalCondition forward = GoalCondition.FromLiterals(problem, [], comparisons);
        GoalCondition backward = GoalCondition.FromLiterals(problem, [], comparisons.Reverse().ToArray());
        Assert.Equal(forward, backward);
        Assert.Equal(forward.GetHashCode(), backward.GetHashCode());
    }
}
