using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public sealed class ExtendedStateTests
{
    [Fact]
    public void EqualCompactStatesProduceEqualExtendedStates()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["present"]);
        Fact<Fluent> present = fixture.RegisterFluent("present");
        State firstState = StateFactory.Default.Create(fixture.Context, [present]);
        State secondState = StateFactory.Default.Create(fixture.Context, [present]);

        ExtendedState first = firstState.Expand();
        ExtendedState second = secondState.Expand();

        Assert.NotSame(first, second);
        Assert.Same(firstState, first.State);
        Assert.Equal(first, second);
        Assert.Equal(first.GetHashCode(), second.GetHashCode());
        Assert.Single(new HashSet<ExtendedState> { first, second });
    }

    [Fact]
    public void StaticClosurePersistsWhileStateDependentClosureChanges()
    {
        Problem problem = CreateProblemForStaticClosurePersistsWhileStateDependentClosureChanges();
        DerivedPredicate staticReady = problem.Domain.Derived
            .Single(predicate => predicate.Name == "static-ready");
        DerivedPredicate dynamicReady = problem.Domain.Derived
            .Single(predicate => predicate.Name == "dynamic-ready");
        Fact<Derived> staticReadyFact = problem.Context.RegisterFact(
            staticReady,
            Array.Empty<Constant>());
        Fact<Derived> dynamicReadyFact = problem.Context.RegisterFact(
            dynamicReady,
            Array.Empty<Constant>());
        ExtendedState initial = problem.InitialState.Expand();
        GroundAction clear = Assert.Single(
            new GroundedApplicableActionGenerator(
                problem,
                problem.InitialState,
                new RpgGrounder())
                .GetApplicableActions(initial));

        ExtendedState successor = initial.Apply(clear).Expand();

        Assert.True(initial.IsTrue(staticReadyFact));
        Assert.True(initial.IsTrue(dynamicReadyFact));
        Assert.True(successor.IsTrue(staticReadyFact));
        Assert.False(successor.IsTrue(dynamicReadyFact));
    }

    [Fact]
    public void StratifiedNegationReadsTheCompletedLowerComponent()
    {
        Problem problem = CreateProblemForStratifiedNegationReadsTheCompletedLowerComponent();
        DerivedPredicate lower = problem.Domain.Derived
            .Single(predicate => predicate.Name == "lower");
        DerivedPredicate upper = problem.Domain.Derived
            .Single(predicate => predicate.Name == "upper");
        Fact<Derived> lowerFact = problem.Context.RegisterFact(
            lower,
            Array.Empty<Constant>());
        Fact<Derived> upperFact = problem.Context.RegisterFact(
            upper,
            Array.Empty<Constant>());

        ExtendedState state = problem.InitialState.Expand();

        Assert.False(state.IsTrue(upperFact));
        Assert.True(state.IsTrue(lowerFact));
        Assert.False(state.IsTrue(upperFact));
    }

    [Fact]
    public void StateDependenceIsClassifiedTransitively()
    {
        Problem problem = CreateProblemForStateDependenceIsClassifiedTransitively();
        DerivedPredicate staticBase = problem.Domain.Derived
            .Single(predicate => predicate.Name == "static-base");
        DerivedPredicate staticTransitive = problem.Domain.Derived
            .Single(predicate => predicate.Name == "static-transitive");
        DerivedPredicate dynamicBase = problem.Domain.Derived
            .Single(predicate => predicate.Name == "dynamic-base");
        DerivedPredicate dynamicTransitive = problem.Domain.Derived
            .Single(predicate => predicate.Name == "dynamic-transitive");

        Assert.True(problem.Context.IsStaticDerived(staticBase));
        Assert.True(problem.Context.IsStaticDerived(staticTransitive));
        Assert.False(problem.Context.IsStaticDerived(dynamicBase));
        Assert.False(problem.Context.IsStaticDerived(dynamicTransitive));
    }

    [Fact]
    public void ConditionalDerivedGuardUsesTheSourceClosure()
    {
        Problem problem = CreateProblemForConditionalDerivedGuardUsesTheSourceClosure();
        GroundAction activate = Assert.Single(
            new RpgGrounder().Ground(problem, problem.InitialState));
        FluentPredicate triggerPredicate = problem.Domain.Fluents
            .Single(predicate => predicate.Name == "trigger");
        FluentPredicate resultPredicate = problem.Domain.Fluents
            .Single(predicate => predicate.Name == "result");
        Fact<Fluent> trigger = problem.Context.RegisterFact(
            triggerPredicate,
            Array.Empty<Constant>());
        Fact<Fluent> result = problem.Context.RegisterFact(
            resultPredicate,
            Array.Empty<Constant>());

        State firstSuccessor = problem.InitialState.Expand().Apply(activate);
        State secondSuccessor = firstSuccessor.Expand().Apply(activate);

        Assert.True(firstSuccessor.IsTrue(trigger));
        Assert.False(firstSuccessor.IsTrue(result));
        Assert.True(secondSuccessor.IsTrue(result));
    }

    [Fact]
    public void RepeatedExpansionsHaveIndependentDerivedEvaluations()
    {
        Problem problem = CreateProblemForRepeatedExpansionsHaveIndependentDerivedEvaluations();
        DerivedPredicate reachable = problem.Domain.Derived.Single();
        Fact<Derived> reachableFact = problem.Context.RegisterFact(
            reachable,
            [problem.ObjectLookup["value"]]);
        int registeredDerivedCount = problem.Context.Derived.Count;
        ExtendedState first = problem.InitialState.Expand();
        ExtendedState second = problem.InitialState.Expand();

        Assert.NotSame(first, second);
        Assert.True(first.IsTrue(reachableFact));
        Assert.True(second.IsTrue(reachableFact));
        Assert.True(first.IsTrue(reachableFact));
        Assert.Equal(registeredDerivedCount, problem.Context.Derived.Count);
    }

    [Fact]
    public void RepeatedTrueAndFalseDerivedQueriesAreStable()
    {
        Problem problem = CreateStateDependentReadyProblem();
        DerivedPredicate ready = problem.Domain.Derived
            .Single(predicate => predicate.Name == "ready");
        DerivedPredicate missing = problem.Domain.Derived
            .Single(predicate => predicate.Name == "missing");
        Fact<Derived> readyFact = problem.Context.RegisterFact(
            ready,
            Array.Empty<Constant>());
        Fact<Derived> missingFact = problem.Context.RegisterFact(
            missing,
            Array.Empty<Constant>());
        ExtendedState state = problem.InitialState.Expand();

        for (int iteration = 0; iteration < 4; iteration++)
        {
            Assert.True(state.IsTrue(readyFact));
            Assert.False(state.IsTrue(missingFact));
        }
    }

    [Fact]
    public void AlternatingTrueAndFalseUnaryDerivedQueriesAreStable()
    {
        Problem problem = CreateProblemForAlternatingTrueAndFalseUnaryDerivedQueriesAreStable();
        DerivedPredicate ready = problem.Domain.Derived.Single();
        (string ObjectName, bool Expected)[] expectations =
        [
            ("true-a", true),
            ("false-a", false),
            ("true-b", true),
            ("false-b", false),
            ("true-c", true),
            ("false-c", false),
        ];
        Fact<Derived>[] facts = expectations
            .Select(expectation => problem.Context.RegisterFact(
                ready,
                [problem.ObjectLookup[expectation.ObjectName]]))
            .ToArray();
        ExtendedState state = problem.InitialState.Expand();

        for (int index = 0; index < facts.Length; index++)
            Assert.Equal(expectations[index].Expected, state.IsTrue(facts[index]));
    }

    [Fact]
    public void AcyclicChainsAroundRecursiveComponentWorkInBothQueryOrders()
    {
        Problem problem = CreateProblemForAcyclicChainsAroundRecursiveComponentWorkInBothQueryOrders();
        DerivedPredicate lower = problem.Domain.Derived
            .Single(predicate => predicate.Name == "lower");
        DerivedPredicate upper = problem.Domain.Derived
            .Single(predicate => predicate.Name == "upper");
        Fact<Derived> lowerFact = problem.Context.RegisterFact(
            lower,
            Array.Empty<Constant>());
        Fact<Derived> upperFact = problem.Context.RegisterFact(
            upper,
            Array.Empty<Constant>());

        ExtendedState upperFirst = problem.InitialState.Expand();
        Assert.True(upperFirst.IsTrue(upperFact));
        Assert.True(upperFirst.IsTrue(lowerFact));

        ExtendedState lowerFirst = problem.InitialState.Expand();
        Assert.True(lowerFirst.IsTrue(lowerFact));
        Assert.True(lowerFirst.IsTrue(upperFact));
    }

    [Fact]
    public void DerivedFactRegisteredAfterExpansionCanBeQueried()
    {
        Problem problem = CreateStateDependentReadyProblem();
        ExtendedState state = problem.InitialState.Expand();
        DerivedPredicate ready = problem.Domain.Derived
            .Single(predicate => predicate.Name == "ready");
        Fact<Derived> readyFact = problem.Context.RegisterFact(
            ready,
            Array.Empty<Constant>());

        Assert.True(state.IsTrue(readyFact));
    }

    [Fact]
    public void OldInterleavedStatesGrowForLateAcyclicSlotsWithoutLosingTruth()
    {
        Problem problem = CreateProblemForOldInterleavedStatesGrowForLateAcyclicSlotsWithoutLosingTruth();
        DerivedPredicate ready = problem.Domain.Derived.Single();
        ExtendedState first = problem.InitialState.Expand();
        ExtendedState second = problem.InitialState.Expand();
        (string Name, bool Expected)[] expectations =
        [
            ("false-a", false),
            ("true-a", true),
            ("false-b", false),
            ("true-b", true),
            ("true-c", true),
        ];
        var facts = new List<Fact<Derived>>();

        foreach ((string name, bool expected) in expectations)
        {
            Fact<Derived> fact = problem.Context.RegisterFact(
                ready,
                [problem.ObjectLookup[name]]);
            facts.Add(fact);

            Assert.Equal(expected, first.IsTrue(fact));
            Assert.Equal(expected, second.IsTrue(fact));

            for (int index = 0; index < facts.Count; index++)
            {
                Assert.Equal(expectations[index].Expected, first.IsTrue(facts[index]));
                Assert.Equal(expectations[index].Expected, second.IsTrue(facts[index]));
            }
        }
    }

    [Fact]
    public void RecursiveTruthSupportsFactsRegisteredAfterExpansion()
    {
        Problem problem = CreateProblemForRecursiveTruthSupportsFactsRegisteredAfterExpansion();
        DerivedPredicate reachable = problem.Domain.Derived
            .Single(predicate => predicate.Name == "reachable");
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Constant c = problem.ObjectLookup["c"];
        GroundAction clearA = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Single(action => action.Schema.Name == "clear"
                && ReferenceEquals(action.Arguments[0], a));
        Fact[] publicFacts = problem.Context.AllFacts.ToArray();
        ExtendedState initial = problem.InitialState.Expand();
        ExtendedState successor = initial.Apply(clearA).Expand();

        Assert.Empty(problem.Context.Derived);
        Assert.Equal(publicFacts, problem.Context.AllFacts);
        Assert.True(problem.Context.TryGetDerivedEvaluationSlot(
            new DerivedFactKey(reachable, [a]),
            out _));
        Assert.True(problem.Context.TryGetDerivedEvaluationSlot(
            new DerivedFactKey(reachable, [b]),
            out _));
        Assert.False(problem.Context.TryGetDerivedEvaluationSlot(
            new DerivedFactKey(reachable, [c]),
            out _));
        int recursiveTrueSlotCount = problem.Context.DerivedEvaluationSlotCount;

        Fact<Derived> unreachable = problem.Context.RegisterFact(
            reachable,
            [c]);
        Assert.Equal(
            recursiveTrueSlotCount + 1,
            problem.Context.DerivedEvaluationSlotCount);

        Fact<Derived> firstReachable = problem.Context.RegisterFact(
            reachable,
            [a]);
        Fact<Derived> secondReachable = problem.Context.RegisterFact(
            reachable,
            [b]);
        Assert.Equal(
            recursiveTrueSlotCount + 1,
            problem.Context.DerivedEvaluationSlotCount);

        for (int iteration = 0; iteration < 4; iteration++)
        {
            Assert.False(successor.IsTrue(secondReachable));
            Assert.True(initial.IsTrue(firstReachable));
            Assert.False(successor.IsTrue(unreachable));
            Assert.True(initial.IsTrue(secondReachable));
            Assert.False(successor.IsTrue(firstReachable));
            Assert.False(initial.IsTrue(unreachable));
        }
    }

    [Fact]
    public void DerivedQueryCachesAreIsolatedAcrossStates()
    {
        Problem problem = CreateStateDependentReadyProblem();
        DerivedPredicate ready = problem.Domain.Derived
            .Single(predicate => predicate.Name == "ready");
        Fact<Derived> readyFact = problem.Context.RegisterFact(
            ready,
            Array.Empty<Constant>());
        GroundAction clear = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Single(action => action.Schema.Name == "clear");
        ExtendedState initial = problem.InitialState.Expand();

        Assert.True(initial.IsTrue(readyFact));

        ExtendedState successor = initial.Apply(clear).Expand();

        Assert.False(successor.IsTrue(readyFact));
        Assert.True(initial.IsTrue(readyFact));
        Assert.False(successor.IsTrue(readyFact));
    }

    [Fact]
    public void DerivedMemoizationIsIndependentAcrossBindingsAndStates()
    {
        Problem problem = CreateProblemForDerivedMemoizationIsIndependentAcrossBindingsAndStates();
        DerivedPredicate ready = problem.Domain.Derived.Single();
        (string ObjectName, bool Initial, bool Successor)[] expectations =
        [
            ("true-a", true, false),
            ("false-a", false, false),
            ("true-b", true, true),
            ("false-b", false, false),
            ("true-c", true, true),
            ("false-c", false, false),
            ("true-d", true, true),
            ("false-d", false, false),
        ];
        Fact<Derived>[] facts = expectations
            .Select(expectation => problem.Context.RegisterFact(
                ready,
                [problem.ObjectLookup[expectation.ObjectName]]))
            .ToArray();
        GroundAction clearTrueA = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Single(action => action.Schema.Name == "clear" && action.Arguments[0].Name == "true-a");
        ExtendedState initial = problem.InitialState.Expand();
        ExtendedState successor = initial.Apply(clearTrueA).Expand();

        for (int iteration = 0; iteration < 4; iteration++)
        {
            for (int index = 0; index < facts.Length; index++)
            {
                Assert.Equal(expectations[index].Initial, initial.IsTrue(facts[index]));
                Assert.Equal(expectations[index].Successor, successor.IsTrue(facts[index]));
            }
        }
    }

    [Fact]
    public void IsTrueRejectsDerivedFactFromAnotherProblem()
    {
        Problem localProblem = CreateStateDependentReadyProblem();
        Problem foreignProblem = CreateStateDependentReadyProblem();
        DerivedPredicate foreignReady = foreignProblem.Domain.Derived
            .Single(predicate => predicate.Name == "ready");
        Fact<Derived> foreignFact = foreignProblem.Context.RegisterFact(
            foreignReady,
            Array.Empty<Constant>());
        ExtendedState localState = localProblem.InitialState.Expand();

        Assert.Throws<ArgumentException>(() => localState.IsTrue(foreignFact));
    }

    [Fact]
    public void TrueAcyclicFactsAreEnumeratedWithoutPriorRegistration()
    {
        Problem problem =
            CreateProblemForOldInterleavedStatesGrowForLateAcyclicSlotsWithoutLosingTruth();
        ExtendedState state = problem.InitialState.Expand();

        Assert.Empty(problem.Context.Derived);

        IReadOnlyList<Fact<Derived>> facts = state.GetTrueDerivedFacts();

        Assert.Equal(
            ["true-a", "true-b", "true-c"],
            facts
                .Select(fact => Assert.Single(fact.Arguments).Name)
                .Order(StringComparer.Ordinal)
                .ToArray());
        Assert.Equal(3, problem.Context.Derived.Count);
        DerivedPredicate ready = problem.Domain.Derived.Single();
        Fact<Derived> registered = problem.Context.RegisterFact(
            ready,
            [problem.ObjectLookup["true-a"]]);
        Assert.Contains(facts, fact => ReferenceEquals(fact, registered));
    }

    [Fact]
    public void RecursiveClosureFactsAreEnumeratedWithoutReevaluationQueries()
    {
        Problem problem =
            CreateProblemForRecursiveTruthSupportsFactsRegisteredAfterExpansion();
        ExtendedState state = problem.InitialState.Expand();

        Assert.Empty(problem.Context.Derived);

        IReadOnlyList<Fact<Derived>> facts = state.GetTrueDerivedFacts();
        DerivedPredicate reachable = problem.Domain.Derived
            .Single(predicate => predicate.Name == "reachable");

        Assert.Equal(
            ["a", "b"],
            facts
                .Where(fact => ReferenceEquals(fact.Predicate, reachable))
                .Select(fact => Assert.Single(fact.Arguments).Name)
                .Order(StringComparer.Ordinal)
                .ToArray());
        Assert.Equal(3, facts.Count);
        Assert.Equal(3, problem.Context.Derived.Count);
        Assert.All(facts, fact => Assert.True(state.IsTrue(fact)));
    }

    [Fact]
    public void StaticDerivedFactsAreEnumeratedWithoutStateEvaluation()
    {
        Domain domain = new DomainBuilder("static-enumeration")
            .Requirements().Add(":adl").Close()
            .Predicates().Add("seed").Add("done").Add("ready").Close()
            .Actions().Add("finish").AddEffect("done").Close().Close()
            .DerivedPredicates().Define("ready", Logic.Atom("seed")).Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "static-enumeration-problem")
            .InitialState().AddFact("seed").Close()
            .Goal().Add("done").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();

        Assert.Empty(problem.Context.Derived);

        Fact<Derived> fact = Assert.Single(state.GetTrueDerivedFacts());

        Assert.Equal("ready", fact.Predicate.Name);
        Assert.Empty(fact.Arguments);
        Assert.Same(fact, Assert.Single(problem.Context.Derived));
    }

    [Fact]
    public void DerivedEnumerationReflectsSuccessorStateChanges()
    {
        Problem problem =
            CreateProblemForOldInterleavedStatesGrowForLateAcyclicSlotsWithoutLosingTruth();
        Constant trueA = problem.ObjectLookup["true-a"];
        GroundAction clearTrueA = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Single(action => action.Schema.Name == "clear"
                && ReferenceEquals(action.Arguments[0], trueA));
        ExtendedState initial = problem.InitialState.Expand();
        ExtendedState successor = initial.Apply(clearTrueA).Expand();

        Assert.Equal(
            ["true-a", "true-b", "true-c"],
            initial.GetTrueDerivedFacts()
                .Select(fact => Assert.Single(fact.Arguments).Name)
                .Order(StringComparer.Ordinal)
                .ToArray());
        Assert.Equal(
            ["true-b", "true-c"],
            successor.GetTrueDerivedFacts()
                .Select(fact => Assert.Single(fact.Arguments).Name)
                .Order(StringComparer.Ordinal)
                .ToArray());
    }

    [Fact]
    public void RepeatedDerivedEnumerationReturnsTheSameSnapshot()
    {
        Problem problem =
            CreateProblemForRecursiveTruthSupportsFactsRegisteredAfterExpansion();
        ExtendedState state = problem.InitialState.Expand();

        IReadOnlyList<Fact<Derived>> first = state.GetTrueDerivedFacts();
        int registeredFactCount = problem.Context.Derived.Count;
        DerivedPredicate reachable = problem.Domain.Derived
            .Single(predicate => predicate.Name == "reachable");
        Fact<Derived> falseFact = problem.Context.RegisterFact(
            reachable,
            [problem.ObjectLookup["c"]]);

        Assert.False(state.IsTrue(falseFact));
        Assert.Same(first, state.GetTrueDerivedFacts());
        Assert.Equal(registeredFactCount + 1, problem.Context.Derived.Count);
        Assert.DoesNotContain(first, fact => ReferenceEquals(fact, falseFact));
    }

    private static Problem CreateStateDependentReadyProblem()
    {
        Domain domain = new DomainBuilder("state-dependent-ready")
            .Requirements().Add(":adl").Close()
            .Predicates().Add("seed").Add("done").Add("ready").Add("missing").Close()
            .Actions().Add("clear").AddPrecondition("seed")
                .AddEffect("seed", Polarity.Negative).AddEffect("done").Close().Close()
            .DerivedPredicates().Define("ready", Logic.Atom("seed"))
                .Define("missing", Logic.Not(Logic.Atom("seed"))).Close().Build();
        return new ProblemBuilder(domain, "state-dependent-ready-problem")
            .InitialState().AddFact("seed").Close().Goal().Add("done").Close().Build();
    }


    private static Problem CreateProblemForStaticClosurePersistsWhileStateDependentClosureChanges()
    {
        DomainBuilder domainBuilder = new DomainBuilder("closure-types");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("static-seed");
        predicates.Add("fluent-seed");
        predicates.Add("done");
        predicates.Add("static-ready");
        predicates.Add("dynamic-ready");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddPrecondition("dynamic-ready");
        action.AddEffect("fluent-seed", Polarity.Negative);
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("static-ready", Logic.Atom("static-seed"));
        derivedPredicates.Define("dynamic-ready", Logic.Atom("fluent-seed"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "closure-types-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("static-seed");
        initialState.AddFact("fluent-seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStratifiedNegationReadsTheCompletedLowerComponent()
    {
        DomainBuilder domainBuilder = new DomainBuilder("stratified-negation");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("lower");
        predicates.Add("upper");
        predicates.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("lower", Logic.Atom("seed"));
        derivedPredicates.Define("upper", Logic.Not(Logic.Atom("lower")));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "stratified-negation-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("upper", Polarity.Negative);
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStateDependenceIsClassifiedTransitively()
    {
        DomainBuilder domainBuilder = new DomainBuilder("transitive-classification");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("static-seed");
        predicates.Add("fluent-seed");
        predicates.Add("static-base");
        predicates.Add("static-transitive");
        predicates.Add("dynamic-base");
        predicates.Add("dynamic-transitive");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddPrecondition("fluent-seed");
        action.AddEffect("fluent-seed", Polarity.Negative);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("static-base", Logic.Atom("static-seed"));
        derivedPredicates.Define("static-transitive", Logic.Atom("static-base"));
        derivedPredicates.Define("dynamic-base", Logic.Atom("fluent-seed"));
        derivedPredicates.Define("dynamic-transitive", Logic.Atom("dynamic-base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "transitive-classification-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("static-seed");
        initialState.AddFact("fluent-seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("dynamic-transitive");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForConditionalDerivedGuardUsesTheSourceClosure()
    {
        DomainBuilder domainBuilder = new DomainBuilder("source-closure");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("trigger");
        predicates.Add("result");
        predicates.Add("ready");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddEffect("trigger");
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("ready");
        conditional0.AddEffect("result");
        conditional0.Close();
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("trigger"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "source-closure-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("result");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForRepeatedExpansionsHaveIndependentDerivedEvaluations()
    {
        DomainBuilder domainBuilder = new DomainBuilder("concurrent-closure");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed", ("?value", "object"));
        predicates.Add("reachable", ("?value", "object"));
        predicates.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("seed", "?value"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "concurrent-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("value", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed", "value");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("reachable", "value");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForAlternatingTrueAndFalseUnaryDerivedQueriesAreStable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unary-derived");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed", ("?value", "object"));
        predicates.Add("done");
        predicates.Add("ready", ("?value", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddParameter("?value", "object");
        action.AddPrecondition("seed", "?value");
        action.AddEffect("seed", Polarity.Negative, "?value");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("seed", "?value"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "unary-derived-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("true-a", "object");
        objects.Add("false-a", "object");
        objects.Add("true-b", "object");
        objects.Add("false-b", "object");
        objects.Add("true-c", "object");
        objects.Add("false-c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed", "true-a");
        initialState.AddFact("seed", "true-b");
        initialState.AddFact("seed", "true-c");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("ready", "true-a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForAcyclicChainsAroundRecursiveComponentWorkInBothQueryOrders()
    {
        DomainBuilder domainBuilder = new DomainBuilder("mixed-components");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("done");
        predicates.Add("lower");
        predicates.Add("cycle-left");
        predicates.Add("cycle-right");
        predicates.Add("upper");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("lower", Logic.Atom("seed"));
        derivedPredicates.Define("cycle-left", Logic.Or(Logic.Atom("lower"), Logic.Atom("cycle-right")));
        derivedPredicates.Define("cycle-right", Logic.Atom("cycle-left"));
        derivedPredicates.Define("upper", Logic.Atom("cycle-right"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "mixed-components-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("upper");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForOldInterleavedStatesGrowForLateAcyclicSlotsWithoutLosingTruth()
    {
        DomainBuilder domainBuilder = new DomainBuilder("late-acyclic-slots");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed", ("?value", "object"));
        predicates.Add("done");
        predicates.Add("ready", ("?value", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddParameter("?value", "object");
        action.AddPrecondition("seed", "?value");
        action.AddEffect("seed", Polarity.Negative, "?value");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("seed", "?value"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "late-acyclic-slots-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("true-a", "object");
        objects.Add("false-a", "object");
        objects.Add("true-b", "object");
        objects.Add("false-b", "object");
        objects.Add("true-c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed", "true-a");
        initialState.AddFact("seed", "true-b");
        initialState.AddFact("seed", "true-c");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForRecursiveTruthSupportsFactsRegisteredAfterExpansion()
    {
        DomainBuilder domainBuilder = new DomainBuilder("late-recursive-slots");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed", ("?value", "object"));
        predicates.Add("edge", ("?from", "object"), ("?to", "object"));
        predicates.Add("done");
        predicates.Add("reachable", ("?to", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddParameter("?value", "object");
        action.AddPrecondition("seed", "?value");
        action.AddEffect("seed", Polarity.Negative, "?value");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define(
            "reachable",
            Logic.Or(
                Logic.Atom("seed", "?to"),
                Logic.Exists(
                    new[] { ("?from", "object") },
                    Logic.And(
                        Logic.Atom("reachable", "?from"),
                        Logic.Atom("edge", "?from", "?to")))));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "late-recursive-slots-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed", "a");
        initialState.AddFact("edge", "a", "b");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedMemoizationIsIndependentAcrossBindingsAndStates()
    {
        DomainBuilder domainBuilder = new DomainBuilder("concurrent-unary-derived");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed", ("?value", "object"));
        predicates.Add("done");
        predicates.Add("ready", ("?value", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear");
        action.AddParameter("?value", "object");
        action.AddPrecondition("seed", "?value");
        action.AddEffect("seed", Polarity.Negative, "?value");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("seed", "?value"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "concurrent-unary-derived-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("true-a", "object");
        objects.Add("false-a", "object");
        objects.Add("true-b", "object");
        objects.Add("false-b", "object");
        objects.Add("true-c", "object");
        objects.Add("false-c", "object");
        objects.Add("true-d", "object");
        objects.Add("false-d", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed", "true-a");
        initialState.AddFact("seed", "true-b");
        initialState.AddFact("seed", "true-c");
        initialState.AddFact("seed", "true-d");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("ready", "true-a");
        goals.Close();
        return problemBuilder.Build();
    }

}
