using System.Reflection;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public class BuilderGuardTests
{
    [Fact]
    public void BuildersInferPredicateTypesAndReuseResolvedRuntimeIdentities()
    {
        Domain domain = new DomainBuilder("identity")
            .Requirements()
                .Add(":strips")
                .Add(":typing")
                .Add(":equality")
                .Add(":derived-predicates")
                .Close()
            .Types()
                .Add("item")
                .Close()
            .Constants()
                .Add("home", "item")
                .Close()
            .Predicates()
                .Add("ready", ("?item", "item"))
                .Add("marked", ("?item", "item"))
                .Add("eligible", ("?item", "item"))
                .Close()
            .Actions()
                .Add("mark")
                    .AddParameter("?item", "item")
                    .AddPrecondition("ready", "?item")
                    .AddPrecondition("eligible", "?item")
                    .AddPrecondition("=", "?item", "home")
                    .AddEffect("marked", "?item")
                    .Close()
                .Close()
            .DerivedPredicates()
                .Define("eligible", Logic.Atom("ready", "?item"))
                .Close()
            .Build();

        StaticPredicate ready = Assert.Single(domain.Statics, predicate => predicate.Name == "ready");
        FluentPredicate marked = Assert.Single(domain.Fluents);
        DerivedPredicate eligible = Assert.Single(domain.Derived);
        ActionSchema action = Assert.Single(domain.Actions);

        Assert.Same(
            ready,
            Assert.Single(action.StaticPreconditions, literal => literal.Value.Predicate.Name == "ready")
                .Value.Predicate);
        Assert.Same(marked, Assert.Single(action.Effects).Effect.Value.Predicate);
        Assert.Same(eligible, Assert.Single(action.DerivedPreconditions).Value.Predicate);
        Assert.Same(ready, ((GroundedAtom)domain.DerivedDefinitions["eligible"]).Predicate);
        Assert.Same(domain.EqualityPredicate, action.StaticPreconditions[1].Value.Predicate);

        Problem problem = new ProblemBuilder(domain, "identity-problem")
            .Objects()
                .Add("a", "item")
                .Close()
            .InitialState()
                .AddFact("ready", "a")
                .Close()
            .Goal()
                .Add("marked", "a")
                .Add("=", "a", "a")
                .Close()
            .Build();

        Assert.Same(ready, problem.Context.RegisterFact(ready, [problem.ObjectLookup["a"]]).Predicate);
        Assert.Same(marked, problem.Goal[0].Value.Predicate);
        Assert.Same(domain.EqualityPredicate, problem.Goal[1].Value.Predicate);
        Assert.Same(problem.AllObjects[1], problem.ObjectLookup["a"]);
    }

    [Fact]
    public void DomainSectionsAreOrderedSingleUseAndLockTheirParent()
    {
        var builder = new DomainBuilder("sections");
        RequirementListBuilder requirements = builder.Requirements();

        Assert.Throws<InvalidOperationException>(() => builder.Build());
        Assert.Throws<InvalidOperationException>(() => builder.Types());

        Assert.Same(builder, requirements.Add(":strips").Close());
        Assert.Throws<InvalidOperationException>(() => requirements.Add(":typing"));
        Assert.Throws<InvalidOperationException>(() => builder.Requirements());

        PredicateListBuilder predicates = builder.Predicates();
        Assert.Throws<InvalidOperationException>(() => builder.Types());
        Assert.Same(builder, predicates.Close());
        Assert.Throws<InvalidOperationException>(() => builder.Constants());

        Domain domain = builder.Build();
        Assert.Empty(domain.TypeHierarchy);
        Assert.Empty(domain.Constants);
        Assert.Throws<InvalidOperationException>(() => builder.Build());
        Assert.Throws<InvalidOperationException>(() => builder.Actions());
    }

    [Fact]
    public void FailedSectionCloseAndActionCloseRemainOpenAndCommitTransactionally()
    {
        var builder = new DomainBuilder("transactional");
        TypeListBuilder types = builder.Types().Add("child", "parent");

        Assert.Throws<InvalidOperationException>(() => types.Close());
        types.Add("parent").Close();

        builder.Predicates()
            .Add("p", ("?x", "child"))
            .Close();
        ActionListBuilder actions = builder.Actions();
        ActionSchemaBuilder action = actions.Add("act")
            .AddPrecondition("p", "?x");

        Assert.Throws<InvalidOperationException>(() => action.Close());
        Assert.Throws<InvalidOperationException>(() => actions.Close());

        Assert.Same(action, action.AddParameter("?x", "child"));
        Assert.Same(actions, action.Close());
        Assert.Same(builder, actions.Close());

        Domain domain = builder.Build();
        Assert.Equal(["child", "parent"], domain.TypeHierarchy.Keys);
        Assert.Single(domain.Actions);
    }

    [Fact]
    public void ConditionalEffectCloseReturnsOneLevelAndSealsTheChild()
    {
        var builder = new DomainBuilder("conditional");
        builder.Requirements().Add(":adl").Add(":typing").Close();
        builder.Types().Add("item").Close();
        builder.Predicates()
            .Add("selected", ("?x", "item"))
            .Add("done", ("?x", "item"))
            .Close();

        ActionListBuilder actions = builder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        ConditionalEffectBuilder conditional = action.AddConditionalEffect()
            .AddParameter("?x", "item")
            .AddCondition("selected", "?x")
            .AddEffect("done", "?x");

        Assert.Throws<InvalidOperationException>(() => action.Close());
        Assert.Same(action, conditional.Close());
        Assert.Throws<InvalidOperationException>(() => conditional.AddCondition("selected", "?x"));
        Assert.Same(actions, action.Close());
        Assert.Same(builder, actions.Close());

        ConditionalEffect effect = Assert.Single(Assert.Single(builder.Build().Actions).Effects);
        Assert.Single(effect.QuantifiedVariables);
        Assert.Single(effect.StaticConditions);
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void EmptyProblemSupportsEachGeneratorType(ApplicableActionGeneratorType generatorType)
    {
        Domain domain = new DomainBuilder("empty").Build();
        Problem problem = new ProblemBuilder(domain, "empty-problem", generatorType).Build();

        Assert.Equal(generatorType, problem.GeneratorType);
        Assert.Empty(problem.AllObjects);
        Assert.Empty(problem.Goal);
        Assert.Empty(problem.GetApplicableActionGenerator(problem.InitialState)
            .GetApplicableActions(problem.InitialState.Expand()));
    }

    [Fact]
    public void ProblemSectionsAreOrderedSingleUseAndSealAfterSuccessfulBuild()
    {
        Domain domain = new DomainBuilder("problem-sections").Build();
        var builder = new ProblemBuilder(domain, "section-problem");
        ProblemObjectListBuilder objects = builder.Objects();

        Assert.Throws<InvalidOperationException>(() => builder.Build());
        Assert.Throws<InvalidOperationException>(() => builder.InitialState());
        Assert.Same(builder, objects.Close());
        Assert.Throws<InvalidOperationException>(() => builder.Objects());

        GoalBuilder goal = builder.Goal();
        Assert.Throws<InvalidOperationException>(() => builder.InitialState());
        Assert.Same(builder, goal.Close());

        builder.Build();
        Assert.Throws<InvalidOperationException>(() => builder.Build());
        Assert.Throws<InvalidOperationException>(() => builder.Goal());
    }

    [Fact]
    public void ProblemBuilderPreservesOriginalNumericValue()
    {
        Domain domain = new DomainBuilder("numeric")
            .Requirements()
                .Add(":strips")
                .Add(":action-costs")
                .Close()
            .Functions()
                .Add("value")
                .Close()
            .Actions()
                .Add("epsilon-cost")
                    .WithCost(double.Epsilon)
                    .Close()
                .Close()
            .Build();

        Problem problem = new ProblemBuilder(domain, "numeric-problem")
            .InitialState()
                .AddNumericInitialization("value", double.Epsilon)
                .Close()
            .Build();

        Assert.Equal(
            double.Epsilon,
            problem.GetNumericFunctionValue(domain.Functions.Single(), Array.Empty<Constant>()));
        Assert.Equal(
            double.Epsilon,
            Assert.IsType<ConstantActionCostExpression>(domain.Actions.Single().CostExpression).Value);
    }

    [Fact]
    public void BuildersRejectDuplicateDeclarationsAndInvalidBindings()
    {
        var domainBuilder = new DomainBuilder("guards");
        TypeListBuilder types = domainBuilder.Types().Add("item");
        Assert.Throws<ArgumentException>(() => types.Add("ITEM"));
        types.Close();

        PredicateListBuilder predicates = domainBuilder.Predicates()
            .Add("marked", ("?item", "item"));
        Assert.Throws<ArgumentException>(() => predicates.Add("MARKED"));
        predicates.Close();

        Domain domain = domainBuilder.Build();
        var problemBuilder = new ProblemBuilder(domain, "guards-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects().Add("a", "item");
        Assert.Throws<ArgumentException>(() => objects.Add("A", "item"));
        objects.Add("b").Close();

        InitialStateBuilder initial = problemBuilder.InitialState();
        Assert.Throws<ArgumentException>(() => initial.AddFact("marked"));
        Assert.Throws<ArgumentException>(() => initial.AddFact("marked", "b"));
        Assert.Throws<ArgumentException>(() => initial.AddFact("unknown", "a"));
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            initial.AddNumericInitialization("unknown", double.PositiveInfinity));
    }

    [Fact]
    public void ProblemBuilderRejectsEqualityDerivedAndDuplicateNumericInitializations()
    {
        Domain domain = new DomainBuilder("problem-guards")
            .Requirements()
                .Add(":adl")
                .Add(":derived-predicates")
                .Add(":action-costs")
                .Close()
            .Predicates()
                .Add("base")
                .Add("derived")
                .Close()
            .Functions()
                .Add("value")
                .Close()
            .DerivedPredicates()
                .Define("derived", Logic.Atom("base"))
                .Close()
            .Build();

        var problemBuilder = new ProblemBuilder(domain, "problem-guards-problem");
        InitialStateBuilder initial = problemBuilder.InitialState();
        Assert.Throws<ArgumentException>(() => initial.AddFact("="));
        Assert.Throws<ArgumentException>(() => initial.AddFact("derived"));
        initial.AddNumericInitialization("value", 1d);
        Assert.Throws<ArgumentException>(() => initial.AddNumericInitialization("VALUE", 2d));
        initial.Close();

        GoalBuilder goal = problemBuilder.Goal();
        Assert.Throws<ArgumentOutOfRangeException>(() => goal.Add("base", (Polarity)int.MaxValue));
    }

    [Fact]
    public void DomainBuilderRejectsInvalidDerivedAndEffectCombinations()
    {
        var derivedEffect = new DomainBuilder("derived-effect");
        derivedEffect.Requirements().Add(":adl").Add(":derived-predicates").Close();
        derivedEffect.Predicates().Add("p").Close();
        derivedEffect.Actions().Add("act").AddEffect("p").Close().Close();
        derivedEffect.DerivedPredicates().Define("p", Logic.True()).Close();
        Assert.Throws<InvalidOperationException>(() => derivedEffect.Build());

        var equalityEffect = new DomainBuilder("equality-effect");
        equalityEffect.Requirements().Add(":equality").Close();
        equalityEffect.Predicates().Close();
        ActionSchemaBuilder action = equalityEffect.Actions().Add("act").AddEffect("=");
        Assert.Throws<InvalidOperationException>(() => action.Close());

        var unknownDerived = new DomainBuilder("unknown-derived");
        DerivedPredicateListBuilder definitions = unknownDerived.DerivedPredicates()
            .Define("missing", Logic.True());
        Assert.Throws<InvalidOperationException>(() => definitions.Close());
    }

    [Fact]
    public void BuilderPublicSurfaceIsNameBasedAndDoesNotExposePddl()
    {
        Type[] builderTypes =
        [
            typeof(DomainBuilder),
            typeof(RequirementListBuilder),
            typeof(TypeListBuilder),
            typeof(ConstantListBuilder),
            typeof(PredicateListBuilder),
            typeof(NumericFunctionListBuilder),
            typeof(ActionListBuilder),
            typeof(ActionSchemaBuilder),
            typeof(ConditionalEffectBuilder),
            typeof(DerivedPredicateListBuilder),
            typeof(ProblemBuilder),
            typeof(ProblemObjectListBuilder),
            typeof(InitialStateBuilder),
            typeof(GoalBuilder),
            typeof(LogicalExpressionSpec),
            typeof(ActionCostSpec),
        ];
        Type[] forbiddenCoreHandles =
        [
            typeof(Predicate),
            typeof(Variable),
            typeof(Constant),
            typeof(NumericFunction),
            typeof(ActionSchema),
            typeof(ConditionalEffect),
            typeof(IGroundedExpression),
            typeof(ActionCostExpression),
        ];

        foreach (Type type in builderTypes)
        {
            foreach (MethodInfo method in type.GetMethods(BindingFlags.Instance | BindingFlags.Public))
            {
                Assert.DoesNotContain(forbiddenCoreHandles, forbidden =>
                    ContainsType(method.ReturnType, forbidden));
                foreach (ParameterInfo parameter in method.GetParameters())
                {
                    Assert.DoesNotContain(forbiddenCoreHandles, forbidden =>
                        ContainsType(parameter.ParameterType, forbidden));
                }
                Assert.DoesNotContain(
                    "Mimir.Pddl",
                    method.ReturnType.Assembly.GetName().Name ?? string.Empty);
                Assert.All(method.GetParameters(), parameter =>
                    Assert.DoesNotContain(
                        "Mimir.Pddl",
                        parameter.ParameterType.Assembly.GetName().Name ?? string.Empty));
            }
        }
    }

    private static bool ContainsType(Type candidate, Type forbidden)
    {
        if (candidate == forbidden) return true;
        if (candidate.IsArray) return ContainsType(candidate.GetElementType()!, forbidden);
        return candidate.IsGenericType
            && candidate.GetGenericArguments().Any(argument => ContainsType(argument, forbidden));
    }
}
