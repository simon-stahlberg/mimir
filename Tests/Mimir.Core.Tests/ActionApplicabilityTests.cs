using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class ActionApplicabilityTests
{
    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    [Fact]
    public void IsApplicable_PositiveFluentPreconditionMet_ReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p"]);
        Fact<Fluent> fact = fixture.RegisterFluent("p");
        State state = StateFactory.Default.Create(fixture.Context, [fact]);

        var action = CreateAction(fixture,
            positiveFluentPrec: CreateBitboardWords(fact),
            negativeFluentPrec: default);

        Assert.True(action.IsApplicable(state.Expand()));
    }

    [Fact]
    public void IsApplicable_PositiveFluentPreconditionNotMet_ReturnsFalse()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p"]);
        Fact<Fluent> fact = fixture.RegisterFluent("p");
        State emptyState = StateFactory.Default.CreateEmpty(fixture.Context);

        var action = CreateAction(fixture,
            positiveFluentPrec: CreateBitboardWords(fact),
            negativeFluentPrec: default);

        Assert.False(action.IsApplicable(emptyState.Expand()));
    }

    [Fact]
    public void IsApplicable_NegativeFluentPreconditionViolated_ReturnsFalse()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"]);
        Fact<Fluent> fact = fixture.RegisterFluent("q");
        State state = StateFactory.Default.Create(fixture.Context, [fact]);

        var action = CreateAction(fixture,
            positiveFluentPrec: default,
            negativeFluentPrec: CreateBitboardWords(fact));

        Assert.False(action.IsApplicable(state.Expand()));
    }

    [Fact]
    public void IsApplicable_NegativeFluentPreconditionSatisfied_ReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"]);
        Fact<Fluent> fact = fixture.RegisterFluent("q");
        State emptyState = StateFactory.Default.CreateEmpty(fixture.Context);

        var action = CreateAction(fixture,
            positiveFluentPrec: default,
            negativeFluentPrec: CreateBitboardWords(fact));

        Assert.True(action.IsApplicable(emptyState.Expand()));
    }

    [Fact]
    public void IsApplicable_HighOffsetFluentAndStaticPreconditionsUseGlobalIndices()
    {
        string[] fluentNames = Enumerable.Range(0, 132).Select(index => $"f-{index}").ToArray();
        string[] staticNames = Enumerable.Range(0, 132).Select(index => $"s-{index}").ToArray();
        TestProblemFixture fixture = TestProblemFixture.Create(fluentNames, staticNames);

        Fact<Fluent>[] fluents = fluentNames.Select(fixture.RegisterFluent).ToArray();
        Fact<Static>[] statics = staticNames.Select(fixture.RegisterStatic).ToArray();
        Fact<Fluent> requiredFluent = fluents[130];
        Fact<Fluent> forbiddenFluent = fluents[131];
        Fact<Static> requiredStatic = statics[130];
        Fact<Static> forbiddenStatic = statics[131];
        fixture.Context.SetStaticBitboardWords(CreateDenseBitboard(requiredStatic.LocalIndex));
        State state = StateFactory.Default.Create(fixture.Context, [requiredFluent]);

        Action action = CreateAction(
            fixture,
            CreateBitboardWords(requiredFluent),
            CreateBitboardWords(forbiddenFluent),
            OffsetBitboard.FromSetBitIndices([requiredStatic.LocalIndex]),
            OffsetBitboard.FromSetBitIndices([forbiddenStatic.LocalIndex]));

        Assert.True(action.IsApplicable(state.Expand()));
        Assert.False(action.IsApplicable(StateFactory.Default.CreateEmpty(fixture.Context).Expand()));
    }

    [Fact]
    public void IsApplicable_AllGroundedActionsFromGenerator_ApplicableInSameState()
    {
        var domainPath = Path.Combine(BasePath, "gripper", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "gripper", "p01.pddl");
        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());

        ExtendedState initialState = problem.InitialState.Expand();
        var actions = generator.GetApplicableActions(initialState).ToList();

        Assert.All(actions, a => Assert.True(a.IsApplicable(initialState)));
    }

    [Fact]
    public void IsApplicable_DifferentContext_ThrowsInvalidOperationException()
    {
        var domainPath1 = Path.Combine(BasePath, "gripper", "domain.pddl");
        var problemPath1 = Path.Combine(BasePath, "gripper", "p01.pddl");
        var domain1 = Domain.FromFile(domainPath1);
        var problem1 = Problem.FromFile(domain1, problemPath1);
        var gen1 = new GroundedApplicableActionGenerator(problem1, problem1.InitialState, new RpgGrounder());

        var domainPath2 = Path.Combine(BasePath, "ferry", "domain.pddl");
        var problemPath2 = Path.Combine(BasePath, "ferry", "p01.pddl");
        var domain2 = Domain.FromFile(domainPath2);
        var problem2 = Problem.FromFile(domain2, problemPath2);

        var action = gen1.GetApplicableActions(problem1.InitialState.Expand()).First();

        Assert.Throws<InvalidOperationException>(() => action.IsApplicable(problem2.InitialState.Expand()));
    }

    [Fact]
    public void IsApplicable_DerivedPreconditionSatisfied_ReturnsTrue()
    {
        Problem problem = CreateDerivedProblem();

        var actA = new RpgGrounder().Ground(problem, problem.InitialState).Single(a => a.Schema.Name == "act");

        Assert.True(actA.IsApplicable(problem.InitialState.Expand()));
    }

    [Fact]
    public void IsApplicable_DerivedPreconditionNotSatisfied_ReturnsFalse()
    {
        Problem problem = CreateDerivedProblem();

        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());
        ExtendedState initialState = problem.InitialState.Expand();
        var clearA = generator.GetApplicableActions(initialState).Single(a => a.Schema.Name == "clear");
        State stateWithoutP = initialState.Apply(clearA);

        var actA = new RpgGrounder().Ground(problem, problem.InitialState).Single(a => a.Schema.Name == "act");

        Assert.False(actA.IsApplicable(stateWithoutP.Expand()));
    }

    [Fact]
    public void IsApplicable_MixedStaticAndDynamicDerivedPreconditionsTrackStateChanges()
    {
        DomainBuilder domainBuilder = new DomainBuilder("mixed-derived-preconditions")
            .Requirements().Add(":strips").Add(":adl").Close()
            .Predicates()
                .Add("authorized", ("?x", "object"))
                .Add("prohibited", ("?x", "object"))
                .Add("enabled", ("?x", "object"))
                .Add("positive-done")
                .Add("negative-done")
                .Add("static-ready", ("?x", "object"))
                .Add("static-blocked", ("?x", "object"))
                .Add("dynamic-ready", ("?x", "object"))
                .Close();
        ActionListBuilder actionsBuilder = domainBuilder.Actions();
        actionsBuilder.Add("use-positive")
            .AddParameter("?x")
            .AddPrecondition("static-ready", "?x")
            .AddPrecondition("static-blocked", Polarity.Negative, "?x")
            .AddPrecondition("dynamic-ready", "?x")
            .AddEffect("positive-done")
            .Close();
        actionsBuilder.Add("use-negative")
            .AddParameter("?x")
            .AddPrecondition("static-ready", "?x")
            .AddPrecondition("static-blocked", Polarity.Negative, "?x")
            .AddPrecondition("dynamic-ready", Polarity.Negative, "?x")
            .AddEffect("negative-done")
            .Close();
        actionsBuilder.Add("disable")
            .AddParameter("?x")
            .AddPrecondition("enabled", "?x")
            .AddEffect("enabled", Polarity.Negative, "?x")
            .Close();
        actionsBuilder.Close();
        Domain domain = domainBuilder.DerivedPredicates()
            .Define("static-ready", Logic.Atom("authorized", "?x"))
            .Define("static-blocked", Logic.Atom("prohibited", "?x"))
            .Define("dynamic-ready", Logic.Atom("enabled", "?x"))
            .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "mixed-derived-preconditions-problem")
            .Objects().Add("item").Close()
            .InitialState().AddFact("authorized", "item").AddFact("enabled", "item").Close()
            .Goal().Add("positive-done").Close()
            .Build();
        Action[] actions = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .ToArray();
        Action usePositive = actions.Single(action => action.Schema.Name == "use-positive");
        Action useNegative = actions.Single(action => action.Schema.Name == "use-negative");
        Action disable = actions.Single(action => action.Schema.Name == "disable");
        ExtendedState initial = problem.InitialState.Expand();

        Assert.True(usePositive.IsApplicable(initial));
        Assert.False(useNegative.IsApplicable(initial));

        ExtendedState disabled = initial.Apply(disable).Expand();

        Assert.False(usePositive.IsApplicable(disabled));
        Assert.True(useNegative.IsApplicable(disabled));
    }

    private static Problem CreateDerivedProblem()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d")
            .Requirements().Add(":strips").Add(":adl").Close()
            .Predicates()
                .Add("p", ("?x", "object"))
                .Add("q", ("?x", "object"))
                .Add("derived-cond", ("?x", "object"))
                .Close();
        ActionListBuilder actionsBuilder = domainBuilder.Actions();
        actionsBuilder.Add("act")
            .AddParameter("?x")
            .AddPrecondition("derived-cond", "?x")
            .AddEffect("q", "?x")
            .Close();
        actionsBuilder.Add("clear")
            .AddParameter("?x")
            .AddPrecondition("p", "?x")
            .AddEffect("p", Polarity.Negative, "?x")
            .Close();
        actionsBuilder.Close();
        Domain domain = domainBuilder.DerivedPredicates()
            .Define("derived-cond", Logic.Atom("p", "?x"))
            .Close()
            .Build();

        return new ProblemBuilder(domain, "p")
            .Objects().Add("a").Close()
            .InitialState().AddFact("p", "a").Close()
            .Goal().Add("q", "a").Close()
            .Build();
    }

    private static OffsetBitboard CreateBitboardWords(params Fact<Fluent>[] facts)
    {
        return OffsetBitboard.FromSetBitIndices(facts.Select(fact => fact.LocalIndex).ToArray());
    }

    private static Action CreateAction(
        TestProblemFixture fixture,
        OffsetBitboard positiveFluentPrec,
        OffsetBitboard negativeFluentPrec,
        OffsetBitboard positiveStaticPrec = default,
        OffsetBitboard negativeStaticPrec = default)
    {
        InstanceContext context = fixture.Context;
        ActionSchema schema = fixture.ActionSchema;
        return new Action(
            context,
            schema,
            Array.Empty<Constant>(),
            positiveFluentPrec,
            negativeFluentPrec,
            positiveStaticPrec,
            negativeStaticPrec,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            Array.Empty<GroundConditionalEffect>(),
            1d);
    }

    private static ulong[] CreateDenseBitboard(int setBitIndex)
    {
        var words = new ulong[setBitIndex / 64 + 1];
        words[setBitIndex / 64] = 1UL << (setBitIndex % 64);
        return words;
    }
}
