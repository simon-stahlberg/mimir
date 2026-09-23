using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class StateTests
{
    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    [Fact]
    public void InitialStateMaterializedBeforeGrounding_RemainsUsableAfterFactUniverseGrows()
    {
        var domainPath = Path.Combine(BasePath, "gripper", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "gripper", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);

        var staleInitialState = problem.InitialState;
        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());
        var initialStateAfterGrounding = problem.InitialState;

        var staleApplicable = generator.GetApplicableActions(staleInitialState.Expand())
            .Select(ActionKey)
            .OrderBy(key => key)
            .ToArray();
        var currentApplicable = generator.GetApplicableActions(initialStateAfterGrounding.Expand())
            .Select(ActionKey)
            .OrderBy(key => key)
            .ToArray();

        Assert.Equal(currentApplicable, staleApplicable);
    }

    [Fact]
    public void ApplyingActionToInitialStateMaterializedBeforeGrounding_ProducesCorrectSuccessor()
    {
        var domainPath = Path.Combine(BasePath, "gripper", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "gripper", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);

        var staleInitialState = problem.InitialState;
        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());
        var initialStateAfterGrounding = problem.InitialState;
        var action = generator.GetApplicableActions(initialStateAfterGrounding.Expand()).First();

        var successorFromStale = staleInitialState.Expand().Apply(action);
        var successorFromCurrent = initialStateAfterGrounding.Expand().Apply(action);

        Assert.Equal(successorFromCurrent, successorFromStale);
        Assert.Equal(
            successorFromCurrent.GetTrueFacts().Select(FactKey).OrderBy(key => key),
            successorFromStale.GetTrueFacts().Select(FactKey).OrderBy(key => key));
    }

    [Fact]
    public void InitialStateMaterializedAfterFactUniverseGrows_OmitsTrailingFalseFactWords()
    {
        string[] objectNames = Enumerable.Range(0, 130).Select(index => $"o{index}").ToArray();
        Domain domain = new DomainBuilder("compact-initial-state")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("present", ("?object", "object")).Close()
            .Actions()
                .Add("make-present").AddParameter("?object").AddEffect("present", "?object").Close()
                .Close()
            .Build();
        var problemBuilder = new ProblemBuilder(domain, "compact-initial-state-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        foreach (string name in objectNames)
            objects.Add(name);
        objects.Close();
        problemBuilder.InitialState().AddFact("present", "o0").Close();
        Problem problem = problemBuilder.Build();
        Mimir.Core.Schemas.Predicate<Fluent> predicate = Assert.Single(domain.Fluents);
        foreach (Constant obj in problem.AllObjects)
            problem.Context.RegisterFact(predicate, [obj]);

        State initialState = problem.InitialState;

        Assert.True(initialState.IsTrue(problem.Context.FindFact(predicate, [problem.AllObjects[0]])!));
        Assert.False(initialState.IsTrue(problem.Context.FindFact(predicate, [problem.AllObjects[^1]])!));
    }

    [Fact]
    public void Apply_WidensStateForUnconditionalEffects_WhenAddingLowerAndHigherFacts()
    {
        var fixture = CreateWideningFixture();
        var action = CreateActionWithAddEffects(
            fixture.Owner,
            fixture.LowFact,
            fixture.HighFact);

        var nextState = fixture.InitialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(fixture.LowFact));
        Assert.True(nextState.IsTrue(fixture.MiddleFact));
        Assert.True(nextState.IsTrue(fixture.HighFact));
    }

    [Fact]
    public void Apply_WidensStateForConditionalEffects_WhenAddingLowerAndHigherFacts()
    {
        var fixture = CreateWideningFixture();
        var conditionWords = CreateBitboardWords(fixture.MiddleFact);
        var conditionalEffects = new GroundConditionalEffect[]
        {
            new(
                fixture.Owner.Context,
                conditionWords,
                default,
                default,
                default,
                Array.Empty<Literal<Fact<Derived>>>(),
                [],
                new Literal<Fact<Fluent>>(fixture.LowFact, Polarity.Positive)),
            new(
                fixture.Owner.Context,
                conditionWords,
                default,
                default,
                default,
                Array.Empty<Literal<Fact<Derived>>>(),
                [],
                new Literal<Fact<Fluent>>(fixture.HighFact, Polarity.Positive))
        };
        var action = CreateAction(
            fixture.Owner,
            default,
            default,
            conditionalEffects);

        var nextState = fixture.InitialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(fixture.LowFact));
        Assert.True(nextState.IsTrue(fixture.MiddleFact));
        Assert.True(nextState.IsTrue(fixture.HighFact));
    }

    [Fact]
    public void Apply_HighOffsetEffectPreservesLowerStateWords()
    {
        var fixture = CreateWideningFixture();
        Action action = CreateActionWithAddEffects(fixture.Owner, fixture.HighFact);

        State nextState = fixture.InitialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(fixture.MiddleFact));
        Assert.True(nextState.IsTrue(fixture.HighFact));
    }

    [Fact]
    public void Apply_DoesNotWidenForUntriggeredHighConditionalEffect()
    {
        var fixture = CreateWideningFixture();
        GroundConditionalEffect conditionalEffect = CreateConditionalEffect(
            fixture.LowFact,
            fixture.HighFact,
            Polarity.Positive);
        Action action = CreateAction(fixture.Owner, default, default, [conditionalEffect]);

        State nextState = fixture.InitialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(fixture.MiddleFact));
        Assert.False(nextState.IsTrue(fixture.HighFact));
    }

    [Fact]
    public void Apply_HighUnconditionalDeleteLeavesMissingFactFalse()
    {
        var fixture = CreateHighDeleteFixture();
        Action action = CreateAction(
            fixture.Context,
            fixture.ActionSchema,
            default,
            CreateBitboardWords(fixture.HighFact),
            Array.Empty<GroundConditionalEffect>());

        ExtendedState initialState = fixture.InitialState.Expand();
        State nextState = initialState.Apply(action);
        Assert.Equal(fixture.InitialState, nextState);
    }

    [Fact]
    public void Apply_HighConditionalDeleteLeavesMissingFactFalse()
    {
        var fixture = CreateHighDeleteFixture();
        GroundConditionalEffect conditionalDelete = CreateConditionalEffect(
            fixture.ConditionFact,
            fixture.HighFact,
            Polarity.Negative);
        Action action = CreateAction(
            fixture.Context,
            fixture.ActionSchema,
            default,
            default,
            new[] { conditionalDelete });

        ExtendedState initialState = fixture.InitialState.Expand();
        State nextState = initialState.Apply(action);
        Assert.Equal(fixture.InitialState, nextState);
    }

    [Fact]
    public void Apply_UnconditionalAddWinsOverSatisfiedConditionalDelete()
    {
        TestProblemFixture owner = TestProblemFixture.Create(["condition", "effect"]);
        Fact<Fluent> conditionFact = owner.RegisterFluent("condition");
        Fact<Fluent> effectFact = owner.RegisterFluent("effect");
        State initialState = StateFactory.Default.Create(owner.Context, [conditionFact]);
        GroundConditionalEffect conditionalDelete = CreateConditionalEffect(conditionFact, effectFact, Polarity.Negative);
        Action action = CreateAction(
            owner,
            CreateBitboardWords(effectFact),
            default,
            new[] { conditionalDelete });

        State nextState = initialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(effectFact));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void Apply_ConditionalAddWinsOverConditionalDeleteRegardlessOfOrder(bool addFirst)
    {
        TestProblemFixture owner = TestProblemFixture.Create(["condition", "effect"]);
        Fact<Fluent> conditionFact = owner.RegisterFluent("condition");
        Fact<Fluent> effectFact = owner.RegisterFluent("effect");
        State initialState = StateFactory.Default.Create(owner.Context, [conditionFact]);
        GroundConditionalEffect conditionalAdd = CreateConditionalEffect(conditionFact, effectFact, Polarity.Positive);
        GroundConditionalEffect conditionalDelete = CreateConditionalEffect(conditionFact, effectFact, Polarity.Negative);
        GroundConditionalEffect[] conditionalEffects = addFirst
            ? new[] { conditionalAdd, conditionalDelete }
            : new[] { conditionalDelete, conditionalAdd };
        Action action = CreateAction(
            owner,
            default,
            default,
            conditionalEffects);

        State nextState = initialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(effectFact));
    }

    [Fact]
    public void Apply_ConditionalEffectsAreEvaluatedAgainstSourceState()
    {
        TestProblemFixture owner = TestProblemFixture.Create(["condition", "effect"]);
        Fact<Fluent> conditionFact = owner.RegisterFluent("condition");
        Fact<Fluent> effectFact = owner.RegisterFluent("effect");
        State initialState = StateFactory.Default.CreateEmpty(owner.Context);
        GroundConditionalEffect conditionalAdd = CreateConditionalEffect(conditionFact, effectFact, Polarity.Positive);
        Action action = CreateAction(
            owner,
            CreateBitboardWords(conditionFact),
            default,
            new[] { conditionalAdd });

        State nextState = initialState.Expand().Apply(action);

        Assert.True(nextState.IsTrue(conditionFact));
        Assert.False(nextState.IsTrue(effectFact));
    }

    private static (TestProblemFixture Owner, State InitialState, Fact<Fluent> LowFact, Fact<Fluent> MiddleFact, Fact<Fluent> HighFact) CreateWideningFixture()
    {
        var names = new List<string> { "low", "middle" };
        names.AddRange(Enumerable.Range(2, 63).Select(index => $"filler-{index}"));
        names.Add("high");
        TestProblemFixture owner = TestProblemFixture.Create(names);
        InstanceContext context = owner.Context;
        Fact<Fluent> lowFact = owner.RegisterFluent("low");
        Fact<Fluent> middleFact = owner.RegisterFluent("middle");
        State initialState = StateFactory.Default.Create(context, [middleFact]);

        for (int i = 2; i < 65; i++)
        {
            owner.RegisterFluent($"filler-{i}");
        }

        Fact<Fluent> highFact = owner.RegisterFluent("high");

        return (owner, initialState, lowFact, middleFact, highFact);
    }

    private static (InstanceContext Context, ActionSchema ActionSchema, State InitialState, Fact<Fluent> ConditionFact, Fact<Fluent> HighFact) CreateHighDeleteFixture()
    {
        string[] objectNames = Enumerable.Range(0, 91).Select(index => $"o{index}").ToArray();
        var domainBuilder = new DomainBuilder("high-delete");
        domainBuilder.Requirements().Add(":strips").Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        foreach (string name in objectNames)
            constants.Add(name);
        constants.Close();
        Domain domain = domainBuilder
            .Predicates().Add("value", ("?left", "object"), ("?right", "object")).Close()
            .Actions().Add("make-value").AddEffect("value", "o0", "o0").Close().Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "high-delete-problem").Build();
        Mimir.Core.Schemas.Predicate<Fluent> predicate = Assert.Single(domain.Fluents);
        Constant firstObject = problem.AllObjects[0];
        Fact<Fluent> conditionFact = problem.Context.RegisterFact(
            predicate,
            [firstObject, firstObject]);
        State initialState = StateFactory.Default.Create(problem.Context, [conditionFact]);
        Fact<Fluent> highFact = conditionFact;

        foreach (Constant left in problem.AllObjects)
        {
            foreach (Constant right in problem.AllObjects)
            {
                if (ReferenceEquals(left, firstObject) && ReferenceEquals(right, firstObject))
                    continue;

                Fact<Fluent> fact = problem.Context.RegisterFact(predicate, [left, right]);
                highFact = fact;
            }
        }

        return (problem.Context, Assert.Single(domain.Actions), initialState, conditionFact, highFact);
    }

    private static GroundConditionalEffect CreateConditionalEffect(
        Fact<Fluent> conditionFact,
        Fact<Fluent> effectFact,
        Polarity polarity)
    {
        return new GroundConditionalEffect(
            conditionFact.Context,
            CreateBitboardWords(conditionFact),
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            [],
            new Literal<Fact<Fluent>>(effectFact, polarity));
    }

    private static Action CreateActionWithAddEffects(
        TestProblemFixture owner,
        params Fact<Fluent>[] addFacts)
    {
        return CreateAction(
            owner,
            CreateBitboardWords(addFacts),
            default,
            Array.Empty<GroundConditionalEffect>());
    }

    private static Action CreateAction(
        TestProblemFixture owner,
        OffsetBitboard addEffects,
        OffsetBitboard deleteEffects,
        IReadOnlyList<GroundConditionalEffect> conditionalEffects)
    {
        return CreateAction(
            owner.Context,
            owner.ActionSchema,
            addEffects,
            deleteEffects,
            conditionalEffects);
    }

    private static Action CreateAction(
        InstanceContext context,
        ActionSchema schema,
        OffsetBitboard addEffects,
        OffsetBitboard deleteEffects,
        IReadOnlyList<GroundConditionalEffect> conditionalEffects)
    {
        return TestActions.Create(
            context,
            schema,
            Array.Empty<Constant>(),
            default,
            default,
            default,
            default,
            addEffects,
            deleteEffects,
            Array.Empty<Literal<Fact<Derived>>>(),
            conditionalEffects,
            1d);
    }

    [Fact]
    public void IsTrue_FluentFact_ReturnsTrueForSetFact()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p"]);
        Fact<Fluent> fact = fixture.RegisterFluent("p");

        State state = StateFactory.Default.Create(fixture.Context, [fact]);

        Assert.True(state.IsTrue(fact));
    }

    [Fact]
    public void ConstructorCopiesTheSuppliedBitboard()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p"]);
        Fact<Fluent> fact = fixture.RegisterFluent("p");
        ulong[] bitboard = [1UL];
        var state = new State(fixture.Context, bitboard, []);
        int hashCode = state.GetHashCode();
        var states = new HashSet<State> { state };

        bitboard[0] = 0UL;

        Assert.True(state.IsTrue(fact));
        Assert.Equal(hashCode, state.GetHashCode());
        Assert.Contains(state, states);
    }

    [Fact]
    public void EqualStatesWithTrailingZeroWords_HaveEqualHashCodes()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p"]);
        State compact = new(fixture.Context, [1UL], []);
        State padded = new(fixture.Context, [1UL, 0UL, 0UL], []);

        Assert.Equal(compact, padded);
        Assert.Equal(compact.GetHashCode(), padded.GetHashCode());
    }

    [Fact]
    public void IsTrue_StaticFact_ReturnsItsStaticState()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(staticNames: ["s"]);
        Fact<Static> fact = fixture.RegisterStatic("s");
        State state = StateFactory.Default.CreateEmpty(fixture.Context);

        Assert.False(state.Expand().IsTrue(fact));

        fixture.Context.SetStaticBitboardWords([1UL]);
        Assert.True(state.Expand().IsTrue(fact));
    }

    [Fact]
    public void StaticTruthAssignmentCopiesTheSuppliedBitboard()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(staticNames: ["s"]);
        Fact<Static> fact = fixture.RegisterStatic("s");
        ulong[] bitboard = [1UL];

        fixture.Context.SetStaticBitboardWords(bitboard);
        bitboard[0] = 0UL;

        Assert.True(StateFactory.Default.CreateEmpty(fixture.Context).Expand().IsTrue(fact));
    }

    [Fact]
    public void IsTrue_ForeignFact_ThrowsArgumentException()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["registered"]);
        TestProblemFixture foreignFixture = TestProblemFixture.Create(["registered"]);
        Fact<Fluent> foreignFact = foreignFixture.RegisterFluent("registered");
        State state = StateFactory.Default.CreateEmpty(fixture.Context);

        ArgumentException exception = Assert.Throws<ArgumentException>(() => state.IsTrue(foreignFact));

        Assert.Equal("fact", exception.ParamName);
    }

    private static OffsetBitboard CreateBitboardWords(params Fact<Fluent>[] facts)
    {
        return OffsetBitboard.FromSetBitIndices(facts.Select(fact => fact.LocalIndex).ToArray());
    }

    private static string ActionKey(Action action)
        => $"{action.Schema.Name}({string.Join(',', action.Arguments.Select(argument => argument.Name))})";

    private static string FactKey(Fact<Fluent> fact)
        => $"{fact.Predicate.Name}({string.Join(',', fact.Arguments.Select(argument => argument.Name))})";
}
