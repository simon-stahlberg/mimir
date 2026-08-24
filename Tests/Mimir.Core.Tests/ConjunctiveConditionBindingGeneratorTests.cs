using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

public class ConjunctiveConditionBindingGeneratorTests
{
    private struct BindingCollector
    {
        public List<string> Names { get; }
        public int Limit { get; }
        public int Visits;

        public BindingCollector(List<string> names, int limit)
        {
            Names = names;
            Limit = limit;
            Visits = 0;
        }
    }

    private struct ReentrantBindingCollector
    {
        public required ConjunctiveConditionBindingGenerator Generator { get; init; }
        public required CompiledConjunctiveCondition Condition { get; init; }
        public required ExtendedState NestedState { get; init; }
        public int Visits;
        public int NestedCount;
    }

    private static bool CollectBinding(
        ReadOnlySpan<Constant> binding,
        ref BindingCollector collector)
    {
        collector.Names.Add(binding[0].Name);
        collector.Visits++;
        return collector.Visits < collector.Limit;
    }

    private static bool VisitReentrantly(
        ReadOnlySpan<Constant> binding,
        ref ReentrantBindingCollector collector)
    {
        collector.Visits++;
        collector.NestedCount = collector.Generator.CountBindings(
            collector.Condition,
            collector.NestedState);
        return true;
    }

    private static void AssertStatefulCallbacks(
        Problem problem,
        ConjunctiveConditionBindingGenerator generator,
        CompiledConjunctiveCondition condition)
    {
        var limited = new BindingCollector(new List<string>(), limit: 10);
        int limitedCount = generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            maxBindings: 2,
            ref limited,
            CollectBinding);
        var cancelled = new BindingCollector(new List<string>(), limit: 1);
        int cancelledCount = generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            ref cancelled,
            CollectBinding);
        var convenienceNames = new List<string>();
        generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            maxBindings: 2,
            binding =>
            {
                convenienceNames.Add(binding[0].Name);
                return true;
            });

        Assert.Equal(2, limitedCount);
        Assert.Equal(2, limited.Visits);
        Assert.Equal(2, limited.Names.Count);
        Assert.Equal(convenienceNames, limited.Names);
        Assert.Equal(1, cancelledCount);
        Assert.Equal(1, cancelled.Visits);
        Assert.Single(cancelled.Names);
    }

    private Problem CreateStatefulCallbackProblem()
    {
        Domain domain = new DomainBuilder("d").Requirements().Add(":strips").Close()
            .Actions().Add("act").AddParameter("?x").Close().Close().Build();
        return new ProblemBuilder(domain, "p").Objects()
            .Add("o1").Add("o2").Add("o3").Close().Build();
    }

    private static Problem CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("active", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("o3", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("active", "o1");
        initialState.AddFact("active", "o3");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_RespectsMaxBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("o3", "object");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_StateDependentDerivedBinaryIsReentrant()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":derived-predicates");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("arc", ("?from", "item"), ("?to", "item"));
        predicates.Add("active", ("?x", "item"));
        predicates.Add("done");
        predicates.Add("reachable", ("?from", "item"), ("?to", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddParameter("?x", "item");
        action.AddEffect("active", "?x");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?from", "item");
        action.AddParameter("?to", "item");
        action.AddPrecondition("arc", "?from", "?to");
        action.AddPrecondition("reachable", "?from", "?to");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.And(Logic.Atom("arc", "?from", "?to"), Logic.Atom("active", "?from")));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Add("c", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("arc", "a", "b");
        initialState.AddFact("arc", "b", "c");
        initialState.AddFact("active", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCompile_BinaryDerivedConditionIsReevaluatedForEachState()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":derived-predicates");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("arc", ("?from", "item"), ("?to", "item"));
        predicates.Add("active", ("?x", "item"));
        predicates.Add("done");
        predicates.Add("reachable", ("?from", "item"), ("?to", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddParameter("?x", "item");
        action.AddEffect("active", "?x");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?from", "item");
        action.AddParameter("?to", "item");
        action.AddPrecondition("arc", "?from", "?to");
        action.AddPrecondition("reachable", "?from", "?to");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.And(Logic.Atom("arc", "?from", "?to"), Logic.Atom("active", "?from")));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Add("c", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("arc", "a", "b");
        initialState.AddFact("arc", "b", "c");
        initialState.AddFact("active", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_ZeroArityCondition_YieldsSingleEmptyBinding()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_ZeroArityDerivedCondition_UsesSparseClosure()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":derived-predicates");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("done");
        predicates.Add("enabled");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddPrecondition("enabled");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("enabled", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("base");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindingsUsingFluentState_RejectsDerivedConditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":derived-predicates");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base", ("?x", "object"));
        predicates.Add("gate", ("?x", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("enable");
        action.AddParameter("?x", "object");
        action.AddEffect("base", "?x");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddPrecondition("gate", "?x");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("gate", Logic.Atom("base", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("base", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_BinaryFluentCondition_ReturnsOnlyMatchingOrderedPairs()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("link", ("?x", "object"), ("?y", "object"));
        predicates.Add("visited", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddEffect("link", "?x", "?y");
        action.Close();
        action = actions.Add("traverse");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("link", "?x", "?y");
        action.AddEffect("visited", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("link", "a", "b");
        initialState.AddFact("link", "b", "c");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("link", ("?x", "object"), ("?y", "object"));
        predicates.Add("visited", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddEffect("link", "?x", "?y");
        action.Close();
        action = actions.Add("traverse");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("link", "?x", "?y");
        action.AddEffect("visited", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("link", "a", "b");
        initialState.AddFact("link", "b", "c");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("binary-groups");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("a", "item");
        constants.Add("b", "item");
        constants.Add("c", "item");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("left", ("?x", "item"), ("?z", "item"));
        predicates.Add("right", ("?y", "item"), ("?z", "item"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("inspect");
        action.AddParameter("?x", "item");
        action.AddParameter("?y", "item");
        action.AddParameter("?z", "item");
        action.AddPrecondition("left", "?x", "?z");
        action.AddPrecondition("right", "?y", "?z");
        action.AddEffect("left", "?x", "?z");
        action.AddEffect("right", "?y", "?z");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("left", "a", "c");
        initialState.AddFact("right", "b", "c");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_SparsePositiveStaticLiteralReturnsMatchingBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-static");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("triple", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("triple", "?x", "?y", "?z");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("triple", "c", "a", "b");
        initialState.AddFact("triple", "a", "c", "b");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-static-reentrant");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?value", "object"));
        predicates.Add("triple", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("select");
        action.AddParameter("?from", "object");
        action.AddParameter("?to", "object");
        action.AddPrecondition("active", "?from");
        action.AddEffect("active", Polarity.Negative, "?from");
        action.AddEffect("active", "?to");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("active", "?y");
        action.AddPrecondition("triple", "?x", "?y", "?z");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("active", "a");
        initialState.AddFact("triple", "a", "a", "c");
        initialState.AddFact("triple", "b", "a", "b");
        initialState.AddFact("triple", "c", "b", "a");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_SparseRelationClearsCandidateMembershipAfterCallbackThrows()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-cleanup");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?value", "object"));
        predicates.Add("triple", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("select");
        action.AddParameter("?from", "object");
        action.AddParameter("?to", "object");
        action.AddPrecondition("active", "?from");
        action.AddEffect("active", Polarity.Negative, "?from");
        action.AddEffect("active", "?to");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("active", "?y");
        action.AddPrecondition("triple", "?x", "?y", "?z");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("active", "a");
        initialState.AddFact("triple", "a", "a", "c");
        initialState.AddFact("triple", "b", "a", "b");
        initialState.AddFact("triple", "c", "b", "a");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_SparseRelationUsesMultipleSingletonCandidatesAndEmptySupport()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-singletons");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("left-ok", ("?value", "object"));
        predicates.Add("middle-ok", ("?value", "object"));
        predicates.Add("triple", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("set-left");
        action.AddParameter("?value", "object");
        action.AddEffect("left-ok", "?value");
        action.Close();
        action = actions.Add("set-middle");
        action.AddParameter("?value", "object");
        action.AddEffect("middle-ok", "?value");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("left-ok", "?x");
        action.AddPrecondition("middle-ok", "?y");
        action.AddPrecondition("triple", "?x", "?y", "?z");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Add("d", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("left-ok", "a");
        initialState.AddFact("middle-ok", "b");
        initialState.AddFact("triple", "a", "b", "a");
        initialState.AddFact("triple", "a", "b", "c");
        initialState.AddFact("triple", "a", "c", "b");
        initialState.AddFact("triple", "a", "d", "d");
        initialState.AddFact("triple", "c", "b", "d");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_SparseRelationMatchesRepeatedVariablesConstantsAndPermutedSlots()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-shape");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("marker", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("pattern", ("?first", "object"), ("?fixed", "object"), ("?second", "object"), ("?first-copy", "object"), ("?third", "object"), ("?second-copy", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("pattern", "?z", "marker", "?x", "?z", "?y", "?x");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("pattern", "c", "marker", "a", "c", "b", "a");
        initialState.AddFact("pattern", "a", "marker", "c", "a", "b", "c");
        initialState.AddFact("pattern", "c", "marker", "a", "a", "b", "a");
        initialState.AddFact("pattern", "c", "marker", "a", "c", "b", "b");
        initialState.AddFact("pattern", "c", "a", "a", "c", "b", "a");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_MultipleSparseRelationsIntersect()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-intersection");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("q", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("p", "?x", "?y", "?z");
        action.AddPrecondition("q", "?z", "?x", "?y");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p", "a", "b", "c");
        initialState.AddFact("p", "b", "c", "a");
        initialState.AddFact("p", "c", "a", "b");
        initialState.AddFact("q", "c", "a", "b");
        initialState.AddFact("q", "b", "c", "a");
        initialState.AddFact("q", "a", "a", "a");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_RespectsMaxBindings0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-order");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?value", "object"));
        predicates.Add("triple", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddParameter("?value", "object");
        action.AddEffect("active", "?value");
        action.Close();
        action = actions.Add("produce");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddEffect("triple", "?x", "?y", "?z");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("active", "?y");
        action.AddPrecondition("triple", "?x", "?y", "?z");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("active", "a");
        initialState.AddFact("triple", "c", "a", "c");
        initialState.AddFact("triple", "b", "a", "b");
        initialState.AddFact("triple", "a", "a", "c");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEnumerateBindings_RespectsMaxBindings1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?x", "object"));
        predicates.Add("ready", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("activate");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("active", "?x");
        action.Close();
        action = actions.Add("inspect");
        action.AddParameter("?x", "object");
        action.AddPrecondition("active", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.AddFact("ready", "o2");
        initialState.AddFact("active", "o1");
        initialState.Close();
        return problemBuilder.Build();
    }

    [Fact]
    public void EnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects()
    {
        var problem = CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects();

        var generator = new ConjunctiveConditionBindingGenerator();
        var schema = Assert.Single(problem.Domain.Actions);
        var condition = generator.Compile(problem, schema.Parameters, schema.FluentPreconditions, schema.StaticPreconditions, schema.DerivedPreconditions);

        var bindings = new List<string>();
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            bindings.Add(binding[0].Name);
            return true;
        });

        Assert.Equal(2, count);
        Assert.Equal(new[] { "o1", "o3" }, bindings.OrderBy(name => name).ToArray());
        Assert.Equal(2, generator.CountBindings(condition, problem.InitialState.Expand()));
        Assert.True(generator.HasAnyBinding(condition, problem.InitialState.Expand()));
    }

    [Fact]
    public void EnumerateBindings_RespectsMaxBindings()
    {
        var problem = CreateProblemForEnumerateBindings_RespectsMaxBindings();

        var generator = new ConjunctiveConditionBindingGenerator();
        var schema = Assert.Single(problem.Domain.Actions);
        var condition = generator.Compile(problem, schema.Parameters, schema.FluentPreconditions, schema.StaticPreconditions, schema.DerivedPreconditions);

        var bindings = new List<string>();
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), 2, binding =>
        {
            bindings.Add(binding[0].Name);
            return true;
        });

        Assert.Equal(2, count);
        Assert.Equal(2, bindings.Count);
        Assert.Equal(2, generator.CountBindings(condition, problem.InitialState.Expand(), 2));
    }

    [Fact]
    public void EnumerateBindings_StatefulCallbackPreservesStateLimitsAndEarlyTermination()
    {
        Problem problem = CreateStatefulCallbackProblem();
        var generator = new ConjunctiveConditionBindingGenerator();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);
        AssertStatefulCallbacks(problem, generator, condition);
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void EnumerateBindings_BinaryDerivedConditionUsesSparseClosure(bool isPositive)
    {
        DomainBuilder builder = new DomainBuilder("d")
            .Requirements().Add(":strips").Add(":typing").Add(":negative-preconditions")
                .Add(":derived-predicates").Close()
            .Types().Add("item").Close()
            .Predicates().Add("arc", ("?from", "item"), ("?to", "item"))
                .Add("done").Add("reachable", ("?from", "item"), ("?to", "item")).Close();
        ActionSchemaBuilder inspect = builder.Actions().Add("inspect")
            .AddParameter("?x", "item").AddParameter("?y", "item")
            .AddPrecondition("arc", "?y", "?x");
        inspect.AddPrecondition("reachable", isPositive ? Polarity.Positive : Polarity.Negative,
            isPositive ? ["?y", "?x"] : ["?x", "?y"]);
        Domain domain = inspect.AddEffect("done").Close().Close()
            .DerivedPredicates().Define("reachable", Logic.Atom("arc", "?from", "?to"))
                .Close().Build();
        Problem problem = new ProblemBuilder(domain, "p")
            .Objects().Add("a", "item").Add("b", "item").Add("c", "item").Close()
            .InitialState().AddFact("arc", "b", "a").AddFact("arc", "c", "b").Close()
            .Goal().Add("done").Close().Build();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        var generator = new ConjunctiveConditionBindingGenerator();

        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        Assert.Equal(1, generator.CountBindings(condition, problem.InitialState.Expand(), 1));

        var bindings = new List<string>();
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            bindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        Assert.Equal(2, count);
        Assert.Equal(new[] { "a b", "b c" }, bindings.Order().ToArray());
    }

    [Fact]
    public void Compile_BinaryDerivedConditionIsReevaluatedForEachState()
    {
        Problem problem = CreateProblemForCompile_BinaryDerivedConditionIsReevaluatedForEachState();
        ActionSchema inspect = problem.Domain.Actions.Single(action => action.Name == "inspect");
        var bindingGenerator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = bindingGenerator.Compile(
            problem,
            inspect.Parameters,
            inspect.FluentPreconditions,
            inspect.StaticPreconditions,
            inspect.DerivedPreconditions);
        var groundedGenerator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());
        Mimir.Core.Grounding.Action activateB = groundedGenerator
            .GetApplicableActions(problem.InitialState.Expand())
            .Single(action => action.Schema.Name == "activate" && action.Arguments[0].Name == "b");
        State nextState = problem.InitialState.Expand().Apply(activateB);

        Assert.Equal(1, bindingGenerator.CountBindings(condition, problem.InitialState.Expand()));
        Assert.Equal(2, bindingGenerator.CountBindings(condition, nextState.Expand()));
    }

    [Fact]
    public void EnumerateBindings_StateDependentDerivedBinaryIsReentrant()
    {
        Problem problem = CreateProblemForEnumerateBindings_StateDependentDerivedBinaryIsReentrant();
        ActionSchema inspect = problem.Domain.Actions.Single(action => action.Name == "inspect");
        var bindingGenerator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = bindingGenerator.Compile(
            problem,
            inspect.Parameters,
            inspect.FluentPreconditions,
            inspect.StaticPreconditions,
            inspect.DerivedPreconditions);
        var groundedGenerator = new GroundedApplicableActionGenerator(
            problem,
            problem.InitialState,
            new RpgGrounder());
        Mimir.Core.Grounding.Action activateB = groundedGenerator
            .GetApplicableActions(problem.InitialState.Expand())
            .Single(action => action.Schema.Name == "activate" && action.Arguments[0].Name == "b");
        ExtendedState initialState = problem.InitialState.Expand();
        ExtendedState nextState = initialState.Apply(activateB).Expand();

        var collector = new ReentrantBindingCollector
        {
            Generator = bindingGenerator,
            Condition = condition,
            NestedState = nextState,
            NestedCount = -1,
        };
        int outerCount = bindingGenerator.EnumerateBindings(
            condition,
            initialState,
            ref collector,
            VisitReentrantly);

        Assert.Equal(1, outerCount);
        Assert.Equal(1, collector.Visits);
        Assert.Equal(2, collector.NestedCount);
    }

    [Fact]
    public void EnumerateBindings_ZeroArityCondition_YieldsSingleEmptyBinding()
    {
        var problem = CreateProblemForEnumerateBindings_ZeroArityCondition_YieldsSingleEmptyBinding();

        var generator = new ConjunctiveConditionBindingGenerator();
        var schema = Assert.Single(problem.Domain.Actions);
        var condition = generator.Compile(problem, schema.Parameters, schema.FluentPreconditions, schema.StaticPreconditions, schema.DerivedPreconditions);

        int callbackCount = 0;
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            Assert.Empty(binding.ToArray());
            callbackCount++;
            return true;
        });

        Assert.Equal(1, count);
        Assert.Equal(1, callbackCount);
    }

    [Fact]
    public void EnumerateBindings_ZeroArityDerivedCondition_UsesSparseClosure()
    {
        Problem problem = CreateProblemForEnumerateBindings_ZeroArityDerivedCondition_UsesSparseClosure();

        var generator = new ConjunctiveConditionBindingGenerator();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        int callbackCount = 0;
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            Assert.Empty(binding.ToArray());
            callbackCount++;
            return true;
        });

        Assert.Equal(1, count);
        Assert.Equal(1, callbackCount);
        Assert.Equal(1, generator.CountBindings(condition, problem.InitialState.Expand()));
        Assert.True(generator.HasAnyBinding(condition, problem.InitialState.Expand()));
    }

    [Fact]
    public void EnumerateBindingsUsingFluentState_RejectsDerivedConditions()
    {
        Problem problem = CreateProblemForEnumerateBindingsUsingFluentState_RejectsDerivedConditions();
        ActionSchema schema = problem.Domain.Actions.Single(action => action.Name == "inspect");
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        Assert.Throws<InvalidOperationException>(
            () => generator.EnumerateBindingsUsingFluentState(
                condition,
                problem.InitialState,
                _ => true));
        Assert.Equal(1, generator.CountBindings(condition, problem.InitialState.Expand()));
    }

    [Fact]
    public void EnumerateBindings_BinaryFluentCondition_ReturnsOnlyMatchingOrderedPairs()
    {
        var problem = CreateProblemForEnumerateBindings_BinaryFluentCondition_ReturnsOnlyMatchingOrderedPairs();

        var generator = new ConjunctiveConditionBindingGenerator();
        var schema = problem.Domain.Actions.Single(a => a.Name == "traverse");
        var condition = generator.Compile(problem, schema.Parameters, schema.FluentPreconditions, schema.StaticPreconditions, schema.DerivedPreconditions);

        var bindings = new List<(string x, string y)>();
        generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            bindings.Add((binding[0].Name, binding[1].Name));
            return true;
        });

        Assert.Equal(2, bindings.Count);
        Assert.Contains(("a", "b"), bindings);
        Assert.Contains(("b", "c"), bindings);
        Assert.DoesNotContain(("a", "c"), bindings);
        Assert.DoesNotContain(("a", "a"), bindings);
    }

    [Fact]
    public void EnumerateBindings_BinaryFluentCondition_EarlyTerminationRespectsCallback()
    {
        var problem = CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects1();

        var generator = new ConjunctiveConditionBindingGenerator();
        var schema = problem.Domain.Actions.Single(a => a.Name == "traverse");
        var condition = generator.Compile(problem, schema.Parameters, schema.FluentPreconditions, schema.StaticPreconditions, schema.DerivedPreconditions);

        int callbackCount = 0;
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            callbackCount++;
            return false;
        });

        Assert.Equal(1, count);
        Assert.Equal(1, callbackCount);
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void Compile_BinaryFluentIndexesOnlyRegisteredFactsAndTracksLaterFacts(
        bool isPositive)
    {
        DomainBuilder builder = new DomainBuilder("sparse-binary")
            .Requirements().Add(":strips").Add(":typing").Add(":negative-preconditions").Close()
            .Types().Add("item").Add("tag").Close().Constants().Add("marker", "tag").Close()
            .Predicates().Add("pattern", ("?first", "item"), ("?fixed", "tag"),
                ("?second", "item"), ("?first-copy", "item")).Add("done").Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("inspect").AddParameter("?x", "item").AddParameter("?y", "item")
            .AddPrecondition("pattern", isPositive ? Polarity.Positive : Polarity.Negative,
                "?y", "marker", "?x", "?y").AddEffect("done").Close();
        actions.Add("produce").AddParameter("?x", "item").AddParameter("?y", "item")
            .AddEffect("pattern", "?y", "marker", "?x", "?y").Close();
        Domain domain = actions.Close().Build();
        Problem problem = new ProblemBuilder(domain, "p")
            .Objects().Add("a", "item").Add("b", "item").Add("c", "item").Close()
            .InitialState().AddFact("pattern", "b", "marker", "a", "b").Close().Build();
        ActionSchema schema = problem.Domain.Actions.Single(action => action.Name == "inspect");
        Mimir.Core.Schemas.Predicate<Fluent> pattern = problem.Domain.Fluents
            .Single(predicate => predicate.Name == "pattern");
        Constant a = problem.ObjectLookup["a"];
        Constant c = problem.ObjectLookup["c"];
        Constant marker = problem.ObjectLookup["marker"];
        State initialState = problem.InitialState;
        var generator = new ConjunctiveConditionBindingGenerator();

        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        Assert.Equal(isPositive ? 1 : 8, generator.CountBindings(condition, initialState.Expand()));

        Fact<Fluent> laterFact = problem.Context.RegisterFact(pattern, [a, marker, c, a]);
        State stateWithLaterFact = initialState.WithAdditionalFluentFacts([laterFact]);
        var bindings = new List<string>();
        generator.EnumerateBindings(condition, stateWithLaterFact.Expand(), binding =>
        {
            bindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        Assert.Equal(isPositive ? 2 : 7, bindings.Count);
        Assert.Equal(isPositive, bindings.Contains("a b"));
        Assert.Equal(isPositive, bindings.Contains("c a"));
        Assert.Equal(
            isPositive ? 1 : 8,
            generator.CountBindings(condition, initialState.Expand()));
    }

    [Fact]
    public void EnumerateBindings_FluentConditionsTrackNonadjacentParameterPairsAcrossStates()
    {
        Problem problem = CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects3();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        Assert.Equal(1, generator.CountBindings(condition, problem.InitialState.Expand()));

        Mimir.Core.Schemas.Predicate<Fluent> left = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "left");
        Mimir.Core.Schemas.Predicate<Fluent> right = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "right");
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Constant c = problem.ObjectLookup["c"];
        Fact<Fluent> laterLeft = problem.Context.RegisterFact(left, [b, c]);
        Fact<Fluent> laterRight = problem.Context.RegisterFact(right, [a, c]);
        State stateWithLaterFacts = problem.InitialState.WithAdditionalFluentFacts(
            [laterLeft, laterRight]);

        Assert.Equal(4, generator.CountBindings(condition, stateWithLaterFacts.Expand()));
        Assert.Equal(1, generator.CountBindings(condition, problem.InitialState.Expand()));
    }

    [Fact]
    public void EnumerateBindings_SparsePositiveStaticLiteralReturnsMatchingBindings()
    {
        Problem problem = CreateProblemForEnumerateBindings_SparsePositiveStaticLiteralReturnsMatchingBindings();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        var generator = new ConjunctiveConditionBindingGenerator();

        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        var bindings = new List<string>();
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            bindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        Assert.Equal(2, count);
        Assert.Equal(new[] { "a c b", "c a b" }.Order(), bindings.Order());
    }

    [Fact]
    public void EnumerateBindings_StaticSparseRelationIsFilteredAndReentrant()
    {
        Problem problem = CreateProblemForEnumerateBindings_UnaryFluentCondition_ReturnsMatchingObjects5();
        ActionSchema inspect = problem.Domain.Actions.Single(
            action => action.Name == "inspect");
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            inspect.Parameters,
            inspect.FluentPreconditions,
            inspect.StaticPreconditions,
            inspect.DerivedPreconditions);
        var groundedGenerator = new GroundedApplicableActionGenerator(
            problem,
            problem.InitialState,
            new RpgGrounder());
        Mimir.Core.Grounding.Action selectB = groundedGenerator
            .GetApplicableActions(problem.InitialState.Expand())
            .Single(action => action.Schema.Name == "select"
                && action.Arguments[0].Name == "a"
                && action.Arguments[1].Name == "b");
        ExtendedState initialState = problem.InitialState.Expand();
        ExtendedState selectedBState = initialState.Apply(selectB).Expand();

        var outerBindings = new List<string>();
        var nestedCounts = new List<int>();
        int outerCount = generator.EnumerateBindings(
            condition,
            initialState,
            binding =>
            {
                outerBindings.Add(string.Join(
                    " ",
                    binding.ToArray().Select(constant => constant.Name)));
                nestedCounts.Add(generator.CountBindings(condition, selectedBState));
                return true;
        });

        Assert.Equal(2, outerCount);
        Assert.Equal(new[] { "a a c", "b a b" }.Order(), outerBindings.Order());
        Assert.Equal(outerBindings.Count, outerBindings.Distinct().Count());
        Assert.Equal(new[] { 1, 1 }, nestedCounts);
        Assert.Equal(2, generator.CountBindings(condition, initialState));
    }

    [Fact]
    public void EnumerateBindings_SparseRelationClearsCandidateMembershipAfterCallbackThrows()
    {
        Problem problem = CreateProblemForEnumerateBindings_SparseRelationClearsCandidateMembershipAfterCallbackThrows();
        ActionSchema inspect = problem.Domain.Actions.Single(
            action => action.Name == "inspect");
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            inspect.Parameters,
            inspect.FluentPreconditions,
            inspect.StaticPreconditions,
            inspect.DerivedPreconditions);
        Mimir.Core.Schemas.Predicate<Fluent> active = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "active");
        Fact<Fluent> activeB = problem.Context.RegisterFact(
            active,
            [problem.ObjectLookup["b"]]);
        ExtendedState stateB = StateFactory.Default
            .Create(problem.Context, [activeB])
            .Expand();

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(
            () => generator.EnumerateBindings(
                condition,
                problem.InitialState.Expand(),
                _ => throw new InvalidOperationException("callback failure")));

        Assert.Equal("callback failure", exception.Message);

        var stateBBindings = new List<string>();
        int stateBCount = generator.EnumerateBindings(condition, stateB, binding =>
        {
            stateBBindings.Add(string.Join(
                " ",
                binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        var initialBindings = new List<string>();
        int initialCount = generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            binding =>
            {
                initialBindings.Add(string.Join(
                    " ",
                    binding.ToArray().Select(constant => constant.Name)));
                return true;
            });

        Assert.Equal(1, stateBCount);
        Assert.Equal(new[] { "c b a" }.Order(), stateBBindings.Order());
        Assert.Equal(stateBBindings.Count, stateBBindings.Distinct().Count());
        Assert.Equal(2, initialCount);
        Assert.Equal(new[] { "a a c", "b a b" }.Order(), initialBindings.Order());
        Assert.Equal(initialBindings.Count, initialBindings.Distinct().Count());
    }

    [Fact]
    public void EnumerateBindings_SparseRelationUsesMultipleSingletonCandidatesAndEmptySupport()
    {
        Problem problem = CreateProblemForEnumerateBindings_SparseRelationUsesMultipleSingletonCandidatesAndEmptySupport();
        ActionSchema inspect = problem.Domain.Actions.Single(
            action => action.Name == "inspect");
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            inspect.Parameters,
            inspect.FluentPreconditions,
            inspect.StaticPreconditions,
            inspect.DerivedPreconditions);
        Mimir.Core.Schemas.Predicate<Fluent> leftOk = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "left-ok");
        Mimir.Core.Schemas.Predicate<Fluent> middleOk = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "middle-ok");
        Constant b = problem.ObjectLookup["b"];
        Fact<Fluent> leftB = problem.Context.RegisterFact(leftOk, [b]);
        Fact<Fluent> middleB = problem.Context.RegisterFact(middleOk, [b]);
        ExtendedState emptySupportState = StateFactory.Default
            .Create(problem.Context, [leftB, middleB])
            .Expand();

        var initialBindings = new List<string>();
        int initialCount = generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            binding =>
            {
                initialBindings.Add(string.Join(
                    " ",
                    binding.ToArray().Select(constant => constant.Name)));
                return true;
            });

        int emptyCount = generator.CountBindings(condition, emptySupportState);

        var repeatedBindings = new List<string>();
        int repeatedCount = generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            binding =>
            {
                repeatedBindings.Add(string.Join(
                    " ",
                    binding.ToArray().Select(constant => constant.Name)));
                return true;
            });

        Assert.Equal(2, initialCount);
        Assert.Equal(new[] { "a b a", "a b c" }.Order(), initialBindings.Order());
        Assert.Equal(initialBindings.Count, initialBindings.Distinct().Count());
        Assert.Equal(0, emptyCount);
        Assert.Equal(initialCount, repeatedCount);
        Assert.Equal(initialBindings.Order(), repeatedBindings.Order());
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void Compile_HighArityFluentUsesRuntimeFacts(bool isPositive)
    {
        DomainBuilder builder = new DomainBuilder("d")
            .Requirements().Add(":strips").Add(":negative-preconditions").Close()
            .Predicates().Add("triple", ("?x", "object"), ("?y", "object"), ("?z", "object"))
                .Add("done").Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("inspect").AddParameter("?x").AddParameter("?y").AddParameter("?z")
            .AddPrecondition("triple", isPositive ? Polarity.Positive : Polarity.Negative,
                "?x", "?y", "?z").AddEffect("done").Close();
        actions.Add("produce").AddParameter("?x").AddParameter("?y").AddParameter("?z")
            .AddEffect("triple", "?x", "?y", "?z").Close();
        Domain domain = actions.Close().Build();
        Problem problem = new ProblemBuilder(domain, "p")
            .Objects().Add("a").Add("b").Add("c").Close().Build();
        ActionSchema schema = problem.Domain.Actions.Single(action => action.Name == "inspect");
        Mimir.Core.Schemas.Predicate<Fluent> triple = problem.Domain.Fluents
            .Single(predicate => predicate.Name == "triple");
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Constant c = problem.ObjectLookup["c"];
        State initialState = problem.InitialState;
        var generator = new ConjunctiveConditionBindingGenerator();

        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        Assert.Equal(isPositive ? 0 : 27, generator.CountBindings(condition, initialState.Expand()));

        Fact<Fluent> matchingFact = problem.Context.RegisterFact(triple, [a, b, c]);
        State stateWithMatchingFact = initialState.WithAdditionalFluentFacts([matchingFact]);
        Assert.Equal(isPositive ? 1 : 26, generator.CountBindings(condition, stateWithMatchingFact.Expand()));

        Fact<Fluent> secondMatchingFact = problem.Context.RegisterFact(triple, [c, b, a]);
        Assert.Equal(isPositive ? 1 : 26, generator.CountBindings(condition, stateWithMatchingFact.Expand()));
        State stateWithTwoMatchingFacts = stateWithMatchingFact.WithAdditionalFluentFacts([secondMatchingFact]);
        var bindings = new List<string>();
        generator.EnumerateBindings(condition, stateWithTwoMatchingFacts.Expand(), binding =>
        {
            bindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        Assert.Equal(isPositive ? 2 : 25, bindings.Count);
        if (isPositive)
            Assert.Equal(new[] { "a b c", "c b a" }.Order(), bindings.Order());
        else
        {
            Assert.DoesNotContain("a b c", bindings);
            Assert.DoesNotContain("c b a", bindings);
        }
    }

    [Fact]
    public void EnumerateBindings_SparseRelationMatchesRepeatedVariablesConstantsAndPermutedSlots()
    {
        Problem problem = CreateProblemForEnumerateBindings_SparseRelationMatchesRepeatedVariablesConstantsAndPermutedSlots();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        var bindings = new List<string>();
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            bindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        Assert.Equal(2, count);
        Assert.Equal(new[] { "a b c", "c b a" }.Order(), bindings.Order());
    }

    [Fact]
    public void EnumerateBindings_MultipleSparseRelationsIntersect()
    {
        Problem problem = CreateProblemForEnumerateBindings_MultipleSparseRelationsIntersect();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        var bindings = new List<string>();
        int count = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            bindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        Assert.Equal(2, count);
        Assert.Equal(new[] { "a b c", "c a b" }.Order(), bindings.Order());
    }

    [Fact]
    public void EnumerateBindings_SparseRelationHonorsLimitsAndCancellation()
    {
        Problem problem = CreateProblemForEnumerateBindings_RespectsMaxBindings0();
        ActionSchema schema = problem.Domain.Actions.Single(action => action.Name == "inspect");
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition condition = generator.Compile(
            problem,
            schema.Parameters,
            schema.FluentPreconditions,
            schema.StaticPreconditions,
            schema.DerivedPreconditions);

        var allBindings = new List<string>();
        int allCount = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            allBindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        var limitedBindings = new List<string>();
        int limitedCount = generator.EnumerateBindings(condition, problem.InitialState.Expand(), 2, binding =>
        {
            limitedBindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return true;
        });

        var cancelledBindings = new List<string>();
        int cancelledCount = generator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            cancelledBindings.Add(string.Join(" ", binding.ToArray().Select(constant => constant.Name)));
            return false;
        });

        Assert.Equal(3, allCount);
        Assert.Equal(new[] { "a a c", "b a b", "c a c" }.Order(), allBindings.Order());
        Assert.Equal(2, limitedCount);
        Assert.Equal(2, limitedBindings.Count);
        Assert.Equal(limitedBindings.Count, limitedBindings.Distinct().Count());
        Assert.All(limitedBindings, binding => Assert.Contains(binding, allBindings));
        Assert.Equal(2, generator.CountBindings(condition, problem.InitialState.Expand(), 2));
        Assert.Equal(1, cancelledCount);
        Assert.Single(cancelledBindings);
        Assert.Contains(cancelledBindings[0], allBindings);

        var repeatedBindings = new List<string>();
        int repeatedCount = generator.EnumerateBindings(
            condition,
            problem.InitialState.Expand(),
            binding =>
            {
                repeatedBindings.Add(string.Join(
                    " ",
                    binding.ToArray().Select(constant => constant.Name)));
                return true;
            });

        Assert.Equal(allCount, repeatedCount);
        Assert.Equal(allBindings.Order(), repeatedBindings.Order());
    }

    [Fact]
    public void CompiledCondition_CanBeReusedAcrossStates()
    {
        var problem = CreateProblemForEnumerateBindings_RespectsMaxBindings1();

        var inspectSchema = problem.Domain.Actions.Single(action => action.Name == "inspect");
        var bindingGenerator = new ConjunctiveConditionBindingGenerator();
        var condition = bindingGenerator.Compile(
            problem,
            inspectSchema.Parameters,
            inspectSchema.FluentPreconditions,
            inspectSchema.StaticPreconditions,
            inspectSchema.DerivedPreconditions);

        var groundedGenerator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());
        var activateO2 = groundedGenerator.GetApplicableActions(problem.InitialState.Expand())
            .Single(action => action.Schema.Name == "activate" && action.Arguments[0].Name == "o2");
        var nextState = problem.InitialState.Expand().Apply(activateO2);

        var initialBindings = new List<string>();
        bindingGenerator.EnumerateBindings(condition, problem.InitialState.Expand(), binding =>
        {
            initialBindings.Add(binding[0].Name);
            return true;
        });

        var nextBindings = new List<string>();
        bindingGenerator.EnumerateBindings(condition, nextState.Expand(), binding =>
        {
            nextBindings.Add(binding[0].Name);
            return true;
        });

        Assert.Equal(new[] { "o1" }, initialBindings.OrderBy(name => name).ToArray());
        Assert.Equal(new[] { "o1", "o2" }, nextBindings.OrderBy(name => name).ToArray());
    }

}
