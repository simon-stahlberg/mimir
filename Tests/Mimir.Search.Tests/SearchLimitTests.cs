using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public sealed class SearchLimitTests
{
    [Fact]
    public void EveryAlgorithmHonorsZeroExpansionLimit()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_3");

        ISearchAlgorithm[] searches =
        [
            NewBuilder().BuildBfs(),
            NewBuilder().BuildUcs(),
            NewBuilder().WithHeuristic(BlindHeuristic.Instance).BuildAStar(),
            NewBuilder().WithHeuristic(BlindHeuristic.Instance).BuildGbfs(),
            NewBuilder().BuildIw(3),
        ];

        foreach (ISearchAlgorithm search in searches)
        {
            SearchResult result = search.Search(maxExpandedStates: 0);

            Assert.Equal(SearchStatus.ExpansionLimitReached, result.Status);
            Assert.Equal(0, result.Statistics.NodesExpanded);
            Assert.Empty(result.Plan);
        }

        SearchBuilder NewBuilder() => new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem));
    }

    [Fact]
    public void IwUsesOneExpansionBudgetAcrossWidths()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_3");
        ISearchAlgorithm search = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildIw(3);

        SearchResult result = search.Search(maxExpandedStates: 2);

        Assert.Equal(SearchStatus.ExpansionLimitReached, result.Status);
        Assert.Equal(2, result.Statistics.NodesExpanded);
    }
}
