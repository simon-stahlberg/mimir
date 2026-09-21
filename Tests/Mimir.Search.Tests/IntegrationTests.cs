using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class IntegrationTests
{
    // ========================================================================
    // End-to-End: Simple domains without costs
    // ========================================================================

    [Fact]
    public void BlocksWorld_FindsPlanAndValidatesGoal()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("blocks_4");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new BreadthFirstPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert — plan found
        Assert.True(result.IsSuccess);
        Assert.Equal(4, result.PlanLength);

        // Assert — plan validity: execute each step
        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        // Assert — goal reached
        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    [Fact]
    public void Ferry_FindsPlanAndValidatesGoal()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new BreadthFirstPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.True(result.IsSuccess);
        Assert.Equal(7, result.PlanLength);

        // Validate plan execution
        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    [Fact]
    public void Gripper_FindsPlanAndValidatesGoal()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("gripper");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new BreadthFirstPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.True(result.IsSuccess);
        Assert.Equal(3, result.PlanLength);

        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    // ========================================================================
    // End-to-End: Domains with costs (weighted search)
    // ========================================================================

    [Fact]
    public void Transport_FindsCostOptimalPlan()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("transport");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new UniformCostPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.True(result.IsSuccess);
        Assert.Equal(5, result.PlanLength);
        Assert.Equal(54.0, result.PlanCost);

        // Validate plan execution
        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    // ========================================================================
    // End-to-End: A* with heuristic
    // ========================================================================

    [Fact]
    public void Logistics_FindsPlanWithAStar()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("logistics");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new AStarPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.True(result.IsSuccess);
        Assert.Equal(4, result.PlanLength);

        // Validate plan execution
        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    // ========================================================================
    // Unsolvable problem detection
    // ========================================================================

    [Theory]
    [InlineData("blocks_4")]
    [InlineData("gripper")]
    [InlineData("logistics")]
    public void UnsolvableProblems_ReturnFailure(string problemPrefix)
    {
        // Arrange
        var problem = SearchTestHelpers.LoadUnsolvableProblem(problemPrefix);
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new BreadthFirstPlanner();

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.False(result.IsSuccess);
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Empty(result.Plan);
    }

    // ========================================================================
    // Numeric fluents: reward domain uses numeric initialization and effects
    // ========================================================================

    [Fact]
    public void Reward_FindsPlanWithNumericFluents()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("reward");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new BreadthFirstPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.True(result.IsSuccess);
        Assert.Equal(4, result.PlanLength);

        // Validate plan execution
        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    // ========================================================================
    // Derived predicates: rovers domain uses derived predicates
    // ========================================================================

    [Fact]
    public void Rovers_FindsPlanWithDerivedPredicates()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("rovers");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var planner = new BreadthFirstPlanner();
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var result = planner.Solve(problem);

        // Assert
        Assert.True(result.IsSuccess);
        Assert.Equal(4, result.PlanLength);

        // Validate plan execution
        State state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }
}
