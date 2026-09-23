using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public class PublicModelNullValidationTests
{
    [Fact]
    public void DomainIsCompatible_RequiresBothTypesToBeDeclared()
    {
        Domain domain = new DomainBuilder("compatibility")
            .Requirements().Add(":strips").Add(":typing").Close()
            .Types().Add("vehicle").Add("car", "vehicle").Close()
            .Predicates().Add("ready", ("?x", "vehicle")).Close()
            .Build();

        Assert.True(domain.IsCompatible("object", "object"));
        Assert.True(domain.IsCompatible("vehicle", "object"));
        Assert.True(domain.IsCompatible("car", "vehicle"));
        Assert.True(domain.IsCompatible("car", "object"));
        Assert.True(domain.IsCompatible("CAR", "VeHiClE"));
        Assert.False(domain.IsCompatible("vehicle", "car"));
        Assert.False(domain.IsCompatible("missing", "object"));
        Assert.False(domain.IsCompatible("missing", "missing"));
        Assert.False(domain.IsCompatible("car", "missing"));
        AssertParam<ArgumentNullException>("child", () => domain.IsCompatible(null!, "object"));
        AssertParam<ArgumentNullException>("parent", () => domain.IsCompatible("object", null!));
    }

    [Fact]
    public void DomainIsCompatible_UsesProgrammaticTypeHierarchy()
    {
        Domain domain = CreateEmptyDomain(new Dictionary<string, string>
        {
            ["vehicle"] = "object",
            ["car"] = "vehicle"
        });

        Assert.True(domain.IsCompatible("car", "vehicle"));
        Assert.True(domain.IsCompatible("CAR", "OBJECT"));
        Assert.False(domain.IsCompatible("vehicle", "car"));
        Assert.False(domain.IsCompatible("car", "missing"));
    }

    [Fact]
    public void DomainConstructor_RejectsNullRequirementAndTypeHierarchyValue()
    {
        AssertParam<ArgumentException>("requirements", () => CreateEmptyDomain(
            new Dictionary<string, string>(),
            new string[] { null! }));
        AssertParam<ArgumentException>("requirements", () => CreateEmptyDomain(
            new Dictionary<string, string>(),
            [" "]));

        var hierarchy = new Dictionary<string, string>
        {
            ["child"] = null!
        };
        AssertParam<ArgumentException>("typeHierarchy", () => CreateEmptyDomain(hierarchy));
        AssertParam<ArgumentException>("typeHierarchy", () => CreateEmptyDomain(
            new Dictionary<string, string> { ["child"] = " " }));
        AssertParam<ArgumentException>("typeHierarchy", () => CreateEmptyDomain(
            new Dictionary<string, string> { [" "] = "object" }));
    }

    [Fact]
    public void PredicateFunctionAndAtom_RejectNullMembers()
    {
        var parameter = new Variable("?x");
        var predicate = new FluentPredicate("p", [parameter]);

        AssertParam<ArgumentException>("parameters", () =>
            new FluentPredicate("p", new Variable[] { null! }));
        AssertParam<ArgumentException>("parameters", () =>
            new NumericFunction("cost", new Variable[] { null! }));
        AssertParam<ArgumentNullException>("predicate", () =>
            new Atom<Fluent>(null!, Array.Empty<ITerm>()));
        AssertParam<ArgumentNullException>("arguments", () =>
            new Atom<Fluent>(predicate, null!));
        AssertParam<ArgumentException>("arguments", () =>
            new Atom<Fluent>(predicate, new ITerm[] { null! }));
    }

    [Fact]
    public void ActionSchema_RejectsNullCollectionsValuesAndCost()
    {
        SchemaParts parts = CreateSchemaParts();

        AssertParam<ArgumentNullException>("parameters", () => new ActionSchema(
            "a", null!, parts.FluentLiterals, parts.StaticLiterals, parts.DerivedLiterals,
            [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentNullException>("fluentPreconditions", () => new ActionSchema(
            "a", parts.Parameters, null!, parts.StaticLiterals, parts.DerivedLiterals,
            [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentNullException>("staticPreconditions", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, null!, parts.DerivedLiterals,
            [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentNullException>("derivedPreconditions", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, parts.StaticLiterals, null!,
            [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentNullException>("effects", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, parts.StaticLiterals, parts.DerivedLiterals,
            [], null!, [], parts.Cost));
        AssertParam<ArgumentNullException>("costExpression", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, parts.StaticLiterals, parts.DerivedLiterals,
            [], parts.Effects, [], null!));

        AssertParam<ArgumentException>("parameters", () => new ActionSchema(
            "a", new Variable[] { null! }, parts.FluentLiterals, parts.StaticLiterals,
            parts.DerivedLiterals, [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentException>("fluentPreconditions", () => new ActionSchema(
            "a", parts.Parameters, new Literal<Atom<Fluent>>[] { null! }, parts.StaticLiterals,
            parts.DerivedLiterals, [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentException>("staticPreconditions", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, new Literal<Atom<Static>>[] { null! },
            parts.DerivedLiterals, [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentException>("derivedPreconditions", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, parts.StaticLiterals,
            new Literal<Atom<Derived>>[] { null! }, [], parts.Effects, [], parts.Cost));
        AssertParam<ArgumentException>("effects", () => new ActionSchema(
            "a", parts.Parameters, parts.FluentLiterals, parts.StaticLiterals,
            parts.DerivedLiterals, [], new ConditionalEffect[] { null! }, [], parts.Cost));
    }

    [Fact]
    public void ConditionalEffect_RejectsNullCollectionsValuesAndEffect()
    {
        SchemaParts parts = CreateSchemaParts();

        AssertParam<ArgumentNullException>("quantifiedVariables", () => new ConditionalEffect(
            null!, parts.FluentLiterals, parts.StaticLiterals, parts.DerivedLiterals,
            [],
            parts.FluentLiterals[0]));
        AssertParam<ArgumentNullException>("fluentConditions", () => new ConditionalEffect(
            parts.Parameters, null!, parts.StaticLiterals, parts.DerivedLiterals,
            [],
            parts.FluentLiterals[0]));
        AssertParam<ArgumentNullException>("staticConditions", () => new ConditionalEffect(
            parts.Parameters, parts.FluentLiterals, null!, parts.DerivedLiterals,
            [],
            parts.FluentLiterals[0]));
        AssertParam<ArgumentNullException>("derivedConditions", () => new ConditionalEffect(
            parts.Parameters, parts.FluentLiterals, parts.StaticLiterals, null!,
            [],
            parts.FluentLiterals[0]));
        AssertParam<ArgumentNullException>("effect", () => new ConditionalEffect(
            parts.Parameters, parts.FluentLiterals, parts.StaticLiterals, parts.DerivedLiterals,
            [],
            null!));

        AssertParam<ArgumentException>("quantifiedVariables", () => new ConditionalEffect(
            new Variable[] { null! }, parts.FluentLiterals, parts.StaticLiterals,
            parts.DerivedLiterals, [], parts.FluentLiterals[0]));
        AssertParam<ArgumentException>("fluentConditions", () => new ConditionalEffect(
            parts.Parameters, new Literal<Atom<Fluent>>[] { null! }, parts.StaticLiterals,
            parts.DerivedLiterals, [], parts.FluentLiterals[0]));
        AssertParam<ArgumentException>("staticConditions", () => new ConditionalEffect(
            parts.Parameters, parts.FluentLiterals, new Literal<Atom<Static>>[] { null! },
            parts.DerivedLiterals, [], parts.FluentLiterals[0]));
        AssertParam<ArgumentException>("derivedConditions", () => new ConditionalEffect(
            parts.Parameters, parts.FluentLiterals, parts.StaticLiterals,
            new Literal<Atom<Derived>>[] { null! }, [], parts.FluentLiterals[0]));
    }

    [Fact]
    public void ActionCostRecords_RejectNullMembersOnConstructionAndReplacement()
    {
        var constant = new NumericConstant(1d);
        var binary = new NumericBinaryExpression(NumericOperator.Add, constant, constant);
        var function = new NumericFunction("cost", Array.Empty<Variable>());
        var argument = new Constant("value", "object");
        var functionCost = new FunctionCall(function, [argument]);

        AssertParam<ArgumentNullException>("Left", () =>
            new NumericBinaryExpression(NumericOperator.Add, null!, constant));
        AssertParam<ArgumentNullException>("Right", () =>
            new NumericBinaryExpression(NumericOperator.Add, constant, null!));
        AssertParam<ArgumentNullException>("Left", () => _ = binary with { Left = null! });
        AssertParam<ArgumentNullException>("Right", () => _ = binary with { Right = null! });

        AssertParam<ArgumentNullException>("Function", () =>
            new FunctionCall(null!, [argument]));
        AssertParam<ArgumentNullException>("Arguments", () =>
            new FunctionCall(function, null!));
        AssertParam<ArgumentException>("Arguments", () =>
            new FunctionCall(function, new ITerm[] { null! }));
        AssertParam<ArgumentNullException>("Function", () =>
            _ = functionCost with { Function = null! });
        AssertParam<ArgumentNullException>("Arguments", () =>
            _ = functionCost with { Arguments = null! });
        AssertParam<ArgumentException>("Arguments", () =>
            _ = functionCost with { Arguments = new ITerm[] { null! } });
    }

    [Fact]
    public void LiteralRecord_ValidatesMembersAndPreservesRecordBehavior()
    {
        var literal = new Literal<string>("value", Polarity.Positive);

        AssertParam<ArgumentNullException>("Value", () =>
            new Literal<string>(null!, Polarity.Positive));
        AssertParam<ArgumentNullException>("Value", () => _ = literal with { Value = null! });
        AssertParam<ArgumentOutOfRangeException>("Polarity", () =>
            new Literal<string>("value", (Polarity)42));
        AssertParam<ArgumentOutOfRangeException>("Polarity", () =>
            _ = literal with { Polarity = (Polarity)42 });

        var negative = new Literal<string>("value", Polarity.Negative);
        var (value, polarity) = negative;

        Assert.Equal("value", value);
        Assert.Equal(Polarity.Negative, polarity);
        Assert.Equal(new Literal<string>("value", Polarity.Negative), negative);
        Assert.Equal("(not value)", negative.ToString());
    }

    [Fact]
    public void GroundedScalarRecords_RejectNullOnConstructionAndReplacement()
    {
        var truth = new GroundedTrue();
        var predicate = new StaticPredicate("p", Array.Empty<Variable>());
        var term = new Constant("value", "object");
        var atom = new GroundedAtom(predicate, [term]);
        var not = new GroundedNot(truth);
        var imply = new GroundedImply(truth, truth);

        AssertParam<ArgumentNullException>("Predicate", () =>
            new GroundedAtom(null!, Array.Empty<ITerm>()));
        AssertParam<ArgumentNullException>("Predicate", () =>
            _ = atom with { Predicate = null! });
        AssertParam<ArgumentNullException>("Expression", () => new GroundedNot(null!));
        AssertParam<ArgumentNullException>("Expression", () =>
            _ = not with { Expression = null! });
        AssertParam<ArgumentNullException>("Antecedent", () => new GroundedImply(null!, truth));
        AssertParam<ArgumentNullException>("Consequent", () => new GroundedImply(truth, null!));
        AssertParam<ArgumentNullException>("Antecedent", () =>
            _ = imply with { Antecedent = null! });
        AssertParam<ArgumentNullException>("Consequent", () =>
            _ = imply with { Consequent = null! });
    }

    [Fact]
    public void GroundedCollectionRecords_RejectNullDefaultAndNullElements()
    {
        var truth = new GroundedTrue();
        var predicate = new StaticPredicate("p", Array.Empty<Variable>());
        var term = new Constant("value", "object");
        var atom = new GroundedAtom(predicate, [term]);
        var and = new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(truth));
        var or = new GroundedOr(ImmutableArray.Create<IGroundedExpression>(truth));

        AssertParam<ArgumentNullException>("Arguments", () => new GroundedAtom(predicate, null!));
        AssertParam<ArgumentException>("Arguments", () =>
            new GroundedAtom(predicate, new ITerm[] { null! }));
        AssertParam<ArgumentNullException>("Arguments", () =>
            _ = atom with { Arguments = null! });
        AssertParam<ArgumentException>("Arguments", () =>
            _ = atom with { Arguments = new ITerm[] { null! } });

        AssertParam<ArgumentException>("Expressions", () =>
            new GroundedAnd(default(ImmutableArray<IGroundedExpression>)));
        AssertParam<ArgumentException>("Expressions", () =>
            new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(new IGroundedExpression[] { null! })));
        AssertParam<ArgumentException>("Expressions", () =>
            _ = and with { Expressions = default });
        AssertParam<ArgumentException>("Expressions", () =>
            _ = and with
            {
                Expressions = ImmutableArray.Create<IGroundedExpression>(new IGroundedExpression[] { null! })
            });

        AssertParam<ArgumentException>("Expressions", () =>
            new GroundedOr(default(ImmutableArray<IGroundedExpression>)));
        AssertParam<ArgumentException>("Expressions", () =>
            new GroundedOr(ImmutableArray.Create<IGroundedExpression>(new IGroundedExpression[] { null! })));
        AssertParam<ArgumentException>("Expressions", () =>
            _ = or with { Expressions = default });
        AssertParam<ArgumentException>("Expressions", () =>
            _ = or with
            {
                Expressions = ImmutableArray.Create<IGroundedExpression>(new IGroundedExpression[] { null! })
            });
    }

    [Fact]
    public void GroundedQuantifiers_RejectNullMembersOnConstructionAndReplacement()
    {
        var variable = new Variable("?x");
        var truth = new GroundedTrue();
        var forall = new GroundedForall([variable], truth);
        var exists = new GroundedExists([variable], truth);

        AssertParam<ArgumentNullException>("Variables", () => new GroundedForall(null!, truth));
        AssertParam<ArgumentException>("Variables", () =>
            new GroundedForall(new Variable[] { null! }, truth));
        AssertParam<ArgumentNullException>("Body", () => new GroundedForall([variable], null!));
        AssertParam<ArgumentNullException>("Variables", () =>
            _ = forall with { Variables = null! });
        AssertParam<ArgumentException>("Variables", () =>
            _ = forall with { Variables = new Variable[] { null! } });
        AssertParam<ArgumentNullException>("Body", () => _ = forall with { Body = null! });

        AssertParam<ArgumentNullException>("Variables", () => new GroundedExists(null!, truth));
        AssertParam<ArgumentException>("Variables", () =>
            new GroundedExists(new Variable[] { null! }, truth));
        AssertParam<ArgumentNullException>("Body", () => new GroundedExists([variable], null!));
        AssertParam<ArgumentNullException>("Variables", () =>
            _ = exists with { Variables = null! });
        AssertParam<ArgumentException>("Variables", () =>
            _ = exists with { Variables = new Variable[] { null! } });
        AssertParam<ArgumentNullException>("Body", () => _ = exists with { Body = null! });
    }

    private static SchemaParts CreateSchemaParts()
    {
        var parameter = new Variable("?x");
        var fluentPredicate = new FluentPredicate("fluent", [parameter]);
        var staticPredicate = new StaticPredicate("static", [parameter]);
        var derivedPredicate = new DerivedPredicate("derived", [parameter]);
        var fluentLiteral = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(fluentPredicate, [parameter]),
            Polarity.Positive);
        var staticLiteral = new Literal<Atom<Static>>(
            new Atom<Static>(staticPredicate, [parameter]),
            Polarity.Positive);
        var derivedLiteral = new Literal<Atom<Derived>>(
            new Atom<Derived>(derivedPredicate, [parameter]),
            Polarity.Positive);
        var effect = new ConditionalEffect(
            Array.Empty<Variable>(),
            Array.Empty<Literal<Atom<Fluent>>>(),
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            [],
            fluentLiteral);

        return new SchemaParts(
            [parameter],
            [fluentLiteral],
            [staticLiteral],
            [derivedLiteral],
            [effect],
            new NumericConstant(1d));
    }

    private static Domain CreateEmptyDomain(
        IReadOnlyDictionary<string, string> typeHierarchy,
        IReadOnlyList<string>? requirements = null)
        => new(
            "empty",
            Array.Empty<FluentPredicate>(),
            Array.Empty<StaticPredicate>(),
            Array.Empty<DerivedPredicateDefinition>(),
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            typeHierarchy,
            requirements);

    private static TException AssertParam<TException>(string parameterName, System.Action action)
        where TException : ArgumentException
    {
        TException exception = Assert.Throws<TException>(action);
        Assert.Equal(parameterName, exception.ParamName);
        return exception;
    }

    private sealed record SchemaParts(
        IReadOnlyList<Variable> Parameters,
        IReadOnlyList<Literal<Atom<Fluent>>> FluentLiterals,
        IReadOnlyList<Literal<Atom<Static>>> StaticLiterals,
        IReadOnlyList<Literal<Atom<Derived>>> DerivedLiterals,
        IReadOnlyList<ConditionalEffect> Effects,
        NumericExpression Cost);
}
