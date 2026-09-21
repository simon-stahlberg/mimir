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
        IPlanner planner = PlannerFactory.Create(algorithm);

        PlanResult result = planner.Solve(problem, customStart);

        Assert.Equal(SearchStatus.Succeeded, result.Status);
        Assert.Single(result.Plan);
        Assert.True(GoalCondition.FromProblem(problem).IsSatisfied(customStart.Expand().Apply(result.Plan[0]).Expand()));
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_RejectsForeignInputs(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        Problem foreignProblem = CreateCustomStartProblem();
        IPlanner planner = PlannerFactory.Create(algorithm);

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
    }
    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_ExhaustedSearchReturnsFailedStatus(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        IPlanner planner = PlannerFactory.Create(algorithm);

        PlanResult result = planner.Solve(problem);

        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.False(result.IsSuccess);
        Assert.False(result.IsCanceled);
        Assert.Empty(result.Plan);
    }

    [Theory]
    [MemberData(nameof(Algorithms))]
    public void Solve_PreCanceledTokenReturnsCanceledStatus(string algorithm)
    {
        Problem problem = CreateCustomStartProblem();
        IPlanner planner = PlannerFactory.Create(algorithm);
        using var cancellationSource = new CancellationTokenSource();
        cancellationSource.Cancel();

        PlanResult result = planner.Solve(problem, cancellationToken: cancellationSource.Token);

        Assert.Equal(SearchStatus.Canceled, result.Status);
        Assert.True(result.IsCanceled);
        Assert.False(result.IsSuccess);
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
