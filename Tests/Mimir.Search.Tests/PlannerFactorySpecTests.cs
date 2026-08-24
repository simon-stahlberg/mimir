using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public sealed class PlannerFactorySpecTests
{
    [Theory]
    [InlineData("grounded", 7d)]
    [InlineData(" grounded : ucs ", 32d)]
    [InlineData("grounded:uniform-cost", 32d)]
    [InlineData("grounded:uniformcost", 32d)]
    [InlineData("grounded:astar:blind", 32d)]
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

        var result = PlannerFactory.CreateFromSpec("grounded:gbfs:goal-count").Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Theory]
    [InlineData("", "plannerSpec", "empty string")]
    [InlineData("   ", "plannerSpec", "whitespace")]
    [InlineData("grounded:", "plannerSpec", "Expected '<generator>")]
    [InlineData("grounded::blind", "plannerSpec", "Expected '<generator>")]
    [InlineData(":ucs", "plannerSpec", "Expected '<generator>")]
    [InlineData("::grounded", "plannerSpec", "Expected '<generator>")]
    [InlineData("grounded:astar:", "plannerSpec", "Expected '<generator>")]
    [InlineData("grounded:ucs:blind:extra", "plannerSpec", "Expected '<generator>")]
    [InlineData("unknown:bfs", "generatorType", "Unknown generator type 'unknown'")]
    [InlineData("clique:bfs", "generatorType", "Unknown generator type 'clique'")]
    [InlineData("grounded:unknown", "algorithmType", "Unknown algorithm type 'unknown'")]
    [InlineData("grounded:astar:unknown", "heuristicType", "Unknown heuristic type 'unknown'")]
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

        var planner = PlannerFactory.Create("grounded", algorithm, "unknown");
        var result = planner.Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.Equal(7d, result.PlanCost);
    }

    [Theory]
    [InlineData("bfs")]
    [InlineData("ucs")]
    [InlineData("uniform-cost")]
    [InlineData("uniformcost")]
    [InlineData("astar")]
    [InlineData("gbfs")]
    [InlineData("iw0")]
    [InlineData("iw1")]
    [InlineData("iw2")]
    [InlineData("iw3")]
    public void Create_UnknownGeneratorThrowsBeforeSolveForEveryAlgorithm(string algorithm)
    {
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => PlannerFactory.Create("unknown", algorithm, "blind"));

        Assert.Equal("Unknown generator type 'unknown'. (Parameter 'generatorType')", exception.Message);
    }

    [Theory]
    [InlineData("astar")]
    [InlineData("gbfs")]
    public void Create_HeuristicAlgorithmsValidateUnknownHeuristicBeforeSolve(string algorithm)
    {
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => PlannerFactory.Create("grounded", algorithm, "unknown"));

        Assert.Equal("Unknown heuristic type 'unknown'. (Parameter 'heuristicType')", exception.Message);
    }

    [Theory]
    [InlineData("not-a-search")]
    [InlineData("iw4")]
    [InlineData("a-star")]
    public void Create_UnknownAlgorithmThrowsBeforeSolve(string algorithm)
    {
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => PlannerFactory.Create("grounded", algorithm, "blind"));

        Assert.Equal($"Unknown algorithm type '{algorithm}'. (Parameter 'algorithmType')", exception.Message);
    }

}
