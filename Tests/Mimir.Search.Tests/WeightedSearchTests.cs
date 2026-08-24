using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class WeightedSearchTests
{
    [Fact]
    public void WeightedTransport_BfsReturnsShorterButMoreExpensivePlan()
    {
        var problem = SearchTestHelpers.LoadProblem("transport", "p02.pddl");

        var bfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildBfs();

        var result = bfs.Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3, result.PlanLength);
        Assert.Equal(102d, result.PlanCost);
    }

    [Fact]
    public void WeightedTransport_UcsReturnsCheaperLongerPlan()
    {
        var problem = SearchTestHelpers.LoadProblem("transport", "p02.pddl");

        var ucs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs();

        var result = ucs.Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(5, result.PlanLength);
        Assert.Equal(32d, result.PlanCost);
    }

    [Fact]
    public void WeightedTransport_AStarWithBlindMatchesUcsCheapestPlan()
    {
        var problem = SearchTestHelpers.LoadProblem("transport", "p02.pddl");

        var ucs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs();

        var astar = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .BuildAStar();

        var ucsResult = ucs.Search();
        var astarResult = astar.Search();

        Assert.True(ucsResult.IsSuccess);
        Assert.True(astarResult.IsSuccess);
        Assert.Equal(ucsResult.PlanCost, astarResult.PlanCost);
    }

    [Fact]
    public void Ucs_DoesNotReturnExpensiveGoalQueuedBeforeCheaperDuplicate()
    {
        Problem problem = CreateDirectGoalProblem();

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(2d, result.PlanCost);
        Assert.Equal(new[] { "start-to-a", "a-to-goal" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Fact]
    public void Ucs_SkipsStaleOpenEntryAfterCheaperPathUpdatesSameState()
    {
        Problem problem = CreateDuplicateWeightedProblem();
        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3d, result.PlanCost);
        Assert.Equal(new[] { "start-to-b", "b-to-x", "x-to-goal" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Fact]
    public void Ucs_AllowsZeroCostPrefixBeforePositiveGoalCost()
    {
        Problem problem = CreateZeroCostPrefixProblem();

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3, result.PlanLength);
        Assert.Equal(5d, result.PlanCost);
        Assert.Equal(new[] { "start-to-a", "a-to-b", "b-to-goal" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Fact]
    public void Ucs_ZeroCostCycleTerminatesAndFindsGoal()
    {
        Problem problem = CreateZeroCostCycleProblem();

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(1d, result.PlanCost);
        Assert.Equal(new[] { "start-to-a", "a-to-b", "b-to-goal" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Fact]
    public void Ucs_ChoosesCheaperNumericCostBindingForSameSuccessorState()
    {
        Problem problem = CreateNumericDuplicateProblem();

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3d, result.PlanCost);
        Assert.Equal(new[] { "start-to-via", "via-to-x", "x-to-goal" }, result.Plan.Select(action => action.Schema.Name));
        Assert.Equal("cheap", result.Plan[1].Arguments.Single().Name);
    }

    [Fact]
    public void AStar_WithBlindHeuristic_MatchesUcsOnWeightedDuplicateGraph()
    {
        Problem problem = CreateDuplicateWeightedProblem();

        var ucsResult = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildUcs()
            .Search();

        var astarResult = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .BuildAStar()
            .Search();

        Assert.True(ucsResult.IsSuccess);
        Assert.True(astarResult.IsSuccess);
        Assert.Equal(ucsResult.PlanCost, astarResult.PlanCost);
    }

    [Fact]
    public void AStar_ReopensNumericCostDuplicateStateAndPreservesCheaperPlan()
    {
        Problem problem = CreateNumericDuplicateProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        var viaId = SearchTestHelpers.GetFluentFact(problem, "via");
        var xId = SearchTestHelpers.GetFluentFact(problem, "x");
        var goalId = SearchTestHelpers.GetFluentFact(problem, "goal");

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(new DelegateHeuristic(state =>
            {
                if (state.IsTrue(viaId)) return 2;
                if (state.IsTrue(xId)) return 0;
                if (state.IsTrue(goalId)) return 0;
                return 0;
            }))
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3d, result.PlanCost);
        Assert.Equal(new[] { "start-to-via", "via-to-x", "x-to-goal" }, result.Plan.Select(action => action.Schema.Name));
        Assert.Equal("cheap", result.Plan[1].Arguments.Single().Name);
    }

    [Fact]
    public void AStar_UpdatesOpenStateWhenLowerGHasHigherHeuristicTiePressure()
    {
        Problem problem = CreateDuplicateWeightedProblem();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        _ = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        var xId = SearchTestHelpers.GetFluentFact(problem, "x");
        var bId = SearchTestHelpers.GetFluentFact(problem, "b");
        var goalId = SearchTestHelpers.GetFluentFact(problem, "goal");

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(new DelegateHeuristic(state =>
            {
                if (state.IsTrue(bId)) return 9;
                if (state.IsTrue(xId)) return 0;
                if (state.IsTrue(goalId)) return 0;
                return 0;
            }))
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(3d, result.PlanCost);
        Assert.Equal(new[] { "start-to-b", "b-to-x", "x-to-goal" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Fact]
    public void AStar_DoesNotReturnGoalFromStaleHigherCostDuplicate()
    {
        Problem problem = CreateDirectGoalProblem();

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(BlindHeuristic.Instance)
            .BuildAStar()
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(2d, result.PlanCost);
        Assert.Equal(new[] { "start-to-a", "a-to-goal" }, result.Plan.Select(action => action.Schema.Name));
    }

    [Theory]
    [InlineData("blocks_3")]
    [InlineData("ferry")]
    public void AStar_WithGoalCountHeuristic_FindsOptimalPlan(string domainDir)
    {
        var problem = SearchTestHelpers.LoadProblem(domainDir);

        var bfsResult = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .BuildBfs()
            .Search();

        var astarResult = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .WithHeuristic(new GoalCountHeuristic(problem))
            .BuildAStar()
            .Search();

        Assert.True(bfsResult.IsSuccess);
        Assert.True(astarResult.IsSuccess);
        Assert.Equal(bfsResult.PlanCost, astarResult.PlanCost);
    }

    private static Problem CreateDirectGoalProblem()
        => CreateWeightedGraphProblem(
            "direct-goal-weighted",
            ["start", "a", "goal"],
            [
                ("start-to-goal", "start", "goal", 100d),
                ("start-to-a", "start", "a", 1d),
                ("a-to-goal", "a", "goal", 1d)
            ],
            "goal");

    private static Problem CreateDuplicateWeightedProblem()
        => CreateWeightedGraphProblem(
            "duplicate-weighted",
            ["start", "b", "x", "goal"],
            [
                ("start-to-x-expensive", "start", "x", 10d),
                ("start-to-b", "start", "b", 1d),
                ("b-to-x", "b", "x", 1d),
                ("x-to-goal", "x", "goal", 1d)
            ],
            "goal");

    private static Problem CreateZeroCostPrefixProblem()
        => CreateWeightedGraphProblem(
            "zero-cost-prefix",
            ["start", "a", "b", "goal"],
            [
                ("direct-goal", "start", "goal", 6d),
                ("start-to-a", "start", "a", 0d),
                ("a-to-b", "a", "b", 0d),
                ("b-to-goal", "b", "goal", 5d)
            ],
            "goal");

    private static Problem CreateZeroCostCycleProblem()
        => CreateWeightedGraphProblem(
            "zero-cost-cycle",
            ["start", "a", "b", "goal"],
            [
                ("start-to-a", "start", "a", 0d),
                ("a-to-b", "a", "b", 0d),
                ("b-to-a", "b", "a", 0d),
                ("b-to-goal", "b", "goal", 1d)
            ],
            "goal");

    private static Problem CreateWeightedGraphProblem(
        string name,
        IReadOnlyList<string> predicates,
        IReadOnlyList<(string Name, string From, string To, double Cost)> transitions,
        string goal)
    {
        DomainBuilder builder = CreateWeightedGraphDomain(name, predicates);
        ActionListBuilder actions = builder.Actions();
        foreach ((string actionName, string from, string to, double cost) in transitions)
            AddWeightedTransition(actions, actionName, from, to, cost);

        return CreateGraphProblem(actions.Close().Build(), goal);
    }

    private static DomainBuilder CreateWeightedGraphDomain(
        string name,
        IReadOnlyList<string> predicates)
    {
        DomainBuilder builder = new DomainBuilder(name)
            .Requirements().Add(":strips").Add(":action-costs").Close();
        PredicateListBuilder predicateBuilder = builder.Predicates();
        foreach (string predicate in predicates)
            predicateBuilder.Add(predicate);
        return predicateBuilder.Close();
    }

    private static void AddWeightedTransition(
        ActionListBuilder actions,
        string name,
        string from,
        string to,
        double cost)
        => actions.Add(name)
            .AddPrecondition(from)
            .AddEffect(from, Polarity.Negative)
            .AddEffect(to)
            .WithCost(cost)
            .Close();

    private static Problem CreateGraphProblem(Domain domain, string goal)
        => new ProblemBuilder(domain, $"{domain.Name}-problem")
            .InitialState().AddFact("start").Close()
            .Goal().Add(goal).Close()
            .Build();

    private static Problem CreateNumericDuplicateProblem()
    {
        DomainBuilder builder = new DomainBuilder("numeric-duplicate-cost")
            .Requirements().Add(":strips").Add(":typing").Add(":action-costs").Close()
            .Types().Add("ticket").Close()
            .Predicates()
                .Add("start").Add("via").Add("x").Add("goal")
                .Add("direct-ticket", ("?ticket", "ticket"))
                .Add("via-ticket", ("?ticket", "ticket"))
                .Close()
            .Functions().Add("price", ("?ticket", "ticket")).Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("direct-to-x").AddParameter("?ticket", "ticket")
            .AddPrecondition("start").AddPrecondition("direct-ticket", "?ticket")
            .AddEffect("start", Polarity.Negative).AddEffect("x")
            .WithCost(ActionCost.Function("price", "?ticket")).Close();
        actions.Add("start-to-via").AddPrecondition("start")
            .AddEffect("start", Polarity.Negative).AddEffect("via").WithCost(1d).Close();
        actions.Add("via-to-x").AddParameter("?ticket", "ticket")
            .AddPrecondition("via").AddPrecondition("via-ticket", "?ticket")
            .AddEffect("via", Polarity.Negative).AddEffect("x")
            .WithCost(ActionCost.Function("price", "?ticket")).Close();
        actions.Add("x-to-goal").AddPrecondition("x")
            .AddEffect("x", Polarity.Negative).AddEffect("goal").WithCost(1d).Close();
        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, "numeric-duplicate-cost-problem")
            .Objects().Add("expensive", "ticket").Add("cheap", "ticket").Close()
            .InitialState()
                .AddFact("start")
                .AddFact("direct-ticket", "expensive")
                .AddFact("via-ticket", "cheap")
                .AddNumericInitialization("price", 3d, "expensive")
                .AddNumericInitialization("price", 1d, "cheap")
                .Close()
            .Goal().Add("goal").Close()
            .Build();
    }

}
