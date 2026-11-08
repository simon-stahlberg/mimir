using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class SerializedIteratedWidthTests
{
    [Theory]
    [InlineData("gripper", "lifted")]
    [InlineData("gripper", "factored")]
    [InlineData("childsnack", "factored")]
    [InlineData("delivery", "factored")]
    [InlineData("miconic", "factored")]
    [InlineData("ferry", "factored")]
    [InlineData("logistics", "factored")]
    public void FindsAPlanThatReachesTheGoal(string domainDirectory, string generatorType)
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            domainDirectory,
            generatorType: Enum.Parse<ApplicableActionGeneratorType>(generatorType, true));
        var goal = GoalCondition.FromProblem(problem);

        PlanResult result = new SerializedIteratedWidthPlanner().Solve(problem);

        Assert.True(result.IsSuccess, $"SIW should solve '{domainDirectory}'.");
        State state = problem.InitialState;
        foreach (GroundAction action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()), $"'{action}' is not applicable in '{domainDirectory}'.");
            state = state.Expand().Apply(action);
        }

        Assert.True(goal.IsSatisfied(state.Expand()), $"Goal not reached for '{domainDirectory}'.");
    }

    /// <summary>
    /// Every round has to leave fewer goal atoms unachieved than it started with, so the number of rounds is
    /// bounded by the goal's size however long the plan turns out to be.
    /// </summary>
    [Fact]
    public void MakesProgressOnTheGoalEveryRound()
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            "gripper",
            generatorType: ApplicableActionGeneratorType.Factored);
        var goal = GoalCondition.FromProblem(problem);

        var widths = new List<int>();
        var planner = new SerializedIteratedWidthPlanner();
        planner.RoundCompleted += widths.Add;
        PlanResult result = planner.Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.NotEmpty(widths);
        Assert.True(
            widths.Count <= goal.GoalLiterals.Count,
            $"{widths.Count} rounds for {goal.GoalLiterals.Count} goal atoms.");

        // Replaying the plan must show the count falling at the end of every round.
        State state = problem.InitialState;
        int remaining = goal.CountUnsatisfiedGoals(state.Expand());
        foreach (GroundAction action in result.Plan)
        {
            state = state.Expand().Apply(action);
            int now = goal.CountUnsatisfiedGoals(state.Expand());
            Assert.True(now <= remaining || now < goal.GoalLiterals.Count);
        }

        Assert.Equal(0, goal.CountUnsatisfiedGoals(state.Expand()));
    }

    [Fact]
    public void ReturnsAnEmptyPlanWhenTheGoalAlreadyHolds()
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            "gripper",
            generatorType: ApplicableActionGeneratorType.Factored);
        PlanResult solved = new SerializedIteratedWidthPlanner().Solve(problem);
        Assert.True(solved.IsSuccess);

        State goalState = solved.Plan.Aggregate(
            problem.InitialState,
            (state, action) => state.Expand().Apply(action));

        PlanResult result = new SerializedIteratedWidthPlanner().Solve(problem, goalState);
        Assert.True(result.IsSuccess);
        Assert.Empty(result.Plan);
    }

    [Theory]
    [InlineData("blocks_4")]
    [InlineData("gripper")]
    public void ReportsFailureOnAnUnsolvableProblem(string problemPrefix)
    {
        Problem problem = SearchTestHelpers.LoadUnsolvableProblem(problemPrefix);
        PlanResult result = new SerializedIteratedWidthPlanner().Solve(problem);

        Assert.False(result.IsSuccess);
        Assert.Empty(result.Plan);
    }

    /// <summary>
    /// The detector decides which state a round may commit to. Where no round is stranded it must change nothing,
    /// because a round it rejects is one the search has to keep looking for.
    /// </summary>
    [Theory]
    [InlineData("gripper")]
    [InlineData("miconic")]
    [InlineData("logistics")]
    public void ARoundRejectsOnlyStatesThatStrandTheRemainingGoals(string domainDirectory)
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            domainDirectory,
            generatorType: ApplicableActionGeneratorType.Factored);
        var goal = GoalCondition.FromProblem(problem);

        PlanResult without = new SerializedIteratedWidthPlanner().Solve(problem);
        PlanResult with = new SerializedIteratedWidthPlanner().Solve(
            problem, deadEndDetector: new FfDeadEndDetector(problem));

        Assert.True(without.IsSuccess);
        Assert.True(with.IsSuccess);
        Assert.Equal(without.PlanLength, with.PlanLength);

        State state = problem.InitialState;
        foreach (GroundAction action in with.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goal.IsSatisfied(state.Expand()));
    }

    [Fact]
    public void PlannerFactoryBuildsSiw()
    {
        Assert.IsType<SerializedIteratedWidthPlanner>(PlannerFactory.CreateFromSpec("siw"));
        Assert.IsType<SerializedIteratedWidthPlanner>(PlannerFactory.CreateFromSpec("siw2"));
    }
}
