using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class ActionGroundingPlanTests
{
    [Fact]
    public void SharedDomain_UsesSeparateContextPlansObjectsAndNumericValues()
    {
        Domain domain = new DomainBuilder("shared-plan")
            .Requirements().Add(":strips").Add(":typing").Add(":action-costs").Close()
            .Types().Add("item").Close()
            .Predicates().Add("ready", ("?x", "item")).Add("done", ("?x", "item")).Close()
            .Functions().Add("weight", ("?x", "item")).Close()
            .Actions()
                .Add("use").AddParameter("?x", "item").AddPrecondition("ready", "?x")
                    .AddEffect("done", "?x").WithCost(Numeric.Function("weight", "?x")).Close()
                .Close().Build();
        Problem first = CreateSharedProblem(domain, "first", 2);
        Problem second = CreateSharedProblem(domain, "second", 7);
        GroundAction firstAction = Assert.Single(
            new RpgGrounder().Ground(first, first.InitialState));
        GroundAction secondAction = Assert.Single(
            new RpgGrounder().Ground(second, second.InitialState));

        Assert.Same(first.Context, firstAction.Context);
        Assert.Same(second.Context, secondAction.Context);
        Assert.NotSame(firstAction.Arguments[0], secondAction.Arguments[0]);
        Assert.Equal(2d, firstAction.Cost);
        Assert.Equal(7d, secondAction.Cost);
    }

    [Fact]
    public void FailedCost_ReturnsCleanWorkspaceForNextAction()
    {
        DomainBuilder builder = new DomainBuilder("failed-cost-cleanup")
            .Requirements().Add(":strips").Add(":action-costs").Close()
            .Predicates().Add("bad-effect").Add("good-effect").Close()
            .Functions().Add("penalty").Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("bad").AddEffect("bad-effect").WithCost(Numeric.Function("penalty")).Close();
        actions.Add("good").AddEffect("good-effect").Close();
        Domain domain = actions.Close().Build();
        Problem problem = new ProblemBuilder(domain, "failed-cost-cleanup-problem")
            .InitialState().SetValue(Numeric.Function("penalty"), -1d).Close()
            .Build();
        ActionSchema bad = domain.Actions.Single(action => action.Name == "bad");
        ActionSchema good = domain.Actions.Single(action => action.Name == "good");
        Assert.Throws<InvalidOperationException>(
            () => ActionBuilder.BuildAction(
                bad,
                Array.Empty<Constant>(),
                problem));

        Assert.Null(problem.Context.FindFact(
            domain.Fluents.Single(predicate => predicate.Name == "bad-effect"),
            Array.Empty<Constant>()));

        GroundAction action = ActionBuilder.BuildAction(
            good,
            Array.Empty<Constant>(),
            problem);

        Fact<Fluent> goodEffect = Assert.IsType<Fact<Fluent>>(
            problem.Context.FindFact(
                domain.Fluents.Single(predicate => predicate.Name == "good-effect"),
                Array.Empty<Constant>()));
        Assert.True(problem.InitialState.Expand().Apply(action).IsTrue(goodEffect));
    }

    [Fact]
    public void BuildAction_CopiesMutableCallerArguments()
    {
        Domain domain = new DomainBuilder("copied-action-arguments")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("done", ("?x", "object")).Close()
            .Actions().Add("use").AddParameter("?x").AddEffect("done", "?x").Close().Close().Build();
        Problem problem = new ProblemBuilder(domain, "copied-action-arguments-problem")
            .Objects().Add("a").Add("b").Close().Build();
        var arguments = new[] { problem.ObjectLookup["a"] };

        GroundAction action = ActionBuilder.BuildAction(
            Assert.Single(domain.Actions),
            arguments,
            problem);
        arguments[0] = problem.ObjectLookup["b"];

        Assert.Equal("a", Assert.Single(action.Arguments).Name);
    }

    [Fact]
    public void QuantifiedEffectBinding_DoesNotLeakIntoLaterEffect()
    {
        DomainBuilder builder = new DomainBuilder("quantified-shadow")
            .Requirements().Add(":adl").Close()
            .Predicates().Add("inner", ("?x", "object")).Add("outer", ("?x", "object")).Close();
        ActionSchemaBuilder apply = builder.Actions().Add("apply").AddParameter("?outer");
        apply.AddConditionalEffect().AddParameter("?inner").AddEffect("inner", "?inner").Close();
        apply.AddConditionalEffect().AddEffect("outer", "?outer").Close();
        Domain domain = apply.Close().Close().Build();
        Problem problem = new ProblemBuilder(domain, "quantified-shadow-problem")
            .Objects().Add("a").Add("b").Close().Build();
        ActionSchema schema = Assert.Single(domain.Actions);
        GroundAction action = ActionBuilder.BuildAction(
            schema,
            [problem.ObjectLookup["a"]],
            problem);

        State successor = problem.InitialState.Expand().Apply(action);
        Mimir.Core.Schemas.Predicate<Fluent> inner = domain.Fluents.Single(predicate => predicate.Name == "inner");
        Mimir.Core.Schemas.Predicate<Fluent> outer = domain.Fluents.Single(predicate => predicate.Name == "outer");
        Fact<Fluent> outerB = problem.Context.RegisterFact(outer, [problem.ObjectLookup["b"]]);

        Assert.True(successor.IsTrue(problem.Context.FindFact(inner, [problem.ObjectLookup["a"]])!));
        Assert.True(successor.IsTrue(problem.Context.FindFact(inner, [problem.ObjectLookup["b"]])!));
        Assert.True(successor.IsTrue(problem.Context.FindFact(outer, [problem.ObjectLookup["a"]])!));
        Assert.False(successor.IsTrue(outerB));
    }

    private static Problem CreateSharedProblem(
        Domain domain,
        string name,
        int weight)
        => new ProblemBuilder(domain, name)
            .Objects().Add("item", "item").Close()
            .InitialState()
                .AddFact("ready", "item")
                .SetValue(Numeric.Function("weight", "item"), weight).Close()
            .Build();

}
