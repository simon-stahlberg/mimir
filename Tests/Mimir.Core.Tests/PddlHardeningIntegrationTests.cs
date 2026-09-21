using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

public class PddlHardeningIntegrationTests
{
    [Fact]
    public void NegativeInitialFactRemainsFalseUnderClosedWorldSemantics()
    {
        Domain domain = CreateFactsDomain();

        Problem problem = Problem.FromText(domain, """
(define (problem negative-initial-fact)
  (:domain facts)
  (:objects a)
  (:init (not (p a)))
  (:goal (not (p a))))
""");

        Assert.Empty(problem.InitialState.GetTrueFacts());
        Literal<Fact> goal = Assert.Single(problem.Goal);
        Assert.True(goal.IsNegative);
    }

    [Fact]
    public void ContradictoryInitialFactsAreRejectedDuringProblemConstruction()
    {
        Domain domain = CreateFactsDomain();

        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Problem.FromText(domain, """
(define (problem contradictory-initial-facts)
  (:domain facts)
  (:objects a)
  (:init (p a) (not (p a)))
  (:goal (p a)))
"""));

        Assert.Equal(PddlLoadErrorCode.Validation, exception.ErrorCode);
    }

    [Fact]
    public void DerivedPredicateInitialFactIsRejectedDuringProblemConstruction()
    {
        Domain domain = Domain.FromText("""
(define (domain derived-facts)
  (:requirements :strips :derived-predicates)
  (:predicates (base ?x))
  (:derived (reachable ?x) (base ?x)))
""");

        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Problem.FromText(domain, """
(define (problem derived-initial-fact)
  (:domain derived-facts)
  (:objects a)
  (:init (reachable a))
  (:goal (base a)))
"""));

        Assert.Equal(PddlLoadErrorCode.Validation, exception.ErrorCode);
    }

    [Fact]
    public void BareObjectEqualityGoalIsAnOrdinaryStaticLiteral()
    {
        Domain domain = Domain.FromText("""
(define (domain object-equality)
  (:requirements :strips :equality))
""");

        Problem problem = Problem.FromText(domain, """
(define (problem object-equality-problem)
  (:domain object-equality)
  (:objects a b)
  (:init)
  (:goal (= a b)))
""");

        Literal<Fact> literal = Assert.Single(problem.Goal);
        Assert.Equal("=", literal.Value.Predicate.Name);
        Assert.False(problem.InitialState.Expand().IsTrue(literal.Value));
    }

    [Fact]
    public void NumericComparisonGoalIsRejectedInsteadOfTreatedAsTrue()
    {
        Domain domain = Domain.FromText("""
(define (domain numeric-equality)
  (:requirements :strips :numeric-fluents)
  (:functions (left-value) (right-value)))
""");

        Assert.Throws<NotImplementedException>(() => Problem.FromText(domain, """
(define (problem numeric-equality-problem)
  (:domain numeric-equality)
  (:init)
  (:goal (= left-value right-value)))
"""));

    }

    [Fact]
    public void NestedQuantifiedEffectsApplyToEnabledPairs()
    {
        Domain domain = Domain.FromText("""
(define (domain nested-quantified-effects)
  (:requirements :strips :typing :adl)
  (:types item)
  (:predicates
    (enabled ?left - item ?right - item)
    (marked ?left - item ?right - item))
  (:action mark-enabled
    :parameters ()
    :precondition (and)
    :effect
      (forall (?left - item)
        (forall (?right - item)
          (when (enabled ?left ?right)
            (marked ?left ?right))))))
""");

        Problem problem = Problem.FromText(domain, """
(define (problem nested-quantified-effects-problem)
  (:domain nested-quantified-effects)
  (:objects a b - item)
  (:init (enabled a b))
  (:goal (marked a b)))
""");

        Mimir.Core.Grounding.Action action = Assert.Single(
            new RpgGrounder().Ground(problem, problem.InitialState));
        State successor = problem.InitialState.Expand().Apply(action);
        Mimir.Core.Schemas.Predicate<Fluent> marked = domain.Fluents.Single(
            predicate => predicate.Name == "marked");
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];

        Assert.Contains(successor.GetTrueFacts(), fact =>
            ReferenceEquals(fact.Predicate, marked) && fact.Arguments.SequenceEqual([a, b]));
        Assert.DoesNotContain(successor.GetTrueFacts(), fact =>
            ReferenceEquals(fact.Predicate, marked) && fact.Arguments.SequenceEqual([b, a]));
    }

    [Fact]
    public void ManualDomainRejectsMultiNodeTypeCycle()
    {
        var hierarchy = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["first"] = "second",
            ["second"] = "first"
        };

        ArgumentException exception = Assert.Throws<ArgumentException>(() => new Domain(
            "cyclic-types",
            Array.Empty<Mimir.Core.Schemas.Predicate<Fluent>>(),
            Array.Empty<Mimir.Core.Schemas.Predicate<Static>>(),
            Array.Empty<DerivedPredicateDefinition>(),
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            hierarchy));

        Assert.Contains("cycle", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    private static Domain CreateFactsDomain()
    {
        return Domain.FromText("""
(define (domain facts)
  (:requirements :strips)
  (:predicates (p ?x))
  (:action set-p
    :parameters (?x)
    :precondition (and)
    :effect (p ?x)))
""");
    }
}
