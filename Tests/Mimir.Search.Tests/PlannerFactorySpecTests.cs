using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public sealed class PlannerFactorySpecTests
{
    [Theory]
    [InlineData("bfs", 7d)]
    [InlineData(" ucs ", 32d)]
    [InlineData("uniform-cost", 32d)]
    [InlineData("uniformcost", 32d)]
    [InlineData("astar:blind", 32d)]
    public void CreateFromSpec_ParsesValidSpecsAndAliases(string spec, double expectedPlanCost)
    {
        var problem = spec.Contains("ucs", StringComparison.OrdinalIgnoreCase)
            || spec.Contains("uniform", StringComparison.OrdinalIgnoreCase)
            || spec.Contains("astar", StringComparison.OrdinalIgnoreCase)
            ? SearchTestHelpers.LoadProblem("transport", "p02.pddl")
            : SearchTestHelpers.LoadProblem("ferry");

        var planner = PlannerFactory.CreateFromSpec(spec);

        var result = planner.Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.Equal(expectedPlanCost, result.PlanCost);
    }

    [Fact]
    public void CreateFromSpec_GbfsWithHeuristicFindsGoalReachingPlan()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_3");

        var result = PlannerFactory.CreateFromSpec("gbfs:goal-count").Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Theory]
    [InlineData("", "plannerSpec", "empty string")]
    [InlineData("   ", "plannerSpec", "whitespace")]
    [InlineData("bfs:", "plannerSpec", "Expected '<algorithm>")]
    [InlineData(":blind", "plannerSpec", "Expected '<algorithm>")]
    [InlineData(":ucs", "plannerSpec", "Expected '<algorithm>")]
    [InlineData("::grounded", "plannerSpec", "Expected '<algorithm>")]
    [InlineData("astar:", "plannerSpec", "Expected '<algorithm>")]
    [InlineData("ucs:blind:extra", "plannerSpec", "Expected '<algorithm>")]
    [InlineData("unknown", "algorithmType", "Unknown algorithm type 'unknown'")]
    [InlineData("astar:unknown", "heuristicType", "Unknown heuristic type 'unknown'")]
    public void CreateFromSpec_InvalidSpecsThrowExplicitArgumentException(
        string spec,
        string expectedParameter,
        string expectedMessageFragment)
    {
        ArgumentException exception = Assert.Throws<ArgumentException>(() => PlannerFactory.CreateFromSpec(spec));

        Assert.Equal(expectedParameter, exception.ParamName);
        Assert.Contains(expectedMessageFragment, exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Theory]
    [InlineData("bfs")]
    [InlineData("ucs")]
    [InlineData("uniform-cost")]
    [InlineData("uniformcost")]
    public void Create_NonHeuristicAlgorithmsIgnoreUnknownHeuristic(string algorithm)
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");

        var planner = PlannerFactory.Create(algorithm, "unknown");
        var result = planner.Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.Equal(7d, result.PlanCost);
    }


    [Theory]
    [InlineData("astar")]
    [InlineData("gbfs")]
    public void Create_HeuristicAlgorithmsValidateUnknownHeuristicBeforeSolve(string algorithm)
    {
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => PlannerFactory.Create(algorithm, "unknown"));

        Assert.Equal("Unknown heuristic type 'unknown'. (Parameter 'heuristicType')", exception.Message);
    }

    [Theory]
    [InlineData("not-a-search")]
    [InlineData("iw4")]
    [InlineData("a-star")]
    public void Create_UnknownAlgorithmThrowsBeforeSolve(string algorithm)
    {
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => PlannerFactory.Create(algorithm, "blind"));

        Assert.Equal($"Unknown algorithm type '{algorithm}'. (Parameter 'algorithmType')", exception.Message);
    }

}
