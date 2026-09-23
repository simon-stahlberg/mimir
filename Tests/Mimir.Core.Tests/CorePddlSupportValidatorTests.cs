using System.Collections.Immutable;
using Mimir.Core.Schemas;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Core.Tests;

public sealed class CorePddlSupportValidatorTests
{
    [Theory]
    [InlineData("assign")]
    [InlineData("increase")]
    [InlineData("decrease")]
    [InlineData("scale-up")]
    [InlineData("scale-down")]
    public void ValidateDomain_AllowsOrdinaryNumericMutations(string operation)
    {
        DomainDefinition domain = ParseDomain($$"""
(define (domain numeric-mutation)
  (:requirements :strips :numeric-fluents)
  (:functions (fuel))
  (:action mutate
    :parameters ()
    :precondition ()
    :effect ({{operation}} (fuel) 1)))
""");

        CorePddlSupportValidator.ValidateDomain(domain);
    }

    [Theory]
    [InlineData("assign", "conditional")]
    [InlineData("increase", "conditional")]
    [InlineData("decrease", "conditional")]
    [InlineData("scale-up", "conditional")]
    [InlineData("scale-down", "conditional")]
    [InlineData("assign", "quantified")]
    [InlineData("increase", "quantified")]
    [InlineData("decrease", "quantified")]
    [InlineData("scale-up", "quantified")]
    [InlineData("scale-down", "quantified")]
    public void ValidateDomain_AllowsNestedOrdinaryNumericMutations(
        string operation,
        string nesting)
    {
        string mutation = $"({operation} (fuel) 1)";
        string effect = nesting == "conditional"
            ? $"(when (enabled) {mutation})"
            : $"(forall (?item - object) {mutation})";
        DomainDefinition domain = ParseDomain($$"""
(define (domain nested-numeric-mutation)
  (:requirements :adl :numeric-fluents)
  (:predicates (enabled))
  (:functions (fuel))
  (:action mutate
    :parameters ()
    :precondition (and)
    :effect {{effect}}))
""");

        CorePddlSupportValidator.ValidateDomain(domain);
    }

    [Fact]
    public void ValidateDomain_AllowsStaticNumericFunctionsInActionCosts()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain static-cost)
  (:requirements :strips :action-costs)
  (:functions (distance))
  (:action move
    :parameters ()
    :precondition ()
    :effect (increase (total-cost) (+ 1 (distance)))))
""");

        CorePddlSupportValidator.ValidateDomain(domain);
    }

    [Fact]
    public void ValidateDomain_RejectsCostsReadingFluentsChangedByAnyConditionalEffect()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain changing-cost)
  (:requirements :strips :action-costs :conditional-effects)
  (:predicates (flag))
  (:functions (battery))
  (:action charge
    :parameters ()
    :precondition ()
    :effect (when (flag) (increase (BATTERY) 1)))
  (:action move
    :parameters ()
    :precondition ()
    :effect (increase (total-cost) (* 2 (battery)))))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateDomain(domain));

        Assert.Contains("Action 'move'", exception.Message);
        Assert.Contains("'battery'", exception.Message);
    }

    [Fact]
    public void ValidateDomain_RejectsTotalCostIncreaseWithoutActionCostsRequirement()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain missing-action-costs)
  (:requirements :strips :numeric-fluents)
  (:functions (total-cost))
  (:action move
    :parameters ()
    :precondition ()
    :effect (increase (total-cost) 1)))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateDomain(domain));

        Assert.Contains(":action-costs", exception.Message);
    }

    [Fact]
    public void ValidateDomain_RejectsActionCostWithArguments()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain parameterized-total-cost)
  (:requirements :strips :typing :action-costs)
  (:types location)
  (:functions (total-cost ?where - location))
  (:action move
    :parameters (?where - location)
    :precondition ()
    :effect (increase (total-cost ?where) 1)))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateDomain(domain));

        Assert.Contains("may not take arguments", exception.Message);
    }

    [Fact]
    public void ValidateDomain_RejectsActionCostThatReadsTotalCost()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain self-referential-cost)
  (:requirements :strips :action-costs)
  (:action move
    :parameters ()
    :precondition ()
    :effect (increase (total-cost) (+ (total-cost) 1))))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateDomain(domain));

        Assert.Contains("cannot be read", exception.Message);
    }

    [Fact]
    public void ValidateDomain_RejectsConditionalActionCost()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain conditional-cost)
  (:requirements :strips :conditional-effects :action-costs)
  (:predicates (enabled))
  (:action move
    :parameters ()
    :precondition ()
    :effect (when (enabled) (increase (total-cost) 1))))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateDomain(domain));

        Assert.Contains("Conditional action costs", exception.Message);
    }

    [Fact]
    public void CanonicalAst_NestedTrueConjunctionMakesActionCostUnconditional()
    {
        DomainDefinition parsed = ParseDomain("""
(define (domain nested-true-cost)
  (:requirements :strips :action-costs)
  (:action move
    :parameters ()))
""");
        ILogicalExpression condition = new And([
            new EmptyLogic(),
            new And(ImmutableArray<ILogicalExpression>.Empty)
        ]);
        var increase = new Increase(
            new FluentCall("total-cost", ImmutableArray<Mimir.Pddl.Ast.Models.Term>.Empty),
            new NumberLiteral(3m));
        ActionDefinition action = Assert.Single(parsed.Actions) with
        {
            Precondition = new EmptyLogic(),
            Effect = new AndEffect([
                new Mimir.Pddl.Ast.Effects.ConditionalEffect(condition, increase)
            ])
        };
        DomainDefinition domainDefinition = parsed with { Actions = [action] };

        Assert.True(domainDefinition.IsCanonical());
        CorePddlSupportValidator.ValidateDomain(domainDefinition);
        Domain domain = new(domainDefinition);

        NumericConstant cost = Assert.IsType<NumericConstant>(
            Assert.Single(domain.Actions).CostExpression);
        Assert.Equal(3d, cost.Value);
    }

    [Fact]
    public void ValidateDomain_RejectsQuantifiedActionCost()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain quantified-cost)
  (:requirements :strips :typing :conditional-effects :action-costs)
  (:types location)
  (:action move
    :parameters ()
    :precondition ()
    :effect (forall (?where - location) (increase (total-cost) 1))))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateDomain(domain));

        Assert.Contains("Quantified action costs", exception.Message);
    }

    [Theory]
    [InlineData(":precondition (> (fuel) 0) :effect (done)")]
    [InlineData(":precondition () :effect (when (> (fuel) 0) (done))")]
    public void ValidateDomain_AllowsNumericComparisonsInActions(string actionBody)
    {
        DomainDefinition domain = ParseDomain($$"""
(define (domain numeric-condition)
  (:requirements :strips :numeric-fluents :conditional-effects)
  (:predicates (done))
  (:functions (fuel))
  (:action act :parameters () {{actionBody}}))
""");

        CorePddlSupportValidator.ValidateDomain(domain);
    }

    [Fact]
    public void ValidateDomain_AllowsNumericComparisonInDerivedPredicate()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain numeric-derived)
  (:requirements :strips :numeric-fluents :derived-predicates)
  (:functions (fuel))
  (:derived (has-fuel) (> (fuel) 0)))
""");

        CorePddlSupportValidator.ValidateDomain(domain);
    }

    [Theory]
    [InlineData("(or (p) (q))")]
    [InlineData("(imply (p) (q))")]
    [InlineData("(forall (?x) (p))")]
    [InlineData("(= a b)")]
    [InlineData("(not (and (p) (q)))")]
    public void ValidateProblem_RejectsGoalsThatAreNotLiteralConjunctions(string goal)
    {
        DomainDefinition domain = ParseDomain("""
(define (domain goal-shapes)
  (:requirements :adl :numeric-fluents)
  (:predicates (p) (q))
  (:functions (fuel)))
""");
        ProblemDefinition problem = ParseProblem($$"""
(define (problem p)
  (:domain goal-shapes)
  (:objects a b)
  (:init (= (fuel) 0))
  (:goal {{goal}}))
""");

        Assert.Throws<NotSupportedException>(() => CorePddlSupportValidator.ValidateProblem(domain, problem));
    }

    [Theory]
    [InlineData("()")]
    [InlineData("(p)")]
    [InlineData("(not (q))")]
    [InlineData("(and)")]
    [InlineData("(and (p) (and (not (q))))")]
    public void ValidateProblem_AllowsLiteralConjunctionGoals(string goal)
    {
        DomainDefinition domain = ParseDomain("""
(define (domain goal-shapes)
  (:requirements :strips :negative-preconditions)
  (:predicates (p) (q)))
""");
        ProblemDefinition problem = ParseProblem($$"""
(define (problem p)
  (:domain goal-shapes)
  (:init)
  (:goal {{goal}}))
""");

        CorePddlSupportValidator.ValidateProblem(domain, problem);
    }

    [Theory]
    [InlineData("(:metric maximize (total-cost))")]
    [InlineData("(:metric maximize (fuel))")]
    [InlineData("(:metric minimize (fuel))")]
    [InlineData("(:metric minimize (+ (total-cost) 1))")]
    public void ValidateProblem_RejectsUnsupportedMetrics(string metric)
    {
        DomainDefinition domain = ParseDomain("""
(define (domain metrics)
  (:requirements :strips :action-costs)
  (:functions (fuel)))
""");
        ProblemDefinition problem = ParseProblem($$"""
(define (problem p)
  (:domain metrics)
  (:init (= (fuel) 0) (= (total-cost) 0))
  (:goal ())
  {{metric}})
""");

        Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateProblem(domain, problem));
    }

    [Fact]
    public void ValidateProblem_AllowsMinimizeTotalCostMetric()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain metrics)
  (:requirements :strips :action-costs))
""");
        ProblemDefinition problem = ParseProblem("""
(define (problem p)
  (:domain metrics)
  (:init (= (total-cost) 0))
  (:goal ())
  (:metric minimize (total-cost)))
""");

        CorePddlSupportValidator.ValidateProblem(domain, problem);
    }

    [Theory]
    [InlineData("(= (total-cost) 1)")]
    [InlineData("(= (total-cost somewhere) 0)")]
    [InlineData("(= (total-cost) 0) (= (total-cost) 0)")]
    public void ValidateProblem_RejectsInvalidTotalCostInitialization(string initialization)
    {
        DomainDefinition domain = ParseDomain("""
(define (domain initial-cost)
  (:requirements :strips :action-costs))
""");
        ProblemDefinition problem = ParseProblem($$"""
(define (problem p)
  (:domain initial-cost)
  (:init {{initialization}})
  (:goal ()))
""");

        Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateProblem(domain, problem));
    }

    [Fact]
    public void ValidateProblem_RejectsTotalCostInitializationWithoutActionCostsRequirement()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain initial-cost)
  (:requirements :strips :numeric-fluents)
  (:functions (total-cost)))
""");
        ProblemDefinition problem = ParseProblem("""
(define (problem p)
  (:domain initial-cost)
  (:init (= (total-cost) 0))
  (:goal ()))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateProblem(domain, problem));

        Assert.Contains(":action-costs", exception.Message);
    }

    [Fact]
    public void ValidateProblem_RejectsTotalCostMetricWithoutActionCostsRequirement()
    {
        DomainDefinition domain = ParseDomain("""
(define (domain metrics)
  (:requirements :strips :numeric-fluents)
  (:functions (total-cost)))
""");
        ProblemDefinition problem = ParseProblem("""
(define (problem p)
  (:domain metrics)
  (:init)
  (:goal ())
  (:metric minimize (total-cost)))
""");

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => CorePddlSupportValidator.ValidateProblem(domain, problem));

        Assert.Contains(":action-costs", exception.Message);
    }

    private static DomainDefinition ParseDomain(string pddl)
        => PddlParser.ParseDomain(pddl);

    private static ProblemDefinition ParseProblem(string pddl)
        => PddlParser.ParseProblem(pddl);
}
