using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;

namespace Mimir.Core.Tests;

public class NumericApiTests
{
    private static Problem CreateProblem()
    {
        Domain domain = Domain.FromText("""
(define (domain resources)
  (:requirements :strips :typing :negative-preconditions :conditional-effects :action-costs)
  (:types truck)
  (:predicates (ready ?t - truck) (done ?t - truck) (flag))
  (:functions (total-cost) (fuel ?t - truck))
  (:action drive :parameters (?t - truck)
    :precondition (and (ready ?t) (not (done ?t)))
    :effect (and (not (ready ?t)) (done ?t) (when (flag) (ready ?t))
                 (increase (total-cost) (+ (fuel ?t) 1)))))
""");
        return Problem.FromText(domain, """
(define (problem one-truck) (:domain resources)
  (:objects truck1 - truck)
  (:init (ready truck1) (= (fuel truck1) 20) (= (total-cost) 0))
  (:goal (done truck1)) (:metric minimize (total-cost)))
""");
    }

    [Fact]
    public void StateValues_EvaluateBooleanAndStaticNumericExpressions()
    {
        Problem problem = CreateProblem();
        GroundFunctionCall fuel = problem.FunctionCall("fuel", "truck1");
        Fact ready = problem.Atom("ready", "truck1");
        State state = problem.InitialState;
        Assert.True(state.Value(ready));
        Assert.Equal(20d, state.Value(fuel));
        Assert.Equal(41d, state.Value(2 * fuel + 1));
        Assert.True(state.Holds(fuel.GreaterThanOrEqual(20)));
        Assert.False(state.Holds(fuel.LessThan(20)));
        Assert.Equal(fuel, problem.FunctionCall("fuel", "truck1"));
        Assert.Throws<DivideByZeroException>(() => state.Value(fuel / 0));
        Assert.Throws<ArgumentException>(() => state.Value(CreateProblem().FunctionCall("fuel", "truck1")));
    }

    [Fact]
    public void Actions_ExposeMatchingLiftedAndGroundInspection()
    {
        Problem problem = CreateProblem();
        ActionSchema schema = Assert.Single(problem.Domain.Actions);
        Mimir.Core.Grounding.Action action = Assert.Single(problem.GetApplicableActionGenerator(problem.InitialState)
            .GetApplicableActions(problem.InitialState.Expand()));
        Assert.Equal(2, schema.Precondition.Literals.Count);
        Assert.Equal(2, action.Precondition.Literals.Count);
        Assert.Empty(schema.Precondition.Comparisons);
        Assert.Empty(action.Precondition.Comparisons);
        Assert.Equal(2, schema.Effect.Literals.Count);
        Assert.Equal(2, action.Effect.Literals.Count);
        Assert.Empty(schema.Effect.NumericUpdates);
        Assert.Empty(action.Effect.NumericUpdates);
        ConditionalEffect conditional = Assert.Single(schema.ConditionalEffects);
        Assert.Single(conditional.Condition.Literals);
        Assert.Single(conditional.Effect.Literals);
        NumericBinaryExpression lifted = Assert.IsType<NumericBinaryExpression>(schema.CostExpression);
        NumericBinaryExpression grounded = Assert.IsType<NumericBinaryExpression>(action.CostExpression);
        Assert.Same(schema.Parameters[0], Assert.Single(Assert.IsType<FunctionCall>(lifted.Left).Arguments));
        Assert.Same(action.Arguments[0], Assert.Single(Assert.IsType<GroundFunctionCall>(grounded.Left).Arguments));
        Assert.Equal(action.Cost, problem.InitialState.Value(grounded));
        Assert.True(problem.InitialState.Holds(action.Precondition));
    }

    [Fact]
    public void Builders_ExposeNumericExpressionsButRejectMissingPlanning()
    {
        NumericFunctionSpec fuel = Numeric.Function("fuel", "?t");
        ActionSchemaBuilder action = new DomainBuilder("numeric").Actions().Add("drive");
        Assert.Throws<NotImplementedException>(() => action.AddPrecondition(fuel.GreaterThanOrEqual(1)));
        Assert.Throws<NotImplementedException>(() => action.Assign(fuel, 1));
        Assert.Throws<NotImplementedException>(() => action.Increase(fuel, 1));
        Assert.Throws<NotImplementedException>(() => action.Decrease(fuel, 1));
        ConditionalEffectBuilder conditional = action.AddConditionalEffect();
        Assert.Throws<NotImplementedException>(() => conditional.AddCondition(fuel.EqualTo(1)));
        Assert.Throws<NotImplementedException>(() => conditional.Assign(fuel, 1));
        Assert.Throws<NotImplementedException>(() => new ProblemBuilder(CreateProblem().Domain, "p")
            .Goal().Add(fuel.LessThan(1)));
    }
}
