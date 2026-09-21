using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class PlannerGeneratorParityTests
{
    [Fact]
    public void TransportP02_BfsParity_PreservesDepthOptimalCostlyRoute()
    {
        Problem CreateProblem(ApplicableActionGeneratorType type) => SearchTestHelpers.LoadProblem("transport", "p02.pddl", generatorType: type);

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "bfs");

        AssertPlannerParity( grounded, clique);
        Assert.Equal(3, grounded.PlanLength);
        Assert.Equal(3, clique.PlanLength);
        Assert.Equal(102d, grounded.PlanCost);
        Assert.Equal(102d, clique.PlanCost);
    }

    [Fact]
    public void TransportP02_UcsParity_FindsCheaperLongerRoute()
    {
        Problem CreateProblem(ApplicableActionGeneratorType type) => SearchTestHelpers.LoadProblem("transport", "p02.pddl", generatorType: type);

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "ucs");

        AssertPlannerParity( grounded, clique);
        Assert.Equal(5, grounded.PlanLength);
        Assert.Equal(5, clique.PlanLength);
        Assert.Equal(32d, grounded.PlanCost);
        Assert.Equal(32d, clique.PlanCost);
    }

    [Fact]
    public void TransportP02_AStarBlindParity_MatchesUcsCostOptimalRoute()
    {
        Problem CreateProblem(ApplicableActionGeneratorType type) => SearchTestHelpers.LoadProblem("transport", "p02.pddl", generatorType: type);

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "astar", "blind");

        AssertPlannerParity( grounded, clique);
        Assert.Equal(5, grounded.PlanLength);
        Assert.Equal(5, clique.PlanLength);
        Assert.Equal(32d, grounded.PlanCost);
        Assert.Equal(32d, clique.PlanCost);
    }

    [Fact]
    public void WoodworkingP01_UcsParity_UsesObjectSpecificNumericCosts()
    {
        Problem CreateProblem(ApplicableActionGeneratorType type) => SearchTestHelpers.LoadProblem("woodworking", generatorType: type);

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "ucs");

        AssertPlannerParity( grounded, clique);
    }

    [Fact]
    public void MiconicSimpleAdl_BfsParity_StaticAboveAndConditionalStopEffects()
    {
        Problem CreateProblem(ApplicableActionGeneratorType type) => SearchTestHelpers.LoadProblem("miconic-simpleadl", generatorType: type);

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "bfs");

        AssertPlannerParity( grounded, clique);
    }

    [Fact]
    public void FalseDerivedShortcut_BfsParity_DoesNotSolveThroughInvalidDerivedAction()
    {
        DomainBuilder domainBuilder = new DomainBuilder("false-derived-shortcut-parity")
            .Requirements().Add(":strips").Add(":adl").Close()
            .Predicates()
                .Add("start").Add("mid").Add("done").Add("missing").Add("shortcut-open")
                .Close();
        ActionListBuilder actions = domainBuilder.Actions();
        actions.Add("shortcut").AddPrecondition("start").AddPrecondition("shortcut-open")
            .AddEffect("done").Close();
        actions.Add("advance").AddPrecondition("start").AddEffect("mid")
            .AddEffect("start", Polarity.Negative).Close();
        actions.Add("finish").AddPrecondition("mid").AddEffect("done").Close();
        actions.Close();
        Domain domain = domainBuilder.DerivedPredicates()
            .Define("shortcut-open", Logic.Atom("missing")).Close().Build();
        Problem CreateProblem(ApplicableActionGeneratorType type) => new ProblemBuilder(domain, "false-derived-shortcut-parity-problem", type)
            .InitialState().AddFact("start").Close()
            .Goal().Add("done").Close().Build();

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "bfs");

        AssertPlannerParity( grounded, clique);
        AssertPlanSteps(new[] { "(advance)", "(finish)" }, grounded);
        AssertPlanSteps(new[] { "(advance)", "(finish)" }, clique);
    }

    [Fact]
    public void QuantifiedDerivedGate_BfsParity_FinishOnlyAfterAllItemsChecked()
    {
        DomainBuilder domainBuilder = new DomainBuilder("quantified-derived-gate-parity")
            .Requirements().Add(":strips").Add(":adl").Add(":typing").Add(":negative-preconditions").Close()
            .Types().Add("item").Close()
            .Predicates()
                .Add("checked", ("?x", "item")).Add("done").Add("all-checked").Close();
        ActionListBuilder actions = domainBuilder.Actions();
        actions.Add("check").AddParameter("?x", "item")
            .AddPrecondition("checked", Polarity.Negative, "?x").AddEffect("checked", "?x").Close();
        actions.Add("finish").AddPrecondition("all-checked").AddEffect("done").Close();
        actions.Close();
        Domain domain = domainBuilder.DerivedPredicates()
            .Define("all-checked", Logic.Forall(
                new[] { ("?x", "item") },
                Logic.Atom("checked", "?x")))
            .Close().Build();
        Problem CreateProblem(ApplicableActionGeneratorType type) => new ProblemBuilder(domain, "quantified-derived-gate-parity-problem", type)
            .Objects().Add("i1", "item").Add("i2", "item").Close()
            .Goal().Add("done").Close().Build();

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "bfs");

        AssertPlannerParity( grounded, clique);
        Assert.Equal(3, grounded.PlanLength);
        Assert.Equal(3, clique.PlanLength);
        Assert.Equal("finish", grounded.Plan[^1].Schema.Name);
        Assert.Equal("finish", clique.Plan[^1].Schema.Name);
    }

    [Fact]
    public void DerivedConditionalEffect_BfsParity_ConditionalEffectCanAchieveGoal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-conditional-effect-parity")
            .Requirements().Add(":strips").Add(":adl").Add(":conditional-effects").Close()
            .Predicates().Add("base").Add("goal").Add("enabled-derived").Close();
        ActionListBuilder actions = domainBuilder.Actions();
        actions.Add("make-base").AddEffect("base").Close();
        ActionSchemaBuilder trigger = actions.Add("trigger");
        trigger.AddConditionalEffect().AddCondition("enabled-derived").AddEffect("goal").Close();
        trigger.Close();
        actions.Add("clear-base").AddPrecondition("base").AddEffect("base", Polarity.Negative).Close();
        actions.Close();
        Domain domain = domainBuilder.DerivedPredicates()
            .Define("enabled-derived", Logic.Atom("base")).Close().Build();
        Problem CreateProblem(ApplicableActionGeneratorType type) => new ProblemBuilder(domain, "derived-conditional-effect-parity-problem", type)
            .Goal().Add("goal").Close().Build();

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "bfs");

        AssertPlannerParity( grounded, clique);
        AssertPlanSteps(new[] { "(make-base)", "(trigger)" }, grounded);
        AssertPlanSteps(new[] { "(make-base)", "(trigger)" }, clique);
    }

    [Fact]
    public void StaticPruningMaze_BfsParity_OnlyStaticConnectedEdgesAreUsable()
    {
        Domain domain = new DomainBuilder("static-pruning-maze-parity")
            .Requirements().Add(":strips").Add(":typing").Close()
            .Types().Add("location").Close()
            .Predicates()
                .Add("at", ("?x", "location"))
                .Add("connected", ("?from", "location"), ("?to", "location"))
                .Add("visited", ("?x", "location"))
                .Close()
            .Actions()
                .Add("move")
                    .AddParameter("?from", "location").AddParameter("?to", "location")
                    .AddPrecondition("at", "?from").AddPrecondition("connected", "?from", "?to")
                    .AddEffect("at", Polarity.Negative, "?from")
                    .AddEffect("at", "?to").AddEffect("visited", "?to").Close()
                .Close().Build();
        Problem CreateProblem(ApplicableActionGeneratorType type) => new ProblemBuilder(domain, "static-pruning-maze-parity-problem", type)
            .Objects()
                .Add("start", "location").Add("mid", "location").Add("goal", "location")
                .Add("trap-a", "location").Add("trap-b", "location").Close()
            .InitialState()
                .AddFact("at", "start").AddFact("visited", "start")
                .AddFact("connected", "start", "mid").AddFact("connected", "mid", "goal")
                .AddFact("connected", "trap-a", "goal").AddFact("connected", "start", "trap-b")
                .Close()
            .Goal().Add("at", "goal").Add("visited", "goal").Close().Build();

        var (grounded, clique) = SolveWithBothGenerators(CreateProblem, "bfs");

        AssertPlannerParity( grounded, clique);
        Assert.Equal(2, grounded.PlanLength);
        Assert.Equal(2, clique.PlanLength);
        AssertPlanSteps(new[] { "(move start mid)", "(move mid goal)" }, grounded);
        AssertPlanSteps(new[] { "(move start mid)", "(move mid goal)" }, clique);
    }

    private static (PlanResult Grounded, PlanResult Clique) SolveWithBothGenerators(
        Func<ApplicableActionGeneratorType, Problem> createProblem,
        string algorithm,
        string heuristic = "blind")
    {
        var grounded = PlannerFactory.Create(algorithm, heuristic).Solve(createProblem(ApplicableActionGeneratorType.Grounded));
        var clique = PlannerFactory.Create(algorithm, heuristic).Solve(createProblem(ApplicableActionGeneratorType.Lifted));
        return (grounded, clique);
    }

    private static void AssertPlannerParity(PlanResult grounded, PlanResult clique)
    {
        Assert.Equal(grounded.IsSuccess, clique.IsSuccess);
        Assert.Equal(grounded.PlanLength, clique.PlanLength);
        Assert.Equal(grounded.PlanCost, clique.PlanCost, precision: 10);

        if (!grounded.IsSuccess)
        {
            Assert.Empty(grounded.Plan);
            Assert.Empty(clique.Plan);
            return;
        }

        AssertPlanReachesGoal(grounded.Plan[0].Context.Problem, grounded.Plan);
        AssertPlanReachesGoal(clique.Plan[0].Context.Problem, clique.Plan);
    }

    private static void AssertPlanSteps(string[] expected, PlanResult result)
    {
        Assert.True(result.IsSuccess);
        Assert.Equal(expected, SearchTestHelpers.PlanSteps(result.Plan));
    }

    private static void AssertPlanReachesGoal(Problem problem, IReadOnlyList<GroundAction> plan)
    {
        var goal = GoalCondition.FromProblem(problem);
        State state = problem.InitialState;
        foreach (var action in plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goal.IsSatisfied(state.Expand()));
    }
}
