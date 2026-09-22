using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;

namespace Mimir.Core.Tests;

public class NumericPlanningTests
{
    internal static Problem Load(string effect, string precondition = "(>= (fuel ?t) 2)", string goal = "(<= (fuel a) 0)")
    {
        Domain domain = Domain.FromText($$"""
(define (domain numeric)
 (:requirements :adl :numeric-fluents :conditional-effects :action-costs)
 (:types truck) (:predicates (done ?t - truck))
 (:functions (fuel ?t - truck) (distance ?t - truck) (saved) (total-cost))
 (:action drive :parameters (?t - truck) :precondition {{precondition}}
  :effect (and {{effect}} (increase (total-cost) (distance ?t)))))
""");
        return Problem.FromText(domain, $$"""
(define (problem p) (:domain numeric) (:objects a b - truck)
 (:init (= (fuel a) 4) (= (fuel b) 8) (= (distance a) 4) (= (distance b) 7) (= (saved) 0) (= (total-cost) 0))
 (:goal {{goal}}) (:metric minimize (total-cost)))
""");
    }

    [Fact]
    public void LoadedActions_CheckConditionsAndApplyNumericEffects()
    {
        Problem problem = Load("(decrease (fuel ?t) 2)");
        var action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        GroundFunctionCall fuel = problem.FunctionCall("fuel", "a");
        State initial = problem.InitialState;
        Assert.Single(action.Precondition.Comparisons);
        Assert.Single(action.Effect.NumericUpdates);
        Assert.True(action.IsApplicable(initial.Expand()));
        Assert.Equal(4, action.Cost);
        State next = initial.Expand().Apply(action);
        Assert.Equal(2, next.Value(fuel));
        State last = next.Expand().Apply(action);
        Assert.False(action.IsApplicable(last.Expand()));
        Assert.True(last.Holds(Assert.Single(problem.NumericGoals)));
        Assert.Equal(4, initial.Value(fuel));
        Assert.NotEmpty(problem.GetApplicableActionGenerator(initial).GetApplicableActions(initial.Expand()));
    }

    [Fact]
    public void ConditionalAndQuantifiedEffects_ReadTheSourceState()
    {
        Problem problem = Load("""
(assign (saved) (fuel ?t))
(decrease (fuel ?t) 2)
(when (= (fuel ?t) 4) (done ?t))
(forall (?u - truck) (when (>= (fuel ?u) 4) (increase (fuel ?u) 1)))
""");
        var action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        State next = problem.InitialState.Expand().Apply(action);
        Assert.Equal(3, next.Value(problem.FunctionCall("fuel", "a")));
        Assert.Equal(9, next.Value(problem.FunctionCall("fuel", "b")));
        Assert.Equal(4, next.Value(problem.FunctionCall("saved")));
        Assert.True(next.Value(problem.Atom("done", "a")));
    }

    [Theory]
    [InlineData("(assign (fuel ?t) 3)", 3)]
    [InlineData("(increase (fuel ?t) 3)", 7)]
    [InlineData("(scale-up (fuel ?t) 3)", 12)]
    [InlineData("(scale-down (fuel ?t) 2)", 2)]
    [InlineData("(increase (fuel ?t) 1) (increase (fuel ?t) 1)", 6)]
    [InlineData("(forall (?u - truck) (increase (fuel ?t) 1))", 6)]
    public void Updates_PreserveOperatorAndMultiplicity(string effects, double expected)
    {
        Problem problem = Load(effects);
        var action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        Assert.Equal(expected, problem.InitialState.Expand().Apply(action).Value(problem.FunctionCall("fuel", "a")));
    }

    [Theory]
    [InlineData("(assign (fuel ?t) 3) (increase (fuel ?t) 1)")]
    [InlineData("(scale-up (fuel ?t) 2) (scale-down (fuel ?t) 2)")]
    public void ConflictingWrites_FailWithoutChangingSource(string effects)
    {
        Problem problem = Load(effects);
        var action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        Assert.Throws<InvalidOperationException>(() => problem.InitialState.Expand().Apply(action));
        Assert.Equal(4, problem.InitialState.Value(problem.FunctionCall("fuel", "a")));
    }
    [Fact]
    public void UntriggeredWritesShareStorageAndUndefinedUpdatesMakeActionsInapplicable()
    {
        Problem problem = Load("(when (= (fuel ?t) 0) (increase (fuel ?t) 1)) (done ?t)");
        var action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        State next = problem.InitialState.Expand().Apply(action);
        Assert.True(System.Runtime.CompilerServices.Unsafe.AreSame(
            ref System.Runtime.InteropServices.MemoryMarshal.GetReference(problem.InitialState.NumericValues),
            ref System.Runtime.InteropServices.MemoryMarshal.GetReference(next.NumericValues)));
        Problem invalid = Load("(scale-down (fuel ?t) 0)");
        var division = invalid.GroundAction(invalid.Domain.Actions[0], invalid.ObjectLookup["a"]);
        Assert.False(division.IsApplicable(invalid.InitialState.Expand()));
        Assert.Throws<InvalidOperationException>(() => invalid.InitialState.Expand().Apply(division));
        Assert.Equal(4, invalid.InitialState.Value(invalid.FunctionCall("fuel", "a")));
    }

    [Theory]
    [InlineData("(when (>= (fuel ?t) 0) (scale-down (fuel ?t) 0))", false)]
    [InlineData("(when (> (fuel ?t) 100) (scale-down (fuel ?t) 0))", true)]
    public void TriggeredUndefinedConditionalUpdatesMakeActionsInapplicable(string effect, bool applicable)
    {
        Problem problem = Load(effect);
        var action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        Assert.Equal(applicable, action.IsApplicable(problem.InitialState.Expand()));
        Assert.Equal(applicable, problem.GetApplicableActionGenerator(problem.InitialState)
            .GetApplicableActions(problem.InitialState.Expand()).Contains(action));
    }

    [Fact]
    public void UndefinedFluentsCanOnlyBeAssigned()
    {
        Domain domain = Domain.FromText("""
(define (domain missing) (:requirements :strips :numeric-fluents)
 (:functions (fuel ?t))
 (:action write :parameters (?t) :effect (assign (fuel ?t) 1))
 (:action bump :parameters (?t) :effect (increase (fuel ?t) 1))
 (:action read :parameters (?t) :precondition (> (fuel ?t) 0) :effect (and)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain missing) (:objects a b) (:init (= (fuel a) 2)) (:goal (and)))
""");
        Constant b = problem.ObjectLookup["b"];
        var write = problem.GroundAction(domain.Actions[0], b);
        var bump = problem.GroundAction(domain.Actions[1], b);
        var read = problem.GroundAction(domain.Actions[2], b);
        ExtendedState initial = problem.InitialState.Expand();
        Assert.Equal(2, problem.InitialState.NumericValues.Length);
        Assert.True(double.IsNaN(problem.InitialState.Value(problem.FunctionCall("fuel", "b"))));
        Assert.True(write.IsApplicable(initial));
        Assert.False(bump.IsApplicable(initial));
        Assert.False(read.IsApplicable(initial));
        ExtendedState next = initial.Apply(write).Expand();
        Assert.Equal(1, next.State.Value(problem.FunctionCall("fuel", "b")));
        Assert.True(bump.IsApplicable(next));
        Assert.True(read.IsApplicable(next));
    }

    [Fact]
    public void NumericOnlyBindingsNeverReuseDifferentValuationsOrTargets()
    {
        Problem problem = Load("(decrease (fuel ?t) 1)");
        ActionSchema schema = problem.Domain.Actions[0];
        var first = problem.GroundAction(schema, problem.ObjectLookup["a"]);
        var second = problem.GroundAction(schema, problem.ObjectLookup["b"]);
        Assert.All(ActionBuilder.FindPreconditionRelevantParameters(schema), Assert.True);
        Assert.All(ActionBuilder.FindTransitionRelevantParameters(schema), Assert.True);
        Assert.Equal(first, problem.GroundAction(schema, problem.ObjectLookup["a"]));
        State next = problem.InitialState.Expand().Apply(second);
        Assert.Equal(4, next.Value(problem.FunctionCall("fuel", "a")));
        Assert.Equal(7, next.Value(problem.FunctionCall("fuel", "b")));
        Assert.Equal(4, first.Cost);
        Assert.Equal(7, second.Cost);
    }

    [Fact]
    public void ProgrammaticNumericExpressionsKeepDoublePrecision()
    {
        var builder = new DomainBuilder("precise");
        builder.Requirements().Add(":numeric-fluents").Close();
        builder.Functions().Add("value").Add("tiny").Close();
        builder.Actions().Add("set").Assign(Numeric.Function("value"), Numeric.Function("tiny")).Close().Close();
        Domain domain = builder.Build();
        var initial = new ProblemBuilder(domain, "p");
        initial.InitialState().SetValue(Numeric.Function("value"), 0).SetValue(Numeric.Function("tiny"), double.Epsilon).Close();
        Problem problem = initial.Build();
        NumericFunction tiny = domain.Functions.Single(function => function.Name == "tiny");
        Assert.Equal(double.Epsilon, problem.GetNumericFunctionValue(tiny, []));
        // State values snap to the comparison epsilon grid, so a sub-epsilon assignment is stored as zero.
        State next = problem.InitialState.Expand().Apply(problem.GroundAction(domain.Actions[0]));
        Assert.Equal(0d, next.Value(problem.FunctionCall("value")));
    }

    [Fact]
    public void ConflictingAndOverflowingUpdatesMakeActionsInapplicable()
    {
        Domain domain = Domain.FromText("""
(define (domain alias) (:requirements :strips :numeric-fluents)
 (:functions (fuel ?t))
 (:action update :parameters (?a ?b) :effect (and (assign (fuel ?a) 1) (increase (fuel ?b) 2)))
 (:action overflow :parameters (?a) :effect (scale-up (fuel ?a) 2)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain alias) (:objects a b) (:init (= (fuel a) 4) (= (fuel b) 8)) (:goal (and)))
""");
        Constant a = problem.ObjectLookup["a"];
        var alias = problem.GroundAction(domain.Actions[0], a, a);
        Assert.False(alias.IsApplicable(problem.InitialState.Expand()));
        Assert.Throws<InvalidOperationException>(() => problem.InitialState.Expand().Apply(alias));
        var separate = problem.GroundAction(domain.Actions[0], a, problem.ObjectLookup["b"]);
        State next = problem.InitialState.Expand().Apply(separate);
        Assert.Equal(1, next.Value(problem.FunctionCall("fuel", "a")));
        Assert.Equal(10, next.Value(problem.FunctionCall("fuel", "b")));
        State large = StateFactory.Default.Create(problem.Context, [], new Dictionary<GroundFunctionCall, double>
        {
            [problem.FunctionCall("fuel", "a")] = double.MaxValue,
            [problem.FunctionCall("fuel", "b")] = 8
        });
        var overflow = problem.GroundAction(domain.Actions[1], a);
        Assert.False(overflow.IsApplicable(large.Expand()));
        Assert.Throws<InvalidOperationException>(() => large.Expand().Apply(overflow));
        Assert.Equal(double.MaxValue, large.Value(problem.FunctionCall("fuel", "a")));
        GroundConjunctiveCondition condition = problem.GroundCondition([], [new GroundNumericComparison(
            problem.FunctionCall("fuel", "a"), ComparisonOperator.Equal, 1)]);
        Assert.True(next.Holds(condition));
        Assert.False(problem.InitialState.Holds(condition));
    }
}
