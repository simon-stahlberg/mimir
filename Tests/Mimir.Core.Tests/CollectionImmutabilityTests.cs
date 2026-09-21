using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using GroundAction = Mimir.Core.Grounding.Action;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public class CollectionImmutabilityTests
{
    [Fact]
    public void SchemaCollectionsCannotBeMutatedThroughCasts()
    {
        var parameter = new Variable("?x");
        var quantified = new Variable("?y");
        var fluentPredicate = new FluentPredicate("fluent", [parameter]);
        var staticPredicate = new StaticPredicate("static", [parameter]);
        var derivedPredicate = new DerivedPredicate("derived", [parameter]);
        var function = new NumericFunction("cost", [parameter]);
        var fluentAtom = new Atom<Fluent>(fluentPredicate, [parameter]);
        var staticAtom = new Atom<Static>(staticPredicate, [parameter]);
        var derivedAtom = new Atom<Derived>(derivedPredicate, [parameter]);
        var fluentLiteral = new Literal<Atom<Fluent>>(fluentAtom, Polarity.Positive);
        var staticLiteral = new Literal<Atom<Static>>(staticAtom, Polarity.Positive);
        var derivedLiteral = new Literal<Atom<Derived>>(derivedAtom, Polarity.Positive);
        var effect = new ConditionalEffect(
            [quantified],
            [fluentLiteral],
            [staticLiteral],
            [derivedLiteral],
            fluentLiteral);
        var schema = new ActionSchema(
            "act",
            [parameter],
            [fluentLiteral],
            [staticLiteral],
            [derivedLiteral],
            [effect],
            new NumericConstant(1d));

        AssertReadOnly(fluentPredicate.Parameters);
        AssertReadOnly(function.Parameters);
        AssertReadOnly(fluentAtom.Arguments);
        AssertReadOnly(effect.QuantifiedVariables);
        AssertReadOnly(effect.FluentConditions);
        AssertReadOnly(effect.StaticConditions);
        AssertReadOnly(effect.DerivedConditions);
        AssertReadOnly(schema.Parameters);
        AssertReadOnly(schema.FluentPreconditions);
        AssertReadOnly(schema.StaticPreconditions);
        AssertReadOnly(schema.DerivedPreconditions);
        AssertReadOnly(schema.Effects);
    }

    [Fact]
    public void NumericFunctionActionCostExpressionCopiesReplacementArguments()
    {
        var functionParameter = new Variable("?value");
        var function = new NumericFunction("cost", [functionParameter]);
        var first = new Constant("first", "object");
        var second = new Constant("second", "object");
        var replacementArguments = new List<ITerm> { first };
        var original = new FunctionCall(function, [first]);
        FunctionCall replacement = original with
        {
            Arguments = replacementArguments
        };

        replacementArguments[0] = second;

        Assert.Same(first, Assert.Single(replacement.Arguments));
    }

    [Fact]
    public void DomainAndProblemCollectionsCannotBeMutatedThroughCasts()
    {
        (Domain domain, Problem problem) = CreateDomainAndProblem();

        AssertReadOnly(domain.Requirements);
        AssertReadOnly(domain.ExpandedRequirements);
        AssertReadOnly(domain.Fluents);
        AssertReadOnly(domain.Statics);
        AssertReadOnly(domain.Derived);
        AssertReadOnly(domain.Constants);
        AssertReadOnly(domain.Actions);
        AssertReadOnly(domain.TypeHierarchy);
        AssertReadOnly(domain.DerivedDefinitions);
        AssertReadOnly(problem.DeclaredObjects);
        AssertReadOnly(problem.AllObjects);
        AssertReadOnly(problem.ObjectLookup);
        AssertReadOnly(problem.AllPredicates);
        AssertReadOnly(problem.Goal);
    }

    [Fact]
    public void RuntimeCollectionsCannotBeMutatedThroughCasts()
    {
        (Domain domain, Problem problem) = CreateDomainAndProblem();
        Constant obj = problem.ObjectLookup["o"];
        StaticPredicate staticPredicate = Assert.Single(domain.Statics);
        Fact<Static> fact = Assert.IsType<Fact<Static>>(
            problem.Context.FindFact(staticPredicate, [obj]));
        GroundAction action = Assert.Single(new RpgGrounder().Ground(problem, problem.InitialState));

        Variable first = problem.NewVariable("?first", "thing");
        Variable second = problem.NewVariable("?second", "thing");
        Atom<Static> atom = problem.NewAtom(staticPredicate, [first]);
        Literal<Atom<Static>> literal = problem.NewLiteral(atom, isPositive: true);
        ConjunctiveCondition condition = problem.NewConjunctiveCondition(
            [first, second],
            [literal]);
        CompiledConjunctiveCondition compiled =
            new ConjunctiveConditionBindingGenerator().Compile(condition, problem);

        int conditionHash = condition.GetHashCode();
        AssertReadOnly(fact.Arguments);
        AssertReadOnly(action.Arguments);
        AssertReadOnly(action.DerivedPreconditions);
        AssertReadOnly(action.ConditionalEffects);
        AssertReadOnly(condition.Parameters);
        AssertReadOnly(condition.StaticLiterals);
        AssertReadOnly(compiled.Variables);
        Assert.Equal(conditionHash, condition.GetHashCode());
    }

    private static (Domain Domain, Problem Problem) CreateDomainAndProblem()
    {
        DomainBuilder domainBuilder = new DomainBuilder("immutable")
            .Requirements()
                .Add(":strips")
                .Add(":typing")
                .Add(":negative-preconditions")
                .Add(":conditional-effects")
                .Add(":derived-predicates")
                .Close()
            .Types().Add("thing").Close()
            .Constants().Add("c", "thing").Close()
            .Predicates()
                .Add("base", ("?x", "thing"))
                .Add("fluent", ("?x", "thing"))
                .Add("ready", ("?x", "thing"))
                .Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act")
            .AddParameter("?x", "thing")
            .AddPrecondition("base", "?x")
            .AddPrecondition("ready", "?x")
            .AddPrecondition("fluent", Polarity.Negative, "?x");
        action.AddConditionalEffect()
            .AddCondition("base", "?x")
            .AddEffect("fluent", "?x")
            .Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.DerivedPredicates()
            .Define("ready", Logic.Atom("base", "?x"))
            .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "immutable-problem")
            .Objects().Add("o", "thing").Close()
            .InitialState().AddFact("base", "o").Close()
            .Goal().Add("fluent", "o").Close()
            .Build();
        return (domain, problem);
    }

    private static void AssertReadOnly<T>(IReadOnlyList<T> values)
    {
        T value = Assert.Single(values.Take(1));
        IList<T> list = Assert.IsAssignableFrom<IList<T>>(values);

        Assert.Throws<NotSupportedException>(() => list[0] = value);
    }

    private static void AssertReadOnly<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> values)
        where TKey : notnull
    {
        KeyValuePair<TKey, TValue> entry = Assert.Single(values.Take(1));
        IDictionary<TKey, TValue> dictionary = Assert.IsAssignableFrom<IDictionary<TKey, TValue>>(values);

        Assert.Throws<NotSupportedException>(() => dictionary[entry.Key] = entry.Value);
    }
}
