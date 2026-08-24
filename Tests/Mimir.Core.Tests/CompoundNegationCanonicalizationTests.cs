using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class CompoundNegationCanonicalizationTests
{
    [Fact]
    public void NegatedConjunctionPrecondition_IsTrueWhenOnlyOneConjunctIsTrue()
    {
        Problem problem = CreateProblem(
            """
(define (domain negated-conjunction-precondition)
  (:requirements :adl)
  (:predicates (p) (q) (done))
  (:action act
    :parameters ()
    :precondition (not (and (p) (q)))
    :effect (done)))
""",
            """
(define (problem test)
  (:domain negated-conjunction-precondition)
  (:init (p))
  (:goal (done)))
""");

        GroundAction action = new RpgGrounder().Ground(problem, problem.InitialState).Single();

        Assert.True(action.IsApplicable(problem.InitialState.Expand()));
    }

    [Fact]
    public void NegatedConjunctionConditionalEffect_FiresWhenOnlyOneConjunctIsTrue()
    {
        Problem problem = CreateProblem(
            """
(define (domain negated-conjunction-effect)
  (:requirements :adl)
  (:predicates (p) (q) (done))
  (:action act
    :parameters ()
    :effect (when (not (and (p) (q))) (done))))
""",
            """
(define (problem test)
  (:domain negated-conjunction-effect)
  (:init (p))
  (:goal (done)))
""");

        GroundAction action = new RpgGrounder().Ground(problem, problem.InitialState).Single();
        State successor = problem.InitialState.Expand().Apply(action);

        Assert.Contains(
            successor.GetTrueFacts(),
            fact => fact.Predicate.Name.Equals("done", StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public void NegatedEmptyLogicPrecondition_IsAlwaysFalse()
    {
        Problem problem = CreateProblem(
            """
(define (domain negated-empty-logic)
  (:requirements :adl)
  (:predicates (done))
  (:action act
    :parameters ()
    :precondition (not ())
    :effect (done)))
""",
            """
(define (problem test)
  (:domain negated-empty-logic)
  (:init)
  (:goal (done)))
""");

        GroundAction action = new RpgGrounder().Ground(problem, problem.InitialState).Single();

        Assert.False(action.IsApplicable(problem.InitialState.Expand()));
    }

    private static Problem CreateProblem(string domainPddl, string problemPddl)
    {
        Domain domain = Domain.FromText(domainPddl);
        return Problem.FromText(domain, problemPddl);
    }
}
