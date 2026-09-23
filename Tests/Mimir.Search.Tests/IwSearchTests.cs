using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class IwSearchTests
{
    [Fact]
    public void PreCancelledIwDoesNotGenerateOrExpandSatisfiedRoot()
    {
        Domain domain = new DomainBuilder("iw-root-goal")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("goal").Close().Build();
        Problem problem = new ProblemBuilder(domain, "iw-root-goal-problem")
            .InitialState().AddFact("goal").Close().Goal().Add("goal").Close().Build();
        using var cancellation = new CancellationTokenSource();
        cancellation.Cancel();
        int widths = 0;
        int generated = 0;
        int expanded = 0;

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .OnWidthStarted(_ => widths++)
            .OnNodeGenerated(_ => generated++)
            .OnNodeExpanded(_ => expanded++)
            .BuildIw(1)
            .Search(cancellation.Token);

        Assert.False(result.IsSuccess);
        Assert.True(result.IsCanceled);
        Assert.Equal(0, widths);
        Assert.Equal(0, generated);
        Assert.Equal(0, expanded);
        Assert.Equal(0, result.Statistics.NodesGenerated);
        Assert.Equal(0, result.Statistics.NodesExpanded);
    }

    [Fact]
    public void IwCancellationDuringSuccessorGenerationReturnsPartialStatistics()
    {
        var problem = SearchTestHelpers.LoadProblem("blocks_4");
        using var cancellation = new CancellationTokenSource();

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(SearchTestHelpers.ContradictoryGoal(problem))
            .OnNodeGenerated(node =>
            {
                if (node.Depth == 1)
                    cancellation.Cancel();
            })
            .BuildIw(0)
            .Search(cancellation.Token);

        Assert.False(result.IsSuccess);
        Assert.True(result.IsCanceled);
        Assert.Equal(1, result.Statistics.NodesExpanded);
        Assert.Equal(2, result.Statistics.NodesGenerated);
        Assert.Equal(0, result.Statistics.MaxDepth);
    }

    private static Problem CreateOneStepProblem(bool alreadyDone = false, string generatorType = "grounded")
    {
        Domain domain = new DomainBuilder("one-step")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("start").Add("done").Close()
            .Actions().Add("finish").AddPrecondition("start")
                .AddEffect("start", Polarity.Negative).AddEffect("done").Close().Close().Build();
        InitialStateBuilder initialState = new ProblemBuilder(
            domain,
            alreadyDone ? "already-done" : "one-step-p1",
            Enum.Parse<ApplicableActionGeneratorType>(generatorType, true)).InitialState();
        initialState.AddFact(alreadyDone ? "done" : "start");
        return initialState.Close().Goal().Add("done").Close().Build();
    }

    private static Problem CreateTwoStepProblem(string generatorType = "grounded")
    {
        DomainBuilder builder = CreateTransitionDomain("two-step", ["start", "middle", "done"]);
        ActionListBuilder actions = builder.Actions();
        AddTransition(actions, "step1", "start", "middle");
        AddTransition(actions, "step2", "middle", "done");
        return CreateProblem(actions.Close().Build(), "two-step-p1", "start", "done", generatorType);
    }

    private static Problem CreateLateEffectProblem(string generatorType = "grounded")
    {
        DomainBuilder builder = CreateTransitionDomain("late-effect", ["start", "ready", "done", "junk"]);
        ActionListBuilder actions = builder.Actions();
        AddTransition(actions, "prepare", "start", "ready");
        actions.Add("finish").AddPrecondition("ready").AddEffect("ready", Polarity.Negative)
            .AddEffect("done").AddEffect("junk").Close();
        return CreateProblem(actions.Close().Build(), "late-effect-p1", "start", "done", generatorType);
    }

    private static Problem CreateNonNovelNegativeGoalProblem(string generatorType = "grounded")
    {
        DomainBuilder builder = new DomainBuilder("non-novel-negative-goal")
            .Requirements().Add(":strips").Add(":negative-preconditions").Close()
            .Predicates().Add("p").Add("q").Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("add-q").AddPrecondition("p").AddEffect("q").Close();
        actions.Add("delete-p").AddPrecondition("p").AddPrecondition("q")
            .AddEffect("p", Polarity.Negative).Close();
        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, "non-novel-negative-goal-p1", Enum.Parse<ApplicableActionGeneratorType>(generatorType, true))
            .InitialState().AddFact("p").Close()
            .Goal().Add("q").Add("p", Polarity.Negative).Close().Build();
    }

    private static Problem CreateWidthTwoProblem(string generatorType = "grounded")
    {
        DomainBuilder builder = CreateTransitionDomain("width-two", ["start", "a", "b", "c"]);
        ActionListBuilder actions = builder.Actions();
        actions.Add("see-a").AddPrecondition("start").AddEffect("a").Close();
        actions.Add("see-b").AddPrecondition("start").AddEffect("b").Close();
        actions.Add("achieve").AddPrecondition("a").AddPrecondition("b").AddEffect("c").Close();
        return CreateProblem(actions.Close().Build(), "width-two-p1", "start", "c", generatorType);
    }

    private static DomainBuilder CreateTransitionDomain(string name, IReadOnlyList<string> predicates)
    {
        DomainBuilder builder = new DomainBuilder(name).Requirements().Add(":strips").Close();
        PredicateListBuilder predicateBuilder = builder.Predicates();
        foreach (string predicate in predicates)
            predicateBuilder.Add(predicate);
        return predicateBuilder.Close();
    }

    private static void AddTransition(ActionListBuilder actions, string name, string from, string to)
        => actions.Add(name).AddPrecondition(from).AddEffect(from, Polarity.Negative)
            .AddEffect(to).Close();

    private static Problem CreateProblem(
        Domain domain,
        string name,
        string initialFact,
        string goal,
        string generatorType = "grounded")
        => new ProblemBuilder(domain, name, Enum.Parse<ApplicableActionGeneratorType>(generatorType, true))
            .InitialState().AddFact(initialFact).Close()
            .Goal().Add(goal).Close().Build();

    // =========================================================================
    // 1. IW(0): goal at depth 1 succeeds
    // =========================================================================

    [Fact]
    public void IW0_GoalAtDepth1_Succeeds()
    {
        Problem problem = CreateOneStepProblem();
        var goalCondition = GoalCondition.FromProblem(problem);

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(0)
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Single(result.Plan);
        Assert.True(goalCondition.IsSatisfied(SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    // =========================================================================
    // 2. IW(0): goal requiring depth 2 fails
    // =========================================================================

    [Fact]
    public void IW0_GoalRequiresDepth2_Fails()
    {
        Problem problem = CreateTwoStepProblem();

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .BuildIw(0)
            .Search();

        Assert.False(result.IsSuccess);
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Empty(result.Plan);
    }

    // =========================================================================
    // 3. IW(0): initial state already satisfies the goal
    // =========================================================================

    [Fact]
    public void IW0_InitialStateIsGoal_ReturnsEmptyPlan()
    {
        Problem problem = CreateOneStepProblem(alreadyDone: true);
        var goalCondition = GoalCondition.FromProblem(problem);

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(0)
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Empty(result.Plan);
        Assert.True(goalCondition.IsSatisfied(problem.InitialState.Expand()));
    }

    // =========================================================================
    // 4. IW(1): finds a valid plan on a width-1 problem, step-by-step validation
    //
    // The two-step domain is width-1 by construction: start→middle→done, each
    // transition introduces a strictly new atom. ferry/gripper p01 both have
    // width 2 (the ferry/robot must return to a location seen in the initial
    // state, and that state carries no new singleton atom).
    // =========================================================================

    [Theory]
    [InlineData("grounded")]
    [InlineData("lifted")]
    public void IW1_FindsValidPlan_GoalIsReached(string generatorType)
    {
        Problem problem = CreateTwoStepProblem(generatorType);
        var goalCondition = GoalCondition.FromProblem(problem);

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(1)
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(2, result.Plan.Count);

        var state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()), $"Action '{action}' is not applicable.");
            state = state.Expand().Apply(action);
        }
        Assert.True(goalCondition.IsSatisfied(state.Expand()));
    }

    [Theory]
    [InlineData("grounded")]
    [InlineData("lifted")]
    public void IW1_LateRegisteredEffectFact_FindsRequiredActions(string generatorType)
    {
        Problem problem = CreateLateEffectProblem(generatorType);
        var goalCondition = GoalCondition.FromProblem(problem);

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(1)
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(new[] { "prepare", "finish" }, result.Plan.Select(action => action.Schema.Name));
        Assert.True(goalCondition.IsSatisfied(SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Theory]
    [InlineData(1, "grounded")]
    [InlineData(1, "lifted")]
    [InlineData(2, "grounded")]
    [InlineData(2, "lifted")]
    [InlineData(3, "grounded")]
    [InlineData(3, "lifted")]
    public void IWk_NonNovelNegativeGoal_IsNotPruned(int k, string generatorType)
    {
        Problem problem = CreateNonNovelNegativeGoalProblem(generatorType);
        GoalCondition goalCondition = GoalCondition.FromProblem(problem);

        SearchResult result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(k)
            .Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(new[] { "add-q", "delete-p" }, result.Plan.Select(action => action.Schema.Name));
        Assert.True(goalCondition.IsSatisfied(SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    // =========================================================================
    // 7. IW(2) solves a width-2 problem that IW(1) cannot solve
    // =========================================================================

    [Theory]
    [InlineData("grounded")]
    [InlineData("lifted")]
    public void IW2_SolvesWidth2ProblemThatIW1CannotSolve(string generatorType)
    {
        Problem problem = CreateWidthTwoProblem(generatorType);
        var goalCondition = GoalCondition.FromProblem(problem);

        var iw1Result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(1)
            .Search();

        var iw2Result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(2)
            .Search();

        Assert.False(iw1Result.IsSuccess, "IW(1) should fail on a width-2 problem.");
        Assert.Equal(SearchStatus.Failed, iw1Result.Status);
        Assert.True(iw2Result.IsSuccess, "IW(2) should solve a width-2 problem.");
        Assert.True(goalCondition.IsSatisfied(SearchTestHelpers.ApplyPlan(problem.InitialState, iw2Result.Plan).Expand()));
    }

    // =========================================================================
    // 8. IW(k): plan validity on real benchmarks (step-by-step execution)
    // =========================================================================

    // Ferry p01 (2 cars, 2 locations) has width 3: after transporting car1, the
    // state needed to transport car2 contains the pair (at-ferry(loc1), on(car2))
    // which was registered early from a symmetric branch — requiring a new triple.
    // Gripper/delivery/blocks_3 p01 all have width 2.
    [Theory]
    [InlineData("gripper", 2, "grounded")]
    [InlineData("delivery", 2, "grounded")]
    [InlineData("blocks_3", 2, "grounded")]
    [InlineData("ferry", 3, "grounded")]
    [InlineData("ferry", 3, "lifted")]
    public void IWk_PlanIsValid_OnRealBenchmarks(
        string domainDir,
        int k,
        string generatorType)
    {
        var problem = SearchTestHelpers.LoadProblem(domainDir,
            generatorType: Enum.Parse<ApplicableActionGeneratorType>(generatorType, true));
        var goalCondition = GoalCondition.FromProblem(problem);

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .BuildIw(k)
            .Search();

        Assert.True(result.IsSuccess, $"IW({k}) should find a plan for '{domainDir}'.");
        var state = problem.InitialState;
        foreach (var action in result.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()), $"Action '{action}' is not applicable in '{domainDir}'.");
            state = state.Expand().Apply(action);
        }
        Assert.True(goalCondition.IsSatisfied(state.Expand()), $"Goal not reached for '{domainDir}' with IW({k}).");
    }

    // =========================================================================
    // 9. IW(k): returns failure on genuinely unsolvable problems
    // =========================================================================

    [Theory]
    [InlineData("blocks_4", 1)]
    [InlineData("gripper", 1)]
    [InlineData("ferry", 2)]
    public void IWk_UnsolvableProblem_ReturnsFailure(string problemPrefix, int k)
    {
        var problem = SearchTestHelpers.LoadUnsolvableProblem(problemPrefix);

        var result = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .BuildIw(k)
            .Search();

        Assert.False(result.IsSuccess);
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Empty(result.Plan);
    }

    // =========================================================================
    // 10. PlannerFactory: "<generator>:iwN" spec creates a working planner
    // =========================================================================

    [Theory]
    [InlineData("iw0", "grounded")]
    [InlineData("iw1", "grounded")]
    [InlineData("iw2", "grounded")]
    [InlineData("iw3", "grounded")]
    [InlineData("iw0", "lifted")]
    [InlineData("iw1", "lifted")]
    [InlineData("iw2", "lifted")]
    [InlineData("iw3", "lifted")]
    public void PlannerFactory_IwSpec_CreatesWorkingPlanner(string spec, string generatorType)
    {
        // One-step domain is solvable by every IW(k) including k=0.
        Problem problem = CreateOneStepProblem(generatorType: generatorType);

        var result = PlannerFactory.CreateFromSpec(spec).Solve(problem);

        Assert.True(result.IsSuccess, $"Spec '{spec}' should produce a planner that solves the problem.");
        Assert.Single(result.Plan);
    }

}
