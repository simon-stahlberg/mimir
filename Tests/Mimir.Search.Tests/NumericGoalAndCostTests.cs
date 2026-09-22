using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class NumericGoalAndCostTests
{
    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void NumericGoalsAndStaticNumericCostsSurvivePlanReporting(ApplicableActionGeneratorType generatorType)
    {
        Domain domain = Domain.FromText("""
(define (domain d) (:requirements :strips :numeric-fluents :action-costs)
 (:functions (fuel) (rate) (total-cost))
 (:action step :parameters () :precondition (>= (fuel) 1)
  :effect (and (decrease (fuel) 1) (increase (total-cost) (rate)))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain d) (:init (= (fuel) 2) (= (rate) 1.5) (= (total-cost) 0))
 (:goal (= (fuel) 0)) (:metric minimize (total-cost)))
""", generatorType);
        var action = problem.GroundAction(domain.Actions[0]);
        GoalCondition goal = GoalCondition.FromProblem(problem);
        State initial = problem.InitialState;
        Assert.False(goal.IsSatisfied(initial.Expand()));
        Assert.Equal(1, goal.CountUnsatisfiedGoals(initial.Expand()));
        Assert.Equal(1.5, action.Cost);
        State next = initial.Expand().Apply(action);
        State last = next.Expand().Apply(action);
        Assert.True(goal.IsSatisfied(last.Expand()));
        Assert.Equal(0, goal.CountUnsatisfiedGoals(last.Expand()));
        Assert.NotEqual(goal, GoalCondition.Always(problem));
        Assert.Equal(goal, GoalCondition.FromLiterals(problem, [], problem.NumericGoals));
        Assert.Equal(goal.GetHashCode(), GoalCondition.FromProblem(problem).GetHashCode());
        SearchResult result = SearchResult.Success([action, action], new SearchStatistics(2, 3, TimeSpan.Zero, 2));
        PlanResult plan = PlannerExecution.ToPlanResult(result, TimeSpan.Zero, TimeSpan.Zero);
        Assert.Equal(3, plan.PlanCost);
        Assert.Equal(1, new GoalCountHeuristic(problem).Evaluate(initial.Expand()).Value);
        IPlanner[] planners = [new UniformCostPlanner(), new BreadthFirstPlanner()];
        foreach (IPlanner planner in planners)
        {
            PlanResult solved = planner.Solve(problem);
            Assert.True(solved.IsSuccess);
            Assert.Equal(3, solved.PlanCost);
            Assert.Equal(2, solved.PlanLength);
        }
        Assert.Throws<NotSupportedException>(() => new LiftedFfHeuristic(problem));
        Assert.Throws<NotSupportedException>(() => new IteratedWidthPlanner(1).Solve(problem));
        if (generatorType == ApplicableActionGeneratorType.Grounded)
        {
            var grounded = (Mimir.Core.Engines.GroundedApplicableActionGenerator)problem.GetApplicableActionGenerator(initial);
            Assert.Throws<NotSupportedException>(() => new MaxHeuristic(grounded));
            Assert.Throws<NotSupportedException>(() => new H2Heuristic(problem));
        }
    }

    [Fact]
    public void StateDependentActionCostsAreRejectedAtLoad()
    {
        Domain domain = Domain.FromText("""
(define (domain d) (:requirements :strips :numeric-fluents :action-costs)
 (:functions (fuel) (total-cost))
 (:action step :parameters () :precondition (>= (fuel) 1)
  :effect (and (decrease (fuel) 1) (increase (total-cost) (fuel)))))
""");
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Problem.FromText(domain, """
(define (problem p) (:domain d) (:init (= (fuel) 2) (= (total-cost) 0))
 (:goal (= (fuel) 0)) (:metric minimize (total-cost)))
"""));
        Assert.IsType<NotSupportedException>(exception.InnerException);
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
