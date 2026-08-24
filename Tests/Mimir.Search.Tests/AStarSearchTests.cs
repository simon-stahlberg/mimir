using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class AStarSearchTests
{
    [Theory]
    [InlineData("blocks_4")]
    public void AStarWithBlindHeuristic_IsCostOptimalAgainstBfs(string domainDir)
    {
        var problem = SearchTestHelpers.LoadProblem(domainDir);

        var bfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildBfs();

        var astar = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .BuildAStar();

        var bfsResult = bfs.Search();
        var astarResult = astar.Search();
        GoalCondition goalCondition = GoalCondition.FromProblem(problem);

        Assert.True(bfsResult.IsSuccess);
        Assert.True(astarResult.IsSuccess);
        Assert.True(goalCondition.IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, bfsResult.Plan).Expand()));
        Assert.True(goalCondition.IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, astarResult.Plan).Expand()));
        Assert.Equal(bfsResult.PlanCost, astarResult.PlanCost);
    }

    [Theory]
    [InlineData("blocks_4")]
    [InlineData("gripper")]
    [InlineData("logistics")]
    public void AStarWithBlindHeuristic_ReturnsFailureOnUnsolvableProblems(string problemPrefix)
    {
        var problem = SearchTestHelpers.LoadUnsolvableProblem(problemPrefix);

        var astar = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .BuildAStar();

        var astarResult = astar.Search();

        Assert.False(astarResult.IsSuccess);
        Assert.Equal(SearchStatus.Failed, astarResult.Status);
        Assert.Empty(astarResult.Plan);
    }

    [Fact]
    public void AStarWithGoalCountHeuristicFindsPlanForBlocksWorld()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_4");

        var astar = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new GoalCountHeuristic(problem))
            .BuildAStar();

        var result = astar.Search();

        Assert.True(result.IsSuccess);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Fact]
    public void AStarCallbacks_ReportInitialFLayerAndGeneratedTransition()
    {
        Problem problem = CreateDuplicatePathProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Fact<Fluent> a = SearchTestHelpers.GetFluentFact(problem, "a");
        var completedFLayers = new List<double>();
        var generatedTransitions = new List<SearchTransition>();

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(BlindHeuristic.Instance)
            .OnFLayerFinished(completedFLayers.Add)
            .OnStateGenerated(generatedTransitions.Add)
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Contains(0d, completedFLayers);
        Assert.Contains(generatedTransitions, transition =>
            transition.State.Equals(problem.InitialState)
            && transition.SuccessorState.Expand().IsTrue(a));
    }

    [Fact]
    public void AStarStatePruned_FiresOnceForDistinctInfiniteNonGoalState()
    {
        Problem problem = CreateDuplicatePathProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Fact<Fluent> a = SearchTestHelpers.GetFluentFact(problem, "a");
        var pruned = new List<State>();

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(SearchTestHelpers.ContradictoryGoal(problem))
            .WithActionGenerator(generator)
            .WithHeuristic(new DelegateHeuristic(state => state.IsTrue(a) ? double.PositiveInfinity : 0d))
            .OnStatePruned(pruned.Add)
            .BuildAStar()
            .Search();

        Assert.False(result.IsSuccess);
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Single(pruned);
        Assert.True(pruned[0].IsTrue(a));
    }

    [Fact]
    public void AStarDoesNotPruneGoalSuccessorWithInfiniteHeuristic()
    {
        Problem problem = CreateDuplicatePathProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Fact<Fluent> target = SearchTestHelpers.GetFluentFact(problem, "t");
        var pruned = new List<State>();

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(new DelegateHeuristic(state => state.IsTrue(target) ? double.PositiveInfinity : 0d))
            .OnStatePruned(pruned.Add)
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Empty(pruned);
    }

    [Theory]
    [InlineData(double.NaN)]
    [InlineData(-1d)]
    [InlineData(double.NegativeInfinity)]
    public void AStarRejectsInvalidHeuristicValues(double value)
    {
        Problem problem = CreateDuplicatePathProblem();

        ISearchAlgorithm astar = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new DelegateHeuristic(_ => value))
            .BuildAStar();

        Assert.Throws<InvalidOperationException>(() => astar.Search());
    }

    private static Problem CreateDuplicatePathProblem()
    {
        DomainBuilder builder = new DomainBuilder("duplicate-paths")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("start").Add("a").Add("b").Add("c").Add("t").Close();
        ActionListBuilder actions = builder.Actions();
        AddTransition(actions, "start-to-a", "start", "a");
        AddTransition(actions, "a-to-c", "a", "c");
        AddTransition(actions, "c-to-t", "c", "t");
        AddTransition(actions, "start-to-b", "start", "b");
        AddTransition(actions, "b-to-t", "b", "t");
        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, "duplicate-paths-problem")
            .InitialState().AddFact("start").Close()
            .Goal().Add("t").Close()
            .Build();
    }

    private static void AddTransition(
        ActionListBuilder actions,
        string name,
        string from,
        string to)
        => actions.Add(name).AddPrecondition(from)
            .AddEffect(from, Polarity.Negative).AddEffect(to).Close();
}
