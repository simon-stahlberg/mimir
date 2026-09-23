using System;
using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Analysis;

public class CanonicalizerTests
{
    [Fact]
    public void Canonicalizer_NormalizesOmittedActionClauses()
    {
        DomainDefinition parsed = PddlParser.ParseDomain(
            "(define (domain d) (:action wait :parameters ()))");
        ActionDefinition rawAction = Assert.Single(parsed.Actions);

        Assert.Null(rawAction.Precondition);
        Assert.Null(rawAction.Effect);
        Assert.False(rawAction.IsCanonical());

        DomainDefinition compiled = Canonicalizer.Compile(parsed);
        ActionDefinition compiledAction = Assert.Single(compiled.Actions);

        Assert.IsType<EmptyLogic>(compiledAction.Precondition);
        AndEffect effect = Assert.IsType<AndEffect>(compiledAction.Effect);
        Assert.Empty(effect.Effects);
        Assert.True(compiledAction.IsCanonical());
    }

    private ActionDefinition CreateTestAction()
    {
        // Precondition: (or (and A B) (exists (?x - obj) (imply C D)))
        var precond = new Or(ImmutableArray.Create<ILogicalExpression>(
            new And(ImmutableArray.Create<ILogicalExpression>(
                new PredicateCall("A", ImmutableArray<Term>.Empty),
                new PredicateCall("B", ImmutableArray<Term>.Empty)
            )),
            new Exists(
                ImmutableArray.Create(new Parameter("?x", "obj")),
                new Imply(
                    new PredicateCall("C", ImmutableArray.Create(Term.Variable("?x"))),
                    new PredicateCall("D", ImmutableArray.Create(Term.Variable("?x")))
                )
            )
        ));

        // Effect: (when X (and Y (when Z W)))
        var effect = new ConditionalEffect(
            new PredicateCall("X", ImmutableArray<Term>.Empty),
            new AndEffect(ImmutableArray.Create<IEffect>(
                new AddEffect(new PredicateCall("Y", ImmutableArray<Term>.Empty)),
                new ConditionalEffect(
                    new PredicateCall("Z", ImmutableArray<Term>.Empty),
                    new AddEffect(new PredicateCall("W", ImmutableArray<Term>.Empty))
                )
            ))
        );

        return new ActionDefinition("complex-action", ImmutableArray<Parameter>.Empty, precond, effect);
    }

    [Fact]
    public void Canonicalizer_EliminatesComplexStructures()
    {
        var domain = new DomainDefinition(
            "test",
            ImmutableArray<PddlRequirement>.Empty,
            ImmutableArray.Create(new TypeDeclaration("obj")),
            ImmutableArray<TypeDeclaration>.Empty,
            ImmutableArray.Create(new PredicateDeclaration("A", ImmutableArray<Parameter>.Empty),
                                  new PredicateDeclaration("B", ImmutableArray<Parameter>.Empty),
                                  new PredicateDeclaration("C", ImmutableArray.Create(new Parameter("?x", "obj"))),
                                  new PredicateDeclaration("D", ImmutableArray.Create(new Parameter("?x", "obj"))),
                                  new PredicateDeclaration("X", ImmutableArray<Parameter>.Empty),
                                  new PredicateDeclaration("Y", ImmutableArray<Parameter>.Empty),
                                  new PredicateDeclaration("Z", ImmutableArray<Parameter>.Empty),
                                  new PredicateDeclaration("W", ImmutableArray<Parameter>.Empty)),
            ImmutableArray<FunctionDeclaration>.Empty,
            ImmutableArray.Create(CreateTestAction()),
            ImmutableArray<DerivedPredicate>.Empty
        );

        var compiled = Canonicalizer.Compile(domain);
        var compiledAgain = Canonicalizer.Compile(domain);

        Assert.Equal(compiled.ToPddlString(), compiledAgain.ToPddlString());

        ActionDefinition action = Assert.Single(compiled.Actions);

        // 1. Assert Precondition contains no Or, Exists, Forall, Imply
        AssertNoComplexLogic(action.Precondition!);

        // 2. Assert Effect is a flat AndEffect of ConditionalEffects
        Assert.True(action.IsCanonical());

        // 3. Assert Derived Predicates were generated for Or and Exists
        Assert.True(compiled.DerivedPredicates.Length >= 1);

        // 4. Assert full canonical form
        Assert.True(compiled.IsCanonical());
    }

    private void AssertNoComplexLogic(ILogicalExpression expr)
    {
        Assert.IsNotType<Or>(expr);
        Assert.IsNotType<Exists>(expr);
        Assert.IsNotType<Forall>(expr);
        Assert.IsNotType<Imply>(expr);

        if (expr is And and)
        {
            foreach (var child in and.Expressions)
                AssertNoComplexLogic(child);
        }
        else if (expr is Not not)
        {
            AssertNoComplexLogic(not.Expression);
        }
    }

    [Fact]
    public void Canonicalizer_ExtractsCompoundNegations()
    {
        const string precondition = "(not (and (p) (q)))";
        DomainDefinition domain = PddlParser.ParseDomain($"""
(define (domain compound-negation)
  (:requirements :adl)
  (:predicates (p) (q) (done))
  (:action act
    :parameters ()
    :precondition {precondition}
    :effect (done)))
""");

        Assert.False(domain.Actions.Single().IsCanonical());

        DomainDefinition compiled = Canonicalizer.Compile(domain);
        ActionDefinition action = Assert.Single(compiled.Actions);
        PredicateCall axiomCall = Assert.IsType<PredicateCall>(action.Precondition);
        DerivedPredicate axiom = Assert.Single(compiled.DerivedPredicates);

        Assert.Equal(axiom.Signature.Name, axiomCall.Name, ignoreCase: true);
        Assert.Equal(PddlParser.ParseLogicalExpression(precondition), axiom.Body);
        Assert.True(compiled.IsCanonical());
    }

    [Theory]
    [InlineData("(and (and))")]
    [InlineData("(and (and) ())")]
    [InlineData("(or (and) (p))")]
    [InlineData("(not (or))")]
    [InlineData("(imply (or) (p))")]
    [InlineData("(imply (p) (and))")]
    [InlineData("(forall (?other - item) (and))")]
    [InlineData("(forall () (and))")]
    [InlineData("(exists () (and))")]
    [InlineData("(= ?item ?ITEM)")]
    [InlineData("(or (and) (< (score) 1))")]
    public void Canonicalizer_FoldsStructurallyTruePreconditions(string precondition)
    {
        DomainDefinition compiled = CompileDomainWithPrecondition(precondition);

        Assert.IsType<EmptyLogic>(Assert.Single(compiled.Actions).Precondition);
        Assert.Empty(compiled.DerivedPredicates);
        Assert.True(compiled.IsCanonical());
    }

    [Theory]
    [InlineData("(or)")]
    [InlineData("(not (and))")]
    [InlineData("(exists (?other - item) (or))")]
    [InlineData("(= first second)")]
    [InlineData("(and (p) (or) (< (score) 1))")]
    public void Canonicalizer_NormalizesStructurallyFalsePreconditions(string precondition)
    {
        DomainDefinition compiled = CompileDomainWithPrecondition(precondition);

        Assert.IsType<PredicateCall>(Assert.Single(compiled.Actions).Precondition);
        DerivedPredicate axiom = Assert.Single(compiled.DerivedPredicates);
        Or body = Assert.IsType<Or>(axiom.Body);
        Assert.Empty(body.Expressions);
        Assert.True(compiled.IsCanonical());
    }

    [Theory]
    [InlineData("(forall (?other - item) (or))")]
    [InlineData("(exists (?other - item) (and))")]
    public void Canonicalizer_DoesNotAssumeQuantifiedTypesArePopulated(string precondition)
    {
        DomainDefinition compiled = CompileDomainWithPrecondition(precondition);

        Assert.IsNotType<EmptyLogic>(Assert.Single(compiled.Actions).Precondition);
        Assert.NotEmpty(compiled.DerivedPredicates);
        Assert.True(compiled.IsCanonical());
    }

    [Fact]
    public void Canonicalizer_EliminatesImplicationsFromGeneratedAxiomBodies()
    {
        DomainDefinition compiled = CompileDomainWithPrecondition("(imply (p) (not (p)))");

        Assert.All(compiled.DerivedPredicates, derived => AssertContainsNoImplication(derived.Body));
    }

    [Fact]
    public void Canonicalizer_PreservesSimpleNegativePredicateLiteral()
    {
        DomainDefinition domain = PddlParser.ParseDomain("""
(define (domain simple-negation)
  (:requirements :negative-preconditions)
  (:predicates (p) (done))
  (:action act
    :parameters ()
    :precondition (not (p))
    :effect (done)))
""");

        DomainDefinition compiled = Canonicalizer.Compile(domain);
        ActionDefinition action = Assert.Single(compiled.Actions);
        Not precondition = Assert.IsType<Not>(action.Precondition);

        Assert.IsType<PredicateCall>(precondition.Expression);
        Assert.Empty(compiled.DerivedPredicates);
        Assert.True(compiled.IsCanonical());
    }

    [Fact]
    public void Canonicalizer_FoldsReflexiveEquality()
    {
        DomainDefinition domain = PddlParser.ParseDomain("""
(define (domain equality-precondition)
  (:requirements :equality)
  (:predicates (done))
  (:action act
    :parameters (?x)
    :precondition (= ?x ?X)
    :effect (done)))
""");

        Assert.False(domain.Actions.Single().IsCanonical());

        DomainDefinition compiled = Canonicalizer.Compile(domain);

        Assert.IsType<EmptyLogic>(compiled.Actions.Single().Precondition);
        Assert.Empty(compiled.DerivedPredicates);
        Assert.True(compiled.IsCanonical());
    }

    [Fact]
    public void Canonicalizer_PreservesEqualityBetweenDifferentVariables()
    {
        DomainDefinition domain = PddlParser.ParseDomain("""
(define (domain variable-equality)
  (:requirements :equality)
  (:predicates (done))
  (:action act
    :parameters (?x ?y)
    :precondition (= ?x ?y)
    :effect (done)))
""");

        DomainDefinition compiled = Canonicalizer.Compile(domain);

        Assert.IsType<Equality>(compiled.Actions.Single().Precondition);
        Assert.Empty(compiled.DerivedPredicates);
        Assert.True(compiled.IsCanonical());
    }

    [Fact]
    public void Canonicalizer_GeneratedAxiomsDoNotReuseExistingSymbols()
    {
        DomainDefinition domain = PddlParser.ParseDomain("""
(define (domain generated-axiom-name)
  (:requirements :disjunctive-preconditions :numeric-fluents)
  (:predicates (left) (right))
  (:functions (score))
  (:action choose
    :parameters ()
    :precondition (or (left) (right))
    :effect (left)))
""");

        DomainDefinition compiled = Canonicalizer.Compile(domain);
        DerivedPredicate generated = Assert.Single(compiled.DerivedPredicates);
        string[] existingSymbols =
        [
            .. domain.Predicates.Select(predicate => predicate.Name),
            .. domain.Functions.Select(function => function.Name),
            .. domain.DerivedPredicates.Select(derived => derived.Signature.Name)
        ];

        Assert.DoesNotContain(existingSymbols,
            name => name.Equals(generated.Signature.Name, StringComparison.OrdinalIgnoreCase));
        PredicateCall precondition = Assert.IsType<PredicateCall>(Assert.Single(compiled.Actions).Precondition);
        Assert.Equal(generated.Signature.Name, precondition.Name, ignoreCase: true);
    }

    [Fact]
    public void Canonicalizer_RejectsGeneratedAxiomSymbolCollision()
    {
        DomainDefinition domain = PddlParser.ParseDomain("""
(define (domain generated-axiom-collision)
  (:requirements :disjunctive-preconditions)
  (:predicates (left) (right) (axiom_0))
  (:action choose
    :parameters ()
    :precondition (or (left) (right))
    :effect (left)))
""");

        Assert.Throws<PddlValidationException>(() => Canonicalizer.Compile(domain));
    }

    [Fact]
    public void Canonicalizer_PreservesConditionalTotalCostForCoreValidation()
    {
        DomainDefinition domain = PddlParser.ParseDomain(@"
(define (domain conditional-total-cost)
  (:requirements :action-costs :conditional-effects)
  (:predicates (guard))
  (:functions (total-cost))
  (:action charge
    :parameters ()
    :precondition (guard)
    :effect (when (guard) (increase (total-cost) 1)))
)");

        DomainDefinition compiled = Canonicalizer.Compile(domain);

        AndEffect effect = Assert.IsType<AndEffect>(Assert.Single(compiled.Actions).Effect);
        ConditionalEffect conditionalEffect = Assert.IsType<ConditionalEffect>(Assert.Single(effect.Effects));
        Assert.Equal("guard", Assert.IsType<PredicateCall>(conditionalEffect.Condition).Name);
        Increase increase = Assert.IsType<Increase>(conditionalEffect.Effect);
        Assert.Equal("total-cost", increase.Fluent.Name);
        Assert.Equal(1m, Assert.IsType<NumberLiteral>(increase.Value).Value);
        Assert.True(compiled.IsCanonical());
    }

    private static DomainDefinition CompileDomainWithPrecondition(string precondition)
    {
        DomainDefinition domain = PddlParser.ParseDomain($$"""
(define (domain constant-folding)
  (:requirements :adl :equality :numeric-fluents)
  (:types item)
  (:constants first second - item)
  (:predicates (p) (done))
  (:functions (score))
  (:action act
    :parameters (?item - item)
    :precondition {{precondition}}
    :effect (done)))
""");

        return Canonicalizer.Compile(domain);
    }

    private static void AssertContainsNoImplication(ILogicalExpression expression)
    {
        Assert.IsNotType<Imply>(expression);
        switch (expression)
        {
            case And and:
                Assert.All(and.Expressions, AssertContainsNoImplication);
                break;
            case Or or:
                Assert.All(or.Expressions, AssertContainsNoImplication);
                break;
            case Not not:
                AssertContainsNoImplication(not.Expression);
                break;
            case Forall forall:
                AssertContainsNoImplication(forall.Body);
                break;
            case Exists exists:
                AssertContainsNoImplication(exists.Body);
                break;
        }
    }
}
