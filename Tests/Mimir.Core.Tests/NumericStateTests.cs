using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class NumericStateTests
{
    [Fact]
    public void EmptyNumericStorage_IsSharedForBooleanAndStaticNumericProblems()
    {
        Problem boolean = TestProblemFixture.Create(["done"]).Problem;
        Problem statics = CreateProblem(mutable: false);
        foreach (Problem problem in new[] { boolean, statics })
        {
            State initial = problem.InitialState;
            State empty = StateFactory.Default.CreateEmpty(problem.Context);
            State custom = StateFactory.Default.Create(problem.Context, Array.Empty<Fact<Fluent>>(),
                new Dictionary<GroundFunctionCall, double>());
            var action = Assert.Single(problem.GetApplicableActionGenerator(initial).GetApplicableActions(initial.Expand()));
            State successor = initial.Expand().Apply(action);
            foreach (State state in new[] { initial, empty, custom, successor })
            {
                Assert.True(Unsafe.AreSame(ref MemoryMarshal.GetReference(Array.Empty<double>().AsSpan()),
                    ref MemoryMarshal.GetReference(state.NumericValues)));
            }
        }
        Assert.Equal(20, statics.InitialState.Value(statics.FunctionCall("fuel", "truck1")));
    }

    [Fact]
    public void InitialStateAndCustomStates_UseTheirOwnNumericValues()
    {
        Problem problem = CreateProblem();
        GroundFunctionCall fuel = problem.FunctionCall("fuel", "truck1");
        GroundFunctionCall otherFuel = problem.FunctionCall("fuel", "truck2");
        GroundFunctionCall distance = problem.FunctionCall("distance");
        State initial = problem.InitialState;
        Assert.Equal(2, initial.NumericValues.Length);
        Assert.Equal(20, initial.Value(fuel));
        Assert.Equal(35, initial.Value(otherFuel));
        Assert.Equal(7, initial.Value(distance));

        var input = new Dictionary<GroundFunctionCall, double> { [fuel] = 3, [otherFuel] = 0 };
        State custom = StateFactory.Default.Create(problem.Context, [], input);
        input[fuel] = 900;
        Assert.Equal(3, custom.Value(fuel));
        Assert.Equal(0, custom.Value(otherFuel));
        Assert.Equal(7, custom.Value(distance));
        Assert.Equal(20, initial.Value(fuel));
        Assert.Equal(7, custom.Value(fuel * 2 + 1));
        Assert.True(custom.Holds(fuel.LessThan(4)));
    }

    [Fact]
    public void BooleanChanges_ShareNumericStorageWithoutMutatingTheSource()
    {
        Problem problem = CreateProblem();
        State initial = problem.InitialState;
        Action finish = problem.GroundAction(problem.Domain.Actions.Single(action => action.Name == "finish"));
        State successor = initial.Expand().Apply(finish);
        Fact<Fluent> done = Assert.IsType<Fact<Fluent>>(problem.Atom("done"));
        State withFact = initial.WithAdditionalFluentFacts([done]);
        Assert.False(initial.Value(done));
        Assert.True(successor.Value(done));
        Assert.Equal(successor, withFact);
        Assert.True(Unsafe.AreSame(ref MemoryMarshal.GetReference(initial.NumericValues),
            ref MemoryMarshal.GetReference(successor.NumericValues)));
        Assert.True(Unsafe.AreSame(ref MemoryMarshal.GetReference(initial.NumericValues),
            ref MemoryMarshal.GetReference(withFact.NumericValues)));
        Assert.Equal(20, successor.Value(problem.FunctionCall("fuel", "truck1")));
    }

    [Fact]
    public void NumericChanges_ShareFluentStorageWithoutMutatingTheSource()
    {
        Problem problem = CreateProblem();
        Action finish = problem.GroundAction(problem.Domain.Actions.Single(action => action.Name == "finish"));
        Action refuel = problem.GroundAction(
            problem.Domain.Actions.Single(action => action.Name == "refuel"), problem.ObjectLookup["truck1"]);
        State finished = problem.InitialState.Expand().Apply(finish);
        State refueled = finished.Expand().Apply(refuel);
        GroundFunctionCall fuel = problem.FunctionCall("fuel", "truck1");

        Assert.True(Unsafe.AreSame(ref MemoryMarshal.GetReference(finished.Bitboard),
            ref MemoryMarshal.GetReference(refueled.Bitboard)));
        Assert.True(refueled.Value(problem.Atom("done")));
        Assert.Equal(20, finished.Value(fuel));
        Assert.Equal(21, refueled.Value(fuel));
        Assert.NotEqual(finished, refueled);
    }

    [Fact]
    public void ActionsWithoutFiringEffects_ReturnTheSourceState()
    {
        Domain domain = Domain.FromText("""
(define (domain idle) (:requirements :strips :numeric-fluents :conditional-effects)
  (:functions (fuel))
  (:action wait :parameters () :precondition (and) :effect (when (> (fuel) 10) (increase (fuel) 1))))
""");
        Problem problem = Problem.FromText(domain, "(define (problem p) (:domain idle) (:init (= (fuel) 2)) (:goal ()))");
        State initial = problem.InitialState;

        Assert.Same(initial, initial.Expand().Apply(problem.GroundAction(domain.Actions.Single())));
    }

    [Fact]
    public void EqualityAndHashing_IncludeNumericValuesAndNormalizeSignedZero()
    {
        Problem problem = CreateProblem();
        double[] input = [20, 0];
        var first = new State(problem.Context, [], input);
        int originalHash = first.GetHashCode();
        input[0] = 99;
        var same = new State(problem.Context, [0UL, 0UL], [20, -0d]);
        var different = new State(problem.Context, [], [21, 0]);
        Assert.Equal(first, same);
        Assert.Equal(originalHash, same.GetHashCode());
        Assert.NotEqual(first, different);
        Assert.Equal(2, new HashSet<State> { first, same, different }.Count);
        Assert.Equal(20, first.NumericValues[0]);
        Assert.Equal(0L, BitConverter.DoubleToInt64Bits(same.NumericValues[1]));
        Assert.NotEqual(first, new State(CreateProblem().Context, [], [20, 0]));
    }

    [Fact]
    public void CustomStates_RejectIncompleteDuplicateStaticAndForeignAssignments()
    {
        Problem problem = CreateProblem();
        GroundFunctionCall fuel = problem.FunctionCall("fuel", "truck1");
        GroundFunctionCall second = problem.FunctionCall("fuel", "truck2");
        Assert.Throws<ArgumentException>(() => StateFactory.Default.CreateEmpty(problem.Context));
        Assert.Throws<ArgumentException>(() => StateFactory.Default.Create(problem.Context, []));
        Assert.Throws<ArgumentException>(() => StateFactory.Default.Create(problem.Context, [], new Dictionary<GroundFunctionCall, double> { [fuel] = 1 }));
        Assert.Throws<ArgumentException>(() => StateFactory.Default.Create(problem.Context, [], new Dictionary<GroundFunctionCall, double>
        {
            [fuel] = 1, [second] = 2, [problem.FunctionCall("distance")] = 3
        }));
        Assert.Throws<ArgumentException>(() => StateFactory.Default.Create(problem.Context, [], new Dictionary<GroundFunctionCall, double>
        {
            [fuel] = 1, [CreateProblem().FunctionCall("fuel", "truck2")] = 2
        }));
        var duplicate = new Dictionary<GroundFunctionCall, double>(ReferenceEqualityComparer.Instance)
        {
            [fuel] = 1, [problem.FunctionCall("fuel", "truck1")] = 2
        };
        Assert.Throws<ArgumentException>(() => StateFactory.Default.Create(problem.Context, [], duplicate));
        Assert.Throws<ArgumentException>(() => new State(problem.Context, [], [1]));
        Assert.True(double.IsNaN(new State(problem.Context, [], [1, double.NaN]).NumericValues[1]));
        Assert.Throws<ArgumentException>(() => new State(problem.Context, [], [1, double.PositiveInfinity]));
    }

    [Fact]
    public void FunctionCalls_AreInternedAndUninitializedOnesAreUndefined()
    {
        Problem problem = CreateProblem();
        NumericFunction fuel = problem.Domain.Functions.Single(function => function.Name == "fuel");
        Constant truck3 = problem.ObjectLookup["truck3"];

        Assert.Same(problem.FunctionCall("fuel", "truck1"), problem.FunctionCall("fuel", "truck1"));
        Assert.NotNull(problem.FunctionCall("fuel", "truck1").StateIndex);
        Assert.NotNull(problem.FunctionCall("fuel", "truck2").StateIndex);
        Assert.Null(problem.FunctionCall("distance").StateIndex);
        Assert.Equal(7, problem.InitialState.Value(problem.FunctionCall("distance")));
        Assert.Equal(2, problem.Context.NumericStateSize);

        Assert.Null(problem.Context.TryGetFunctionCall(fuel, [truck3]));
        GroundFunctionCall missing = problem.FunctionCall("fuel", "truck3");
        Assert.Same(missing, problem.Context.TryGetFunctionCall(fuel, [truck3]));
        Assert.Null(missing.StateIndex);
        Assert.True(double.IsNaN(problem.InitialState.Value(missing)));
        Assert.Equal(2, problem.Context.NumericStateSize);
    }

    [Fact]
    public void RegisteredFunctionCallsOwnTheirArgumentsAndRejectInfiniteInitialValues()
    {
        Problem problem = CreateProblem();
        NumericFunction fuel = problem.Domain.Functions.Single(function => function.Name == "fuel");
        Constant truck1 = problem.ObjectLookup["truck1"];
        Constant[] arguments = [truck1];
        var context = new InstanceContext(problem, problem.AllObjects,
            new Dictionary<NumericFunctionKey, double> { [new(fuel, arguments)] = 3 }, new HashSet<NumericFunction> { fuel });
        arguments[0] = problem.ObjectLookup["truck2"];

        Assert.Equal(3, Assert.IsType<GroundFunctionCall>(context.TryGetFunctionCall(fuel, [truck1])).InitialValue);
        Assert.Null(context.TryGetFunctionCall(fuel, arguments));
        Assert.Throws<ArgumentException>(() => new InstanceContext(problem, problem.AllObjects,
            new Dictionary<NumericFunctionKey, double> { [new(fuel, [truck1])] = double.PositiveInfinity },
            new HashSet<NumericFunction> { fuel }));
    }

    [Fact]
    public void StatesThatDifferOnlyByFloatingPointNoiseAreEqual()
    {
        Problem problem = CreateProblem();
        var noisy = new State(problem.Context, [], [0.1 + 0.2, 1]);
        var exact = new State(problem.Context, [], [0.3, 1]);
        Assert.Equal(exact, noisy);
        Assert.Equal(exact.GetHashCode(), noisy.GetHashCode());
        Assert.Equal(0.3, noisy.NumericValues[0]);
        Assert.NotEqual(exact, new State(problem.Context, [], [0.3 + 1e-6, 1]));
        Assert.Equal(new State(problem.Context, [], [double.NaN, 1]), new State(problem.Context, [], [double.NaN, 1]));
    }

    [Fact]
    public void ComparisonsUseEpsilonAndFailOnUndefinedValues()
    {
        Assert.True(NumericEvaluation.Compare(ComparisonOperator.Equal, 0.1 + 0.2, 0.3));
        Assert.True(NumericEvaluation.Compare(ComparisonOperator.LessThanOrEqual, 0.3 + 1e-12, 0.3));
        Assert.True(NumericEvaluation.Compare(ComparisonOperator.GreaterThanOrEqual, 0.3 - 1e-12, 0.3));
        Assert.False(NumericEvaluation.Compare(ComparisonOperator.LessThan, 0.3 - 1e-12, 0.3));
        Assert.False(NumericEvaluation.Compare(ComparisonOperator.GreaterThan, 0.3 + 1e-12, 0.3));
        Assert.True(NumericEvaluation.Compare(ComparisonOperator.LessThan, 0.3, 0.3 + 1e-6));
        foreach (ComparisonOperator operation in Enum.GetValues<ComparisonOperator>())
        {
            Assert.False(NumericEvaluation.Compare(operation, double.NaN, 0));
            Assert.False(NumericEvaluation.Compare(operation, 0, double.NaN));
        }

        Problem problem = CreateProblem();
        GroundFunctionCall fuel = problem.FunctionCall("fuel", "truck1");
        Assert.True(problem.InitialState.Holds((fuel + 1e-12).EqualTo(20)));
    }

    private static Problem CreateProblem(bool mutable = true)
    {
        string refuel = mutable
            ? "(:action refuel :parameters (?truck) :precondition (and) :effect (increase (fuel ?truck) 1))"
            : "";
        Domain domain = Domain.FromText($$"""
(define (domain numeric-storage)
  (:requirements :strips :numeric-fluents)
  (:predicates (done))
  (:functions (fuel ?truck) (distance))
  (:action finish :parameters () :precondition () :effect (done))
  {{refuel}})
""");
        return Problem.FromText(domain, """
(define (problem storage) (:domain numeric-storage)
  (:objects truck1 truck2 truck3)
  (:init (= (fuel truck1) 20) (= (fuel truck2) 35) (= (distance) 7))
  (:goal ()))
""");
    }
}
