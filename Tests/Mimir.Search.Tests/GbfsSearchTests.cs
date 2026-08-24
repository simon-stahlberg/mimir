using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class GbfsSearchTests
{
    [Theory]
    [InlineData("blocks_4")]
    [InlineData("gripper")]
    [InlineData("logistics")]
    public void GbfsWithBlindHeuristic_ReturnsFailureOnUnsolvableProblems(string problemPrefix)
    {
        var problem = SearchTestHelpers.LoadUnsolvableProblem(problemPrefix);

        var gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .BuildGbfs();

        var gbfsResult = gbfs.Search();

        Assert.False(gbfsResult.IsSuccess);
        Assert.Equal(SearchStatus.Failed, gbfsResult.Status);
        Assert.Empty(gbfsResult.Plan);
    }

    [Fact]
    public void GbfsWithLiftedFfHeuristic_FindsPlanForBlocksWorld()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_4");

        var gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new LiftedFfHeuristic(problem))
            .BuildGbfs();

        var result = gbfs.Search();

        Assert.True(result.IsSuccess);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(
            SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Fact]
    public void GbfsChecksGoalAtRoot_BeforeExpandingSuccessors()
    {
        Domain domain = new DomainBuilder("root-goal")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("goal").Add("other").Close()
            .Actions().Add("distract").AddPrecondition("goal").AddEffect("other").Close().Close().Build();
        Problem problem = new ProblemBuilder(domain, "root-goal-problem")
            .InitialState().AddFact("goal").Close().Goal().Add("goal").Close().Build();

        var goalNodes = new List<SearchNode>();

        var gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .OnGoalNodeExpanded(goalNodes.Add)
            .BuildGbfs();

        var result = gbfs.Search();

        Assert.True(result.IsSuccess);
        Assert.Empty(result.Plan);
        Assert.Single(goalNodes);
        Assert.Equal(problem.InitialState, goalNodes[0].State);
    }

    [Fact]
    public void GbfsCancellation_ReturnsCanceledWithoutGeneratingRoot()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_4");
        using var cancellation = new CancellationTokenSource();
        cancellation.Cancel();

        var gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new LiftedFfHeuristic(problem))
            .BuildGbfs();

        var result = gbfs.Search(cancellation.Token);

        Assert.False(result.IsSuccess);
        Assert.True(result.IsCanceled);
        Assert.Equal(SearchStatus.Canceled, result.Status);
        Assert.Empty(result.Plan);
        Assert.Equal(0, result.Statistics.NodesExpanded);
        Assert.Equal(0, result.Statistics.NodesGenerated);
        Assert.Equal(0, result.Statistics.MaxDepth);
    }

    [Fact]
    public void GbfsCancellationFromStateGeneratedCallback_StopsAtNextLoopBoundary()
    {
        Problem problem = CreateTieBreakProblem();
        using var cancellation = new CancellationTokenSource();
        int transitions = 0;

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .OnStateGenerated(_ =>
            {
                transitions++;
                cancellation.Cancel();
            })
            .BuildGbfs()
            .Search(cancellation.Token);

        Assert.True(result.IsCanceled);
        Assert.True(transitions > 0);
    }

    [Fact]
    public void GbfsCallbackEvents_ReportTheExpandedGoalNode()
    {
        var problem = SearchTestHelpers.LoadProblem("childsnack");
        var expandedGoalStates = new List<SearchNode>();
        var goal = GoalCondition.FromProblem(problem);

        var gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .OnGoalNodeExpanded(expandedGoalStates.Add)
            .BuildGbfs();

        var result = gbfs.Search();

        Assert.True(result.IsSuccess);
        Assert.Single(expandedGoalStates);
        Assert.True(goal.IsSatisfied(expandedGoalStates[0].State.Expand()));
    }

    [Fact]
    public void GbfsCallback_ReportsNewBestHeuristicValue()
    {
        Problem problem = CreateTieBreakProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Fact<Fluent> b = SearchTestHelpers.GetFluentFact(problem, "b");
        var bestHeuristicValues = new List<double>();

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(new DelegateHeuristic(state => state.IsTrue(b) ? 1d : 2d))
            .OnNewBestHValue(bestHeuristicValues.Add)
            .BuildGbfs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Contains(1d, bestHeuristicValues);
    }

    [Fact]
    public void GbfsStatePruned_FiresOnceForDistinctInfiniteNonGoalState()
    {
        Problem problem = CreateTieBreakProblem();
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
            .BuildGbfs()
            .Search();

        Assert.False(result.IsSuccess);
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Single(pruned);
        Assert.True(pruned[0].IsTrue(a));
    }

    [Fact]
    public void GbfsDoesNotPruneGoalSuccessorWithInfiniteHeuristic()
    {
        Problem problem = CreateTieBreakProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Fact<Fluent> a = SearchTestHelpers.GetFluentFact(problem, "a");
        var pruned = new List<State>();

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(new DelegateHeuristic(state => state.IsTrue(a) ? double.PositiveInfinity : 0d))
            .OnStatePruned(pruned.Add)
            .BuildGbfs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Empty(pruned);
        Assert.Equal(new[] { "start-to-a" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Theory]
    [InlineData(double.NaN)]
    [InlineData(-1d)]
    [InlineData(double.NegativeInfinity)]
    public void GbfsRejectsInvalidHeuristicValues(double value)
    {
        Problem problem = CreateTieBreakProblem();

        ISearchAlgorithm gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new DelegateHeuristic(_ => value))
            .BuildGbfs();

        Assert.Throws<InvalidOperationException>(() => gbfs.Search());
    }

    [Fact]
    public void GbfsPrefersPreferredActionsOverEarlierEqualHeuristicSuccessors()
    {
        Problem problem = CreatePreferredActionProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        var startId = SearchTestHelpers.GetFluentFact(problem, "start");
        var aId = SearchTestHelpers.GetFluentFact(problem, "a");
        var bId = SearchTestHelpers.GetFluentFact(problem, "b");
        var expansionOrder = new List<string>();

        var gbfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(SearchTestHelpers.ContradictoryGoal(problem))
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new DelegateHeuristic(state =>
            {
                if (state.IsTrue(startId))
                    return new HeuristicEvaluation(0, action => action.Schema.Name.Equals("start-to-a", StringComparison.OrdinalIgnoreCase));

                return new HeuristicEvaluation(0);
            }))
            .OnNodeExpanded(node => expansionOrder.Add(LabelState(node.State, startId, aId, bId)))
            .BuildGbfs();

        var result = gbfs.Search();

        Assert.False(result.IsSuccess);
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Equal(new[] { "start", "a" }, expansionOrder.Take(2));
    }

    private static string LabelState(State state, Fact<Fluent> startId, Fact<Fluent> aId, Fact<Fluent> bId)
    {
        if (state.IsTrue(startId)) return "start";
        if (state.IsTrue(aId)) return "a";
        if (state.IsTrue(bId)) return "b";
        throw new InvalidOperationException("State did not match any expected label.");
    }

    private static Problem CreateTieBreakProblem()
        => CreateGraphProblem(
            "tie-break-gbfs",
            ["start", "a", "b"],
            [("start-to-a", "start", "a"), ("start-to-b", "start", "b")],
            "a");

    private static Problem CreatePreferredActionProblem()
        => CreateGraphProblem(
            "preferred-action-gbfs",
            ["start", "a", "b"],
            [("start-to-b", "start", "b"), ("start-to-a", "start", "a")],
            "a");

    private static Problem CreateGraphProblem(
        string name,
        IReadOnlyList<string> predicates,
        IReadOnlyList<(string Name, string From, string To)> transitions,
        string goal)
    {
        DomainBuilder builder = new DomainBuilder(name)
            .Requirements().Add(":strips").Close();
        PredicateListBuilder predicateBuilder = builder.Predicates();
        foreach (string predicate in predicates)
            predicateBuilder.Add(predicate);
        predicateBuilder.Close();

        ActionListBuilder actions = builder.Actions();
        foreach ((string actionName, string from, string to) in transitions)
        {
            actions.Add(actionName).AddPrecondition(from)
                .AddEffect(from, Polarity.Negative).AddEffect(to).Close();
        }

        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, $"{name}-problem")
            .InitialState().AddFact("start").Close()
            .Goal().Add(goal).Close().Build();
    }

}
