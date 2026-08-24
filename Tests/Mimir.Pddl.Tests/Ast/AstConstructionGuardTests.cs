using System.Collections.Immutable;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Tests.Ast;

public class AstConstructionGuardTests
{
    [Fact]
    public void NameBearingRecordsRejectInvalidNames()
    {
        System.Action[] invalidConstructions =
        [
            () => new Parameter("x"),
            () => new Parameter("?x", "and"),
            () => new PredicateDeclaration("not", ImmutableArray<Parameter>.Empty),
            () => new FunctionDeclaration("increase", ImmutableArray<Parameter>.Empty),
            () => new TypeDeclaration("domain"),
            () => new TypeDeclaration("item", "or"),
            () => new ActionDefinition("when", ImmutableArray<Parameter>.Empty, null, null),
            () => CreateDomain("domain"),
            () => new ProblemDefinition(
                "problem",
                "valid-domain",
                ImmutableArray<PddlRequirement>.Empty,
                ImmutableArray<TypeDeclaration>.Empty,
                ImmutableArray<IProblemInitElement>.Empty,
                new EmptyLogic(),
                null),
            () => CreateProblem("valid-problem", "domain"),
            () => new PredicateCall("and", ImmutableArray<Term>.Empty),
            () => new FluentCall("increase", ImmutableArray<Term>.Empty),
            () => new AmbiguousEquality("left", "or"),
            () => Term.Constant("not")
        ];

        foreach (System.Action construction in invalidConstructions)
            Assert.ThrowsAny<ArgumentException>(construction);
    }

    [Fact]
    public void EveryArrayBearingRecordRejectsDefaultArrays()
    {
        var logic = new EmptyLogic();
        var effect = new AndEffect(ImmutableArray<IEffect>.Empty);
        System.Action[] invalidConstructions =
        [
            () => new And(default),
            () => new Or(default),
            () => new Forall(default, logic),
            () => new Exists(default, logic),
            () => new PredicateCall("p", default),
            () => new FluentCall("f", default),
            () => new AndEffect(default),
            () => new ForallEffect(default, effect),
            () => new PredicateDeclaration("p", default),
            () => new FunctionDeclaration("f", default),
            () => new ActionDefinition("a", default, null, null),
            () => new DomainDefinition(
                "valid-domain",
                default,
                ImmutableArray<TypeDeclaration>.Empty,
                ImmutableArray<TypeDeclaration>.Empty,
                ImmutableArray<PredicateDeclaration>.Empty,
                ImmutableArray<FunctionDeclaration>.Empty,
                ImmutableArray<ActionDefinition>.Empty,
                ImmutableArray<DerivedPredicate>.Empty),
            () => new ProblemDefinition(
                "valid-problem",
                "valid-domain",
                default,
                ImmutableArray<TypeDeclaration>.Empty,
                ImmutableArray<IProblemInitElement>.Empty,
                logic,
                null)
        ];

        foreach (System.Action construction in invalidConstructions)
            Assert.Throws<ArgumentException>(construction);
    }

    [Fact]
    public void RequiredChildRecordsRejectNull()
    {
        var predicate = new PredicateCall("p", ImmutableArray<Term>.Empty);
        var fluent = new FluentCall("f", ImmutableArray<Term>.Empty);
        var number = new NumberLiteral(1m);
        var logic = new EmptyLogic();
        var effect = new AndEffect(ImmutableArray<IEffect>.Empty);
        System.Action[] invalidConstructions =
        [
            () => new Not(null!),
            () => new Imply(null!, logic),
            () => new Imply(logic, null!),
            () => new Equality(null!, Term.Constant("x")),
            () => new Forall(ImmutableArray<Parameter>.Empty, null!),
            () => new Exists(ImmutableArray<Parameter>.Empty, null!),
            () => new Comparison(ComparisonOperator.Equal, null!, number),
            () => new Add(null!, number),
            () => new Negate(null!),
            () => new Subtract(number, null!),
            () => new Multiply(null!, number),
            () => new Divide(number, null!),
            () => new AddEffect(null!),
            () => new DeleteEffect(null!),
            () => new Assign(null!, number),
            () => new Increase(fluent, null!),
            () => new Decrease(null!, number),
            () => new ScaleUp(fluent, null!),
            () => new ScaleDown(null!, number),
            () => new ConditionalEffect(null!, effect),
            () => new ConditionalEffect(logic, null!),
            () => new ForallEffect(ImmutableArray<Parameter>.Empty, null!),
            () => new DerivedPredicate(null!, logic),
            () => new DerivedPredicate(
                new PredicateDeclaration("p", ImmutableArray<Parameter>.Empty),
                null!),
            () => new Metric(MetricDirection.Minimize, null!),
            () => new NumericInitialization(null!, number),
            () => new NumericInitialization(fluent, null!),
            () => new NegativePredicateInitialization(null!),
            () => new ProblemDefinition(
                "valid-problem",
                "valid-domain",
                ImmutableArray<PddlRequirement>.Empty,
                ImmutableArray<TypeDeclaration>.Empty,
                ImmutableArray<IProblemInitElement>.Empty,
                null!,
                null)
        ];

        foreach (System.Action construction in invalidConstructions)
            Assert.Throws<ArgumentNullException>(construction);
    }

    [Fact]
    public void CollectionsRejectNullElements()
    {
        Assert.Throws<ArgumentException>(() =>
            new And(ImmutableArray.CreateRange(new ILogicalExpression[] { null! })));
        Assert.Throws<ArgumentException>(() =>
            new PredicateDeclaration("p", ImmutableArray.CreateRange(new Parameter[] { null! })));
        Assert.Throws<ArgumentException>(() =>
            CreateDomain() with
            {
                Actions = ImmutableArray.CreateRange(new ActionDefinition[] { null! })
            });
    }

    [Fact]
    public void UndefinedEnumsAreRejectedAtConstruction()
    {
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            new Comparison((ComparisonOperator)999, new NumberLiteral(1m), new NumberLiteral(2m)));
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            new Metric((MetricDirection)999, new NumberLiteral(1m)));
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            CreateDomain() with
            {
                Requirements = ImmutableArray.Create((PddlRequirement)999)
            });
    }

    [Fact]
    public void WithAssignmentsEnforceTheSameInvariants()
    {
        var parameter = new Parameter("?x");
        var not = new Not(new EmptyLogic());
        var comparison = new Comparison(
            ComparisonOperator.Equal,
            new NumberLiteral(1m),
            new NumberLiteral(1m));
        DomainDefinition domain = CreateDomain();

        Assert.Throws<ArgumentException>(() => parameter with { Name = "x" });
        Assert.Throws<ArgumentNullException>(() => not with { Expression = null! });
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            comparison with { Operator = (ComparisonOperator)999 });
        Assert.Throws<ArgumentException>(() =>
            domain with { Predicates = default });
        Assert.Throws<ArgumentException>(() =>
            domain with { Name = "domain" });
    }

    [Fact]
    public void IntentionalOptionalMembersRemainNullable()
    {
        var action = new ActionDefinition("a", ImmutableArray<Parameter>.Empty, null, null);
        ProblemDefinition problem = CreateProblem();

        Assert.Null(action.Precondition);
        Assert.Null(action.Effect);
        Assert.Null(problem.Metric);
    }

    private static DomainDefinition CreateDomain(string name = "valid-domain")
    {
        return new DomainDefinition(
            name,
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray<PredicateDeclaration>.Empty,
            ImmutableArray<FunctionDeclaration>.Empty,
            ImmutableArray<ActionDefinition>.Empty,
            ImmutableArray<DerivedPredicate>.Empty);
    }

    private static ProblemDefinition CreateProblem(
        string name = "valid-problem",
        string domainName = "valid-domain")
    {
        return new ProblemDefinition(
            name,
            domainName,
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray<IProblemInitElement>.Empty,
            new EmptyLogic(),
            null);
    }
}
