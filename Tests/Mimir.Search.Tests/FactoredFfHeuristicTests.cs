using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;


public class FactoredFfHeuristicTests
{
    /// <summary>
    /// The factored graph reaches the same atoms at the same layers as enumerating every binding, so the relaxed
    /// plan it extracts has to be the same size give or take the choice of supporter, which FF never pinned down.
    /// </summary>
    [Theory]
    [InlineData("gripper")]
    [InlineData("childsnack")]
    [InlineData("logistics")]
    [InlineData("miconic")]
    [InlineData("delivery")]
    public void FactoredFfTracksTheBindingEnumeratingFf(string domainDirectory)
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            domainDirectory,
            generatorType: ApplicableActionGeneratorType.Factored);
        var goal = GoalCondition.FromProblem(problem);
        FactoredFfHeuristic? factored = FactoredFfHeuristic.TryCreate(problem, goal);
        Assert.NotNull(factored);
        var lifted = new LiftedFfHeuristic(problem, goal);

        var queue = new Queue<State>();
        var seen = new HashSet<State> { problem.InitialState };
        queue.Enqueue(problem.InitialState);
        var generator = problem.GetApplicableActionGenerator(problem.InitialState);
        int checksRun = 0;

        while (queue.Count > 0 && checksRun < 60)
        {
            ExtendedState state = queue.Dequeue().Expand();
            double a = lifted.Evaluate(state, goal).Value;
            double b = factored!.Evaluate(state, goal).Value;
            Assert.Equal(double.IsPositiveInfinity(a), double.IsPositiveInfinity(b));
            if (!double.IsPositiveInfinity(a))
            {
                // One action of slack for the supporter each picked, plus a margin that grows with the estimate.
                Assert.True(Math.Abs(a - b) <= 1 + (0.2 * Math.Max(a, 1)),
                    $"{domainDirectory}: lifted {a} against factored {b}.");
            }

            checksRun++;
            foreach (GroundAction action in generator.GetApplicableActions(state).Take(4))
            {
                State next = state.Apply(action);
                if (seen.Add(next)) queue.Enqueue(next);
            }
        }

        Assert.True(checksRun > 1);
    }

    [Fact]
    public void FactoredFfIsZeroExactlyAtTheGoal()
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            "gripper", generatorType: ApplicableActionGeneratorType.Factored);
        var goal = GoalCondition.FromProblem(problem);
        FactoredFfHeuristic factored = FactoredFfHeuristic.TryCreate(problem, goal)!;

        Assert.True(factored.Evaluate(problem.InitialState.Expand(), goal).Value > 0);

        PlanResult solved = new GreedyBestFirstPlanner(
            (_, g) => FactoredFfHeuristic.TryCreate(problem, g)!).Solve(problem);
        Assert.True(solved.IsSuccess);
        State goalState = solved.Plan.Aggregate(
            problem.InitialState, (state, action) => state.Expand().Apply(action));
        Assert.Equal(0d, factored.Evaluate(goalState.Expand(), goal).Value);
    }

    [Fact]
    public void AStaticGoalAtomIsSettledByTheStateRatherThanByALayer()
    {
        Domain domain = Domain.FromText("""
(define (domain probe) (:requirements :strips)
 (:predicates (fixed) (p) (q))
 (:action go :parameters () :precondition (p) :effect (q)))
""");
        Problem problem = Problem.FromText(domain,
            "(define (problem s) (:domain probe) (:init (fixed) (p)) (:goal (and (fixed) (q))))");
        var goal = GoalCondition.FromProblem(problem);
        var ff = FactoredFfHeuristic.TryCreate(problem, goal)!;
        double value = ff.Evaluate(problem.InitialState.Expand(), goal).Value;
        Assert.False(double.IsPositiveInfinity(value), $"static goal atom gave {value}");
    }

    [Fact]
    public void ANumericGoalIsDeclinedRatherThanIgnored()
    {
        Domain domain = Domain.FromText("""
(define (domain probe2) (:requirements :strips :numeric-fluents)
 (:predicates (p)) (:functions (v))
 (:action go :parameters () :precondition (p) :effect (p)))
""");
        Problem problem = Problem.FromText(domain,
            "(define (problem s) (:domain probe2) (:init (p) (= (v) 0)) (:goal (and (p) (>= (v) 5))))",
            ApplicableActionGeneratorType.Factored);
        var goal = GoalCondition.FromProblem(problem);
        var ff = FactoredFfHeuristic.TryCreate(problem, goal);
        Assert.True(ff is null, "a numeric goal should be declined, not ignored");
    }
}
