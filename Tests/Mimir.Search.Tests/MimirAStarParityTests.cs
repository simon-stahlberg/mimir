using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class MimirAStarParityTests
{
    private static string BenchmarkPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Benchmark/strips");

    [Fact]
    public void GroundedAStarHMax_SupportsDeliveryEqualityPreconditions()
    {
        var domain = Domain.FromFile(Path.Combine(BenchmarkPath, "delivery", "domain.pddl"));
        var problem = Problem.FromFile(
            domain,
            Path.Combine(BenchmarkPath, "delivery", "instance_s-7_p-3_v-0.pddl"));

        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithHeuristic(new MaxHeuristic(generator))
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }
}
