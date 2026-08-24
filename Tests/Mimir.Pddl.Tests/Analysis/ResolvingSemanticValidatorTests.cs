using System.Collections.Immutable;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Xunit;

namespace Mimir.Pddl.Tests.Analysis;

public class ResolvingSemanticValidatorTests
{
    [Fact]
    public void ValidateDomain_ResolvesZeroArityFunctionEqualityAsNumericComparison()
    {
        DomainDefinition domain = CreateDomain(
            functions: ImmutableArray.Create(
                new FunctionDeclaration("fuel", ImmutableArray<Parameter>.Empty),
                new FunctionDeclaration("capacity", ImmutableArray<Parameter>.Empty)),
            actions: ImmutableArray.Create(
                new ActionDefinition(
                    "compare",
                    ImmutableArray<Parameter>.Empty,
                    new AmbiguousEquality("fuel", "capacity"),
                    null)));

        DomainDefinition validated = SemanticValidator.ValidateDomain(domain);

        Comparison comparison = Assert.IsType<Comparison>(validated.Actions[0].Precondition);
        Assert.Equal(ComparisonOperator.Equal, comparison.Operator);
        Assert.Equal("fuel", Assert.IsType<FluentCall>(comparison.Left).Name);
        Assert.Equal("capacity", Assert.IsType<FluentCall>(comparison.Right).Name);
    }

    [Fact]
    public void ValidateProblem_ResolvesDeclaredObjectEqualityAsTermEquality()
    {
        DomainDefinition domain = CreateDomain(requirements: ImmutableArray.Create(PddlRequirement.Typing));
        var problem = new ProblemDefinition(
            "p",
            domain.Name,
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray.Create(new TypeDeclaration("left"), new TypeDeclaration("right")),
            ImmutableArray<IProblemInitElement>.Empty,
            new AmbiguousEquality("left", "right"),
            null);

        ProblemDefinition validated = SemanticValidator.ValidateProblem(domain, problem);

        Equality equality = Assert.IsType<Equality>(validated.Goal);
        Assert.Equal(Term.Constant("left"), equality.Left);
        Assert.Equal(Term.Constant("right"), equality.Right);
    }

    [Theory]
    [InlineData("mixed")]
    [InlineData("unresolved")]
    [InlineData("multiply-defined")]
    public void ValidateDomain_RejectsEqualityThatCannotBeResolvedUniquely(string scenario)
    {
        ImmutableArray<TypeDeclaration> constants = scenario switch
        {
            "mixed" => ImmutableArray.Create(new TypeDeclaration("item")),
            "multiply-defined" => ImmutableArray.Create(new TypeDeclaration("value")),
            _ => ImmutableArray<TypeDeclaration>.Empty
        };
        ImmutableArray<FunctionDeclaration> functions = scenario switch
        {
            "mixed" => ImmutableArray.Create(new FunctionDeclaration("value", ImmutableArray<Parameter>.Empty)),
            "multiply-defined" => ImmutableArray.Create(
                new FunctionDeclaration("value", ImmutableArray<Parameter>.Empty),
                new FunctionDeclaration("other", ImmutableArray<Parameter>.Empty)),
            _ => ImmutableArray<FunctionDeclaration>.Empty
        };
        AmbiguousEquality equality = scenario switch
        {
            "mixed" => new AmbiguousEquality("value", "item"),
            "multiply-defined" => new AmbiguousEquality("value", "other"),
            _ => new AmbiguousEquality("unknown", "missing")
        };
        DomainDefinition domain = CreateDomain(
            constants: constants,
            functions: functions,
            actions: ImmutableArray.Create(
                new ActionDefinition("compare", ImmutableArray<Parameter>.Empty, equality, null)));

        Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
    }

    [Fact]
    public void ValidateDomain_RejectsSelfReferentialTypeCycle()
    {
        DomainDefinition domain = CreateDomain(
            types: ImmutableArray.Create(new TypeDeclaration("vehicle", "VEHICLE")));

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("cycle", exception.Message);
    }

    [Fact]
    public void ValidateDomain_RejectsMultiNodeTypeCycleCaseInsensitively()
    {
        DomainDefinition domain = CreateDomain(
            types: ImmutableArray.Create(
                new TypeDeclaration("vehicle", "location"),
                new TypeDeclaration("LOCATION", "Vehicle")));

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("cycle", exception.Message);
    }

    [Fact]
    public void ValidateProblem_AcceptsAndPreservesNegativeInitialFact()
    {
        PredicateCall ready = new("ready", ImmutableArray<Term>.Empty);
        DomainDefinition domain = CreateDomain(
            predicates: ImmutableArray.Create(new PredicateDeclaration("ready", ImmutableArray<Parameter>.Empty)));
        var problem = new ProblemDefinition(
            "p",
            domain.Name,
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray.Create<IProblemInitElement>(new NegativePredicateInitialization(ready)),
            ready,
            null);

        ProblemDefinition validated = SemanticValidator.ValidateProblem(domain, problem);

        Assert.IsType<NegativePredicateInitialization>(Assert.Single(validated.Init));
    }

    [Fact]
    public void ValidateProblem_RejectsContradictoryInitialFactsCaseInsensitively()
    {
        DomainDefinition domain = CreateDomain(
            predicates: ImmutableArray.Create(new PredicateDeclaration("ready", ImmutableArray<Parameter>.Empty)));
        var problem = new ProblemDefinition(
            "p",
            domain.Name,
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray.Create<IProblemInitElement>(
                new PredicateCall("Ready", ImmutableArray<Term>.Empty),
                new NegativePredicateInitialization(new PredicateCall("ready", ImmutableArray<Term>.Empty))),
            new EmptyLogic(),
            null);

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateProblem(domain, problem));

        Assert.Contains("contradictory", exception.Message);
    }

    [Fact]
    public void ValidateProblem_RejectsNegativeDerivedPredicateInitialFact()
    {
        PredicateDeclaration safe = new("safe", ImmutableArray<Parameter>.Empty);
        DomainDefinition domain = CreateDomain(
            predicates: ImmutableArray.Create(new PredicateDeclaration("base", ImmutableArray<Parameter>.Empty)),
            derivedPredicates: ImmutableArray.Create(
                new DerivedPredicate(safe, new PredicateCall("base", ImmutableArray<Term>.Empty))));
        var problem = new ProblemDefinition(
            "p",
            domain.Name,
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray.Create<IProblemInitElement>(
                new NegativePredicateInitialization(new PredicateCall("safe", ImmutableArray<Term>.Empty))),
            new EmptyLogic(),
            null);

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateProblem(domain, problem));

        Assert.Contains("Derived predicate 'safe'", exception.Message);
    }

    [Fact]
    public void AdlRequirement_EnablesTypingValidation()
    {
        DomainDefinition domain = CreateDomain(
            requirements: ImmutableArray.Create(PddlRequirement.Adl),
            predicates: ImmutableArray.Create(
                new PredicateDeclaration("known", ImmutableArray.Create(new Parameter("?x")))),
            actions: ImmutableArray.Create(
                new ActionDefinition(
                    "check",
                    ImmutableArray<Parameter>.Empty,
                    new PredicateCall("known", ImmutableArray.Create(Term.Constant("missing"))),
                    null)));

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Undeclared constant or object 'missing'", exception.Message);
    }

    [Fact]
    public void Canonicalizer_DeduplicatesFreeVariablesCaseInsensitively()
    {
        PredicateDeclaration predicate = new(
            "known",
            ImmutableArray.Create(new Parameter("?value")));
        var precondition = new Or(ImmutableArray.Create<ILogicalExpression>(
            new PredicateCall("known", ImmutableArray.Create(Term.Variable("?item"))),
            new PredicateCall("known", ImmutableArray.Create(Term.Variable("?ITEM")))));
        DomainDefinition domain = CreateDomain(
            requirements: ImmutableArray.Create(PddlRequirement.Adl),
            predicates: ImmutableArray.Create(predicate),
            actions: ImmutableArray.Create(
                new ActionDefinition(
                    "check",
                    ImmutableArray.Create(new Parameter("?Item")),
                    precondition,
                    null)));

        DomainDefinition compiled = Canonicalizer.Compile(domain);

        DerivedPredicate derivedPredicate = Assert.Single(compiled.DerivedPredicates);
        Parameter parameter = Assert.Single(derivedPredicate.Signature.Parameters);
        Assert.Equal(
            "?item",
            parameter.Name,
            ignoreCase: true);
    }

    private static DomainDefinition CreateDomain(
        ImmutableArray<PddlRequirement> requirements = default,
        ImmutableArray<TypeDeclaration> types = default,
        ImmutableArray<TypeDeclaration> constants = default,
        ImmutableArray<PredicateDeclaration> predicates = default,
        ImmutableArray<FunctionDeclaration> functions = default,
        ImmutableArray<ActionDefinition> actions = default,
        ImmutableArray<DerivedPredicate> derivedPredicates = default)
    {
        return new DomainDefinition(
            "test-domain",
            requirements.IsDefault ? ImmutableArray<PddlRequirement>.Empty : requirements,
            types.IsDefault ? ImmutableArray<TypeDeclaration>.Empty : types,
            constants.IsDefault ? ImmutableArray<TypeDeclaration>.Empty : constants,
            predicates.IsDefault ? ImmutableArray<PredicateDeclaration>.Empty : predicates,
            functions.IsDefault ? ImmutableArray<FunctionDeclaration>.Empty : functions,
            actions.IsDefault ? ImmutableArray<ActionDefinition>.Empty : actions,
            derivedPredicates.IsDefault ? ImmutableArray<DerivedPredicate>.Empty : derivedPredicates);
    }
}
