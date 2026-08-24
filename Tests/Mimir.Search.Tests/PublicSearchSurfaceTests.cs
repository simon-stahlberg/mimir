using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class PublicSearchSurfaceTests
{
    [Fact]
    public void PlannerConstructorsRejectMissingRequiredGeneratorFactories()
    {
        Assert.Throws<ArgumentNullException>(() => new BreadthFirstPlanner(null!));
        Assert.Throws<ArgumentNullException>(() => new UniformCostPlanner(null!));
        Assert.Throws<ArgumentNullException>(() => new AStarPlanner(null!));
        Assert.Throws<ArgumentNullException>(() => new GreedyBestFirstPlanner(null!));
        Assert.Throws<ArgumentNullException>(() => new IteratedWidthPlanner(1, null!));
    }

    [Theory]
    [InlineData(-1)]
    [InlineData(4)]
    public void IteratedWidthPlannerRejectsUnsupportedWidthAtConstruction(int width)
    {
        Assert.Throws<ArgumentOutOfRangeException>(
            () => new IteratedWidthPlanner(width, SearchTestHelpers.CreateGroundedGenerator));
    }

    [Fact]
    public void SearchBuilder_RejectsGoalFromAnotherProblemDuringBuild()
    {
        var first = SearchTestHelpers.LoadProblem("ferry");
        var second = SearchTestHelpers.LoadProblem("ferry");

        var builder = new SearchBuilder()
            .WithInitialState(first.InitialState)
            .WithGoal(second)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(first));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => builder.BuildBfs());

        Assert.Contains("goal condition", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void SearchBuilder_RejectsGeneratorFromAnotherProblemDuringBuild()
    {
        var first = SearchTestHelpers.LoadProblem("ferry");
        var second = SearchTestHelpers.LoadProblem("ferry");

        var builder = new SearchBuilder()
            .WithInitialState(first.InitialState)
            .WithGoal(first)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(second));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => builder.BuildBfs());

        Assert.Contains("action generator", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void PlanResult_DefensivelyCopiesPlan()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var mutablePlan = new List<Mimir.Core.Grounding.Action>
        {
            generator.GetApplicableActions(problem.InitialState.Expand()).First()
        };
        var result = new PlanResult(
            SearchStatus.Succeeded,
            mutablePlan,
            new SearchStatistics(1, 1, TimeSpan.Zero, 0),
            TimeSpan.Zero,
            TimeSpan.Zero);

        mutablePlan.Clear();

        Assert.Single(result.Plan);
        Assert.Throws<NotSupportedException>(() => ((IList<Mimir.Core.Grounding.Action>)result.Plan).Clear());
    }

    [Fact]
    public void SearchResult_DefensivelyCopiesPlanAndKeepsCachedMetadataConsistent()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Mimir.Core.Grounding.Action action = generator.GetApplicableActions(problem.InitialState.Expand()).First();
        var mutablePlan = new List<Mimir.Core.Grounding.Action> { action };
        var result = SearchResult.Success(
            mutablePlan,
            new SearchStatistics(1, 1, TimeSpan.Zero, 1));

        mutablePlan.Clear();

        Assert.Single(result.Plan);
        Assert.Equal(1, result.PlanLength);
        Assert.Equal(action.Cost, result.PlanCost);
        Assert.Throws<NotSupportedException>(() => ((IList<Mimir.Core.Grounding.Action>)result.Plan).Clear());
    }

}
