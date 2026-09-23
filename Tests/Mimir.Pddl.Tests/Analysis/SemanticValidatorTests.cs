using System;
using System.Collections.Immutable;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Analysis;

public class SemanticValidatorTests
{
    private DomainDefinition CreateValidDomain()
    {
        return new DomainDefinition(
            "test-domain",
            ImmutableArray.Create(PddlRequirement.Typing),
            ImmutableArray.Create(new TypeDeclaration("vehicle")),
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray.Create(new PredicateDeclaration("at", ImmutableArray.Create(new Parameter("?v", "vehicle"), new Parameter("?l", "object")))),
            ImmutableArray<FunctionDeclaration>.Empty,
            ImmutableArray.Create(
                new ActionDefinition(
                    "move",
                    ImmutableArray.Create(new Parameter("?v", "vehicle"), new Parameter("?from", "object"), new Parameter("?to", "object")),
                    new PredicateCall("at", ImmutableArray.Create(Term.Variable("?v"), Term.Variable("?from"))),
                    new AndEffect(ImmutableArray.Create<IEffect>(
                        new DeleteEffect(new PredicateCall("at", ImmutableArray.Create(Term.Variable("?v"), Term.Variable("?from")))),
                        new AddEffect(new PredicateCall("at", ImmutableArray.Create(Term.Variable("?v"), Term.Variable("?to"))))
                    ))
                )
            ),
            ImmutableArray<DerivedPredicate>.Empty
        );
    }

    private static DomainDefinition ParseDomain(string input)
    {
        return PddlParser.ParseDomain(input);
    }

    private static ProblemDefinition ParseProblem(string input)
    {
        return PddlParser.ParseProblem(input);
    }

    [Fact]
    public void Validator_ValidDomain_ThrowsNoException()
    {
        var domain = CreateValidDomain();
        SemanticValidator.ValidateDomain(domain);
    }

    [Fact]
    public void Validator_UndeclaredPredicate_ThrowsValidationException()
    {
        var domain = CreateValidDomain() with
        {
            Actions = ImmutableArray.Create(
                new ActionDefinition(
                    "bad-action",
                    ImmutableArray.Create(new Parameter("?v", "vehicle")),
                    new PredicateCall("fake-predicate", ImmutableArray.Create(Term.Variable("?v"))),
                    null
                )
            )
        };

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
        Assert.Contains("Undeclared predicate 'fake-predicate'", ex.Message);
    }

    [Fact]
    public void Validator_ArityMismatch_ThrowsValidationException()
    {
        var domain = CreateValidDomain() with
        {
            Actions = ImmutableArray.Create(
                new ActionDefinition(
                    "bad-action",
                    ImmutableArray.Create(new Parameter("?v", "vehicle")),
                    new PredicateCall("at", ImmutableArray.Create(Term.Variable("?v"))), // Missing second parameter
                    null
                )
            )
        };

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
        Assert.Contains("Arity mismatch for predicate 'at'", ex.Message);
    }

    [Fact]
    public void Validator_TypeMismatch_ThrowsValidationException()
    {
        var domain = CreateValidDomain() with
        {
            Actions = ImmutableArray.Create(
                new ActionDefinition(
                    "bad-action",
                    ImmutableArray.Create(new Parameter("?v", "object")), // Parameter is 'object', but predicate expects 'vehicle'
                    new PredicateCall("at", ImmutableArray.Create(Term.Variable("?v"), Term.Variable("?l"))),
                    null
                )
            )
        };

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
        Assert.Contains("Type mismatch", ex.Message);
    }

    [Fact]
    public void Validator_UndeclaredVariable_ThrowsValidationException()
    {
        var domain = CreateValidDomain() with
        {
            Actions = ImmutableArray.Create(
                new ActionDefinition(
                    "bad-action",
                    ImmutableArray.Create(new Parameter("?v", "vehicle")),
                    new PredicateCall("at", ImmutableArray.Create(Term.Variable("?v"), Term.Variable("?undeclared"))),
                    null
                )
            )
        };

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
        Assert.Contains("Undeclared variable '?undeclared'", ex.Message);
    }

    [Fact]
    public void Validator_ProblemDomainNameMismatch_ThrowsValidationException()
    {
        var domain = ParseDomain(@"
(define (domain shipping)
  (:requirements :strips)
  (:predicates (ready))
)");

        var problem = ParseProblem(@"
(define (problem p1)
  (:domain typo-shipping)
  (:init (ready))
  (:goal (ready))
)");

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateProblem(domain, problem));
        Assert.Contains("references domain 'typo-shipping'", ex.Message);
    }

    [Fact]
    public void Validator_MetricExpression_ValidatesNestedFluentCalls()
    {
        var domain = ParseDomain(@"
(define (domain metric-validation)
  (:requirements :typing :numeric-fluents)
  (:types truck)
  (:predicates (ready))
  (:functions (drive-cost ?t - truck))
)");

        var problem = ParseProblem(@"
(define (problem p1)
  (:domain metric-validation)
  (:objects t1 - truck)
  (:init (ready) (= (drive-cost t1) 2))
  (:goal (ready))
  (:metric minimize (+ (drive-cost t1) (missing-cost)))
)");

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateProblem(domain, problem));
        Assert.Contains("Undeclared numeric fluent 'missing-cost'", ex.Message);
        Assert.Contains("problem metric", ex.Message);
    }

    [Fact]
    public void Validator_TotalCostMetricValidatesBuiltInArity()
    {
        var domain = ParseDomain(@"
(define (domain total-cost-validation)
  (:requirements :strips :action-costs)
  (:predicates (ready))
)");

        var maximizeProblem = ParseProblem(@"
(define (problem p1)
  (:domain total-cost-validation)
  (:init (ready))
  (:goal (ready))
  (:metric maximize (total-cost))
)");

        SemanticValidator.ValidateProblem(domain, maximizeProblem);

        var arityProblem = ParseProblem(@"
(define (problem p2)
  (:domain total-cost-validation)
  (:objects extra)
  (:init (ready))
  (:goal (ready))
  (:metric minimize (total-cost extra))
)");

        var arityEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateProblem(domain, arityProblem));
        Assert.Contains("Arity mismatch for fluent 'total-cost'", arityEx.Message);
    }

    [Fact]
    public void Validator_TotalCostUsesBuiltInArityWhenExplicitlyDeclared()
    {
        var domain = ParseDomain(@"
(define (domain total-cost-arity)
  (:requirements :action-costs)
  (:predicates (ready))
  (:functions (total-cost ?item))
  (:action charge
    :parameters (?item)
    :precondition (ready)
    :effect (increase (total-cost ?item) 1))
)");

        var exception = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Arity mismatch for fluent 'total-cost'", exception.Message);
    }

    [Fact]
    public void Validator_DerivedPredicateBody_ValidatesPredicateCalls()
    {
        var domain = ParseDomain(@"
(define (domain derived-body-validation)
  (:requirements :typing :derived-predicates)
  (:types loc)
  (:predicates (at ?x - loc))
  (:derived (safe ?x - loc) (and (at ?x) (missing ?x)))
)");

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
        Assert.Contains("Undeclared predicate 'missing'", ex.Message);
        Assert.Contains("derived predicate safe", ex.Message);
    }

    [Fact]
    public void Validator_DerivedPredicateSignature_IsVisibleToActionsAndGoals()
    {
        var domain = ParseDomain(@"
(define (domain derived-signature)
  (:requirements :typing :derived-predicates)
  (:types loc)
  (:predicates (at ?x - loc) (done ?x - loc))
  (:derived (safe ?x - loc) (at ?x))
  (:action finish
    :parameters (?x - loc)
    :precondition (safe ?x)
    :effect (done ?x))
)");

        var problem = ParseProblem(@"
(define (problem p1)
  (:domain derived-signature)
  (:objects l1 - loc)
  (:init (at l1))
  (:goal (safe l1))
)");

        SemanticValidator.ValidateProblem(domain, problem);
    }

    [Fact]
    public void Validator_RepeatedDerivedSignaturesMayRenameParameters()
    {
        var domain = ParseDomain(@"
(define (domain renamed-derived-heads)
  (:requirements :typing :derived-predicates)
  (:types item)
  (:predicates (base ?value - item) (reachable ?value - item))
  (:derived (reachable ?first - item) (base ?first))
  (:derived (REACHABLE ?second - ITEM) (base ?second))
)");

        SemanticValidator.ValidateDomain(domain);
    }

    [Theory]
    [InlineData("(reachable ?left - item ?right - item)", "Arity mismatch")]
    [InlineData("(reachable ?value - other)", "Type mismatch")]
    public void Validator_RepeatedDerivedSignaturesMustMatchPositionally(
        string secondSignature,
        string expectedMessage)
    {
        var domain = ParseDomain($@"
(define (domain mismatched-derived-heads)
  (:requirements :typing :derived-predicates)
  (:types item other)
  (:predicates (base ?value - item))
  (:derived (reachable ?value - item) (base ?value))
  (:derived {secondSignature} (and))
)");

        var exception = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));

        Assert.Contains(expectedMessage, exception.Message);
    }

    [Fact]
    public void Validator_DerivedSignatureMustMatchDeclaredPredicate()
    {
        var domain = ParseDomain(@"
(define (domain declared-derived-signature)
  (:requirements :typing :derived-predicates)
  (:types item other)
  (:predicates (base ?value - other) (reachable ?value - item))
  (:derived (reachable ?value - other) (base ?value))
)");

        var exception = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Type mismatch", exception.Message);
    }

    [Fact]
    public void Validator_DerivedPredicates_CannotBeMutatedOrInitialized()
    {
        var effectDomain = ParseDomain(@"
(define (domain derived-effect)
  (:requirements :typing :derived-predicates)
  (:types loc)
  (:predicates (at ?x - loc))
  (:derived (safe ?x - loc) (at ?x))
  (:action make-safe
    :parameters (?x - loc)
    :effect (safe ?x))
)");

        var effectEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(effectDomain));
        Assert.Contains("Derived predicate 'safe' cannot be modified", effectEx.Message);

        var initDomain = ParseDomain(@"
(define (domain derived-init)
  (:requirements :typing :derived-predicates)
  (:types loc)
  (:predicates (at ?x - loc))
  (:derived (safe ?x - loc) (at ?x))
)");

        var problem = ParseProblem(@"
(define (problem p1)
  (:domain derived-init)
  (:objects l1 - loc)
  (:init (safe l1))
  (:goal (safe l1))
)");

        var initEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateProblem(initDomain, problem));
        Assert.Contains("Derived predicate 'safe' cannot appear in problem init", initEx.Message);
    }

    [Fact]
    public void Validator_PredicateFunctionNamespaceCollision_ThrowsValidationException()
    {
        var domain = ParseDomain(@"
(define (domain namespace-collision)
  (:requirements :numeric-fluents)
  (:predicates (load))
  (:functions (load))
)");

        var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
        Assert.Contains("declared as both predicate and numeric fluent", ex.Message);
    }

    [Fact]
    public void Validator_UnknownDeclaredTypes_ThrowValidationException()
    {
        var predicateDomain = ParseDomain(@"
(define (domain bad-predicate-type)
  (:requirements :typing)
  (:predicates (at ?x - ghost-type))
)");

        var predicateEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(predicateDomain));
        Assert.Contains("Undeclared type 'ghost-type'", predicateEx.Message);

        var actionDomain = ParseDomain(@"
(define (domain bad-action-type)
  (:requirements :typing)
  (:predicates (ready))
  (:action act
    :parameters (?x - ghost-type)
    :precondition (ready)
    :effect (ready))
)");

        var actionEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(actionDomain));
        Assert.Contains("Undeclared type 'ghost-type'", actionEx.Message);

        var constantDomain = ParseDomain(@"
(define (domain bad-constant-type)
  (:requirements :typing)
  (:constants c1 - ghost-type)
  (:predicates (ready))
)");

        var constantEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(constantDomain));
        Assert.Contains("Undeclared type 'ghost-type'", constantEx.Message);

        var objectDomain = ParseDomain(@"
(define (domain bad-object-type)
  (:requirements :typing)
  (:predicates (ready))
)");

        var objectProblem = ParseProblem(@"
(define (problem p1)
  (:domain bad-object-type)
  (:objects o1 - ghost-type)
  (:init (ready))
  (:goal (ready))
)");

        var objectEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateProblem(objectDomain, objectProblem));
        Assert.Contains("Undeclared type 'ghost-type'", objectEx.Message);
    }

    [Theory]
    [InlineData("object")]
    [InlineData("OBJECT - item item")]
    public void Validator_BuiltInObjectTypeCannotBeDeclared(string typeList)
    {
        var domain = ParseDomain($@"
(define (domain bad-object-root)
  (:requirements :typing)
  (:types {typeList})
  (:predicates (ready))
)");

        var exception = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("built-in type 'object'", exception.Message);
    }

    [Fact]
    public void Validator_TypeMayInheritFromBuiltInObject()
    {
        var domain = ParseDomain(@"
(define (domain object-parent)
  (:requirements :typing)
  (:types item - object)
  (:predicates (known ?value - item))
)");

        SemanticValidator.ValidateDomain(domain);
    }

    [Fact]
    public void Validator_UntypedDomainRejectsUndeclaredConstant()
    {
        var domain = ParseDomain(@"
(define (domain untyped-domain-constant)
  (:predicates (known ?value))
  (:action check
    :parameters ()
    :precondition (known missing)
    :effect (known missing))
)");

        var exception = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Undeclared constant or object 'missing'", exception.Message);
    }

    [Fact]
    public void Validator_UntypedProblemRejectsUndeclaredObject()
    {
        var domain = ParseDomain(@"
(define (domain untyped-problem-object)
  (:predicates (known ?value))
)");
        var problem = ParseProblem(@"
(define (problem untyped-problem)
  (:domain untyped-problem-object)
  (:objects declared)
  (:init (known missing))
  (:goal (known declared))
)");

        var exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateProblem(domain, problem));

        Assert.Contains("Undeclared constant or object 'missing'", exception.Message);
    }

    [Fact]
    public void Validator_UntypedProblemGoalRejectsUndeclaredObject()
    {
        var domain = ParseDomain(@"
(define (domain untyped-problem-goal)
  (:predicates (known ?value))
)");
        var problem = ParseProblem(@"
(define (problem untyped-problem)
  (:domain untyped-problem-goal)
  (:objects declared)
  (:init (known declared))
  (:goal (known missing))
)");

        var exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateProblem(domain, problem));

        Assert.Contains("Undeclared constant or object 'missing'", exception.Message);
    }

    [Fact]
    public void Validator_UntypedEqualityRejectsUndeclaredConstant()
    {
        var domain = new DomainDefinition(
            "untyped-equality",
            ImmutableArray.Create(PddlRequirement.Equality),
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray.Create(new TypeDeclaration("declared")),
            ImmutableArray<PredicateDeclaration>.Empty,
            ImmutableArray<FunctionDeclaration>.Empty,
            ImmutableArray.Create(new ActionDefinition(
                "compare",
                ImmutableArray<Parameter>.Empty,
                new Equality(Term.Constant("declared"), Term.Constant("missing")),
                null)),
            ImmutableArray<DerivedPredicate>.Empty);

        var exception = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Undeclared constant or object 'missing'", exception.Message);
    }

    [Fact]
    public void Validator_QuantifierVariables_CannotShadowOrDuplicateNames()
    {
        var shadowDomain = ParseDomain(@"
(define (domain quantifier-shadow)
  (:requirements :typing :existential-preconditions)
  (:types loc)
  (:predicates (at ?x - loc))
  (:action act
    :parameters (?x - loc)
    :precondition (exists (?x - loc) (at ?x))
    :effect (at ?x))
)");

        var shadowEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(shadowDomain));
        Assert.Contains("shadows an existing variable", shadowEx.Message);

        var duplicateDomain = ParseDomain(@"
(define (domain quantifier-duplicate)
  (:requirements :typing :existential-preconditions)
  (:types loc)
  (:predicates (at ?x - loc))
  (:action act
    :parameters ()
    :precondition (exists (?x - loc ?x - loc) (at ?x))
    :effect (at ?x))
)");

        var duplicateEx = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(duplicateDomain));
        Assert.Contains("Duplicate quantified variable '?x'", duplicateEx.Message);
    }

    [Fact]
    public void Validator_ActionNamesAreUniqueIgnoringCase()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain duplicate-actions)
  (:requirements :strips)
  (:predicates (ready))
  (:action move
    :parameters ()
    :effect (ready))
  (:action MOVE
    :parameters ()
    :effect (ready)))
""");

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Duplicate action declaration 'MOVE'", exception.Message);
    }

    [Theory]
    [InlineData("(:predicates (connected ?item ?ITEM))", "predicate 'connected'")]
    [InlineData("(:functions (distance ?item ?ITEM))", "numeric fluent 'distance'")]
    public void Validator_DeclarationParameterNamesAreUniqueIgnoringCase(
        string declaration,
        string expectedContext)
    {
        DomainDefinition domain = ParseDomain($"""
(define (domain duplicate-parameters)
  (:requirements :numeric-fluents)
  {declaration})
""");

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateDomain(domain));

        Assert.Contains("Duplicate parameter '?ITEM'", exception.Message);
        Assert.Contains(expectedContext, exception.Message);
    }

    [Fact]
    public void Validator_ProblemObjectsCannotReuseDomainConstantNamesIgnoringCase()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain object-constant-collision)
  (:requirements :strips :typing)
  (:types vehicle location)
  (:constants shared - vehicle)
  (:predicates (ready)))
""");
        ProblemDefinition problem = ParseProblem("""
(define (problem collision)
  (:domain object-constant-collision)
  (:objects SHARED - location)
  (:init (ready))
  (:goal (ready)))
""");

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateProblem(domain, problem));

        Assert.Contains("Problem object 'SHARED' conflicts with a domain constant", exception.Message);
    }

    [Fact]
    public void Validator_GroundedNumericFluentsCanOnlyBeInitializedOnceIgnoringCase()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain duplicate-numeric-init)
  (:requirements :strips :typing :numeric-fluents)
  (:types item)
  (:predicates (ready))
  (:functions (amount ?item - item)))
""");
        ProblemDefinition problem = ParseProblem("""
(define (problem duplicate-init)
  (:domain duplicate-numeric-init)
  (:objects first second - item)
  (:init
    (ready)
    (= (amount first) 1)
    (= (amount second) 2)
    (= (AMOUNT FIRST) 3))
  (:goal (ready)))
""");

        PddlValidationException exception = Assert.Throws<PddlValidationException>(
            () => SemanticValidator.ValidateProblem(domain, problem));

        Assert.Contains("Numeric fluent '(AMOUNT FIRST)' is initialized more than once", exception.Message);
    }

    [Fact]
    public void Validator_LeavesDuplicateTotalCostInitializationForCoreValidation()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain duplicate-total-cost)
  (:requirements :strips :action-costs)
  (:predicates (ready)))
""");
        ProblemDefinition problem = ParseProblem("""
(define (problem duplicate-cost)
  (:domain duplicate-total-cost)
  (:init
    (ready)
    (= (total-cost) 0)
    (= (TOTAL-COST) 0))
  (:goal (ready)))
""");

        ProblemDefinition validated = SemanticValidator.ValidateProblem(domain, problem);

        NumericInitialization[] totalCostInitializations = validated.Init
            .OfType<NumericInitialization>()
            .Where(initialization => initialization.Fluent.Name.Equals(
                "total-cost",
                StringComparison.OrdinalIgnoreCase))
            .ToArray();

        Assert.Equal(2, totalCostInitializations.Length);
    }
}
