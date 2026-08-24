using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public sealed class PlannerExecutionTests
{
    public static TheoryData<string> Algorithms => new()
    {
        "bfs",
        "ucs",
        "astar",
        "gbfs",
        "iw1"
    };

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_GroundedGeneratorUsesEffectiveCustomStart(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        Fact<Fluent> enabled = SearchTestHelpers.GetFluentFact(problem, "enabled");
        State customStart = problem.InitialState.WithAdditionalFluentFacts([enabled]);
        State? factoryStart = null;
        IPlanner planner = CreatePlanner(algorithm, (factoryProblem, startState) =>
        {
            factoryStart = startState;
            return SearchTestHelpers.CreateGroundedGenerator(factoryProblem, startState);
        });

        PlanResult result = planner.Solve(problem, customStart);

        Assert.Equal(SearchStatus.Succeeded, result.Status);
        Assert.Equal(customStart, factoryStart);
        Assert.Single(result.Plan);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(customStart.Expand().Apply(result.Plan[0]).Expand()));
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_RejectsForeignInputsBeforeInvokingGeneratorFactory(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        Problem foreignProblem = CreateCustomStartProblem();
        int factoryCalls = 0;
        IPlanner planner = CreatePlanner(algorithm, (factoryProblem, startState) =>
        {
            factoryCalls++;
            return SearchTestHelpers.CreateGroundedGenerator(factoryProblem, startState);
        });

        ArgumentException startError = Assert.Throws<ArgumentException>(
            () => planner.Solve(problem, foreignProblem.InitialState));
        ArgumentException goalError = Assert.Throws<ArgumentException>(
            () => planner.Solve(problem, goal: GoalCondition.FromProblem(foreignProblem)));
        using var cancellationSource = new CancellationTokenSource();
        cancellationSource.Cancel();
        Assert.Throws<ArgumentException>(
            () => planner.Solve(
                problem,
                foreignProblem.InitialState,
                cancellationToken: cancellationSource.Token));
        Assert.Throws<ArgumentException>(
            () => planner.Solve(
                problem,
                goal: GoalCondition.FromProblem(foreignProblem),
                cancellationToken: cancellationSource.Token));

        Assert.Equal("startState", startError.ParamName);
        Assert.Equal("goal", goalError.ParamName);
        Assert.Equal(0, factoryCalls);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_RejectsNullGeneratorFactoryResult(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        IPlanner planner = CreatePlanner(algorithm, (_, _) => null!);

        InvalidOperationException error = Assert.Throws<InvalidOperationException>(() => planner.Solve(problem));

        Assert.Contains("returned null", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_RejectsGeneratorForForeignProblem(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        Problem foreignProblem = CreateCustomStartProblem();
        IPlanner planner = CreatePlanner(
            algorithm,
            (_, _) => SearchTestHelpers.CreateGroundedGenerator(foreignProblem, foreignProblem.InitialState));

        InvalidOperationException error = Assert.Throws<InvalidOperationException>(() => planner.Solve(problem));

        Assert.Contains("different problem", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_RejectsGroundedGeneratorForDifferentStart(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        Fact<Fluent> enabled = SearchTestHelpers.GetFluentFact(problem, "enabled");
        State customStart = problem.InitialState.WithAdditionalFluentFacts([enabled]);
        IPlanner planner = CreatePlanner(
            algorithm,
            (factoryProblem, _) => SearchTestHelpers.CreateGroundedGenerator(factoryProblem));

        InvalidOperationException error = Assert.Throws<InvalidOperationException>(
            () => planner.Solve(problem, customStart));

        Assert.Contains("different start state", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_ExhaustedSearchReturnsFailedStatus(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        IPlanner planner = CreatePlanner(algorithm, SearchTestHelpers.CreateGroundedGenerator);

        PlanResult result = planner.Solve(problem);

        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.False(result.IsSuccess);
        Assert.False(result.IsCanceled);
        Assert.Empty(result.Plan);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_PreCanceledTokenSkipsFactoriesAndReturnsCanceledStatus(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        int factoryCalls = 0;
        IPlanner planner = CreatePlanner(algorithm, (factoryProblem, startState) =>
        {
            factoryCalls++;
            return SearchTestHelpers.CreateGroundedGenerator(factoryProblem, startState);
        });
        using var cancellationSource = new CancellationTokenSource();
        cancellationSource.Cancel();

        PlanResult result = planner.Solve(problem, cancellationToken: cancellationSource.Token);

        Assert.Equal(SearchStatus.Canceled, result.Status);
        Assert.True(result.IsCanceled);
        Assert.False(result.IsSuccess);
        Assert.Equal(0, factoryCalls);
        Assert.Empty(result.Plan);
        Assert.Equal(TimeSpan.Zero, result.SearchTime);
        Assert.Equal(result.SetupTime, result.ElapsedTime);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Search_PreCanceledTokenReturnsCanceledBeforeGeneratingRoot(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        int generated = 0;
        var builder = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .OnNodeGenerated(_ => generated++);
        ISearchAlgorithm search = algorithm switch
        {
            "bfs" => builder.BuildBfs(),
            "ucs" => builder.BuildUcs(),
            "astar" => builder.WithHeuristic(BlindHeuristic.Instance).BuildAStar(),
            "gbfs" => builder.WithHeuristic(BlindHeuristic.Instance).BuildGbfs(),
            "iw1" => builder.BuildIw(1),
            _ => throw new ArgumentOutOfRangeException(nameof(algorithm), algorithm, "Unknown search algorithm.")
        };
        using var cancellationSource = new CancellationTokenSource();
        cancellationSource.Cancel();

        SearchResult result = search.Search(cancellationSource.Token);

        Assert.Equal(SearchStatus.Canceled, result.Status);
        Assert.True(result.IsCanceled);
        Assert.Equal(0, generated);
        Assert.Equal(0, result.Statistics.NodesGenerated);
        Assert.Equal(0, result.Statistics.NodesExpanded);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Search_CancellationDuringGenerationReturnsPartialStatistics(string algorithm)
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        using var cancellationSource = new CancellationTokenSource();
        var builder = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(SearchTestHelpers.ContradictoryGoal(problem))
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .OnNodeGenerated(node =>
            {
                if (node.Depth == 1)
                    cancellationSource.Cancel();
            });
        ISearchAlgorithm search = algorithm switch
        {
            "bfs" => builder.BuildBfs(),
            "ucs" => builder.BuildUcs(),
            "astar" => builder.WithHeuristic(BlindHeuristic.Instance).BuildAStar(),
            "gbfs" => builder.WithHeuristic(BlindHeuristic.Instance).BuildGbfs(),
            "iw1" => builder.BuildIw(1),
            _ => throw new ArgumentOutOfRangeException(nameof(algorithm), algorithm, "Unknown search algorithm.")
        };

        SearchResult result = search.Search(cancellationSource.Token);

        Assert.Equal(SearchStatus.Canceled, result.Status);
        Assert.Equal(1, result.Statistics.NodesExpanded);
        Assert.Equal(2, result.Statistics.NodesGenerated);
    }

    private static IPlanner CreatePlanner(
        string algorithm,
        Func<Problem, State, IApplicableActionGenerator> generatorFactory)
    {
        return algorithm switch
        {
            "bfs" => new BreadthFirstPlanner(generatorFactory),
            "ucs" => new UniformCostPlanner(generatorFactory),
            "astar" => new AStarPlanner(generatorFactory, (_, _, _) => BlindHeuristic.Instance),
            "gbfs" => new GreedyBestFirstPlanner(generatorFactory, (_, _, _) => BlindHeuristic.Instance),
            "iw1" => new IteratedWidthPlanner(1, generatorFactory),
            _ => throw new ArgumentOutOfRangeException(nameof(algorithm), algorithm, "Unknown planner algorithm.")
        };
    }

    private static Problem CreateCustomStartProblem()
    {
        DomainBuilder builder = new DomainBuilder("planner-custom-start")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("seed").Add("enabled").Add("done").Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("enable").AddPrecondition("seed").AddEffect("enabled").Close();
        actions.Add("finish").AddPrecondition("enabled").AddEffect("done").Close();
        Domain domain = actions.Close().Build();
        Problem problem = new ProblemBuilder(domain, "planner-custom-start-problem")
            .Goal().Add("enabled").Add("done").Close().Build();

        _ = GoalCondition.FromProblem(problem);
        return problem;
    }
}
