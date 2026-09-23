using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;

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
        var action = Assert.Single(problem.GetApplicableActionGenerator(initial).GetApplicableActions(initial.Expand()));
        State successor = initial.Expand().Apply(action);
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
    public void Layout_IsFrozenAndResolvesUninitializedReferencesAsUndefined()
    {
        Problem problem = CreateProblem();
        NumericFunction function = problem.Domain.Functions.Single(function => function.Name == "fuel");
        var inputs = new Dictionary<NumericFunctionKey, double>
        {
            [new(function, [problem.ObjectLookup["truck1"]])] = 4
        };
        var changing = new List<NumericFunction> { function };
        var layout = new NumericStateLayout(inputs, changing);
        inputs.Clear();
        changing.Clear();
        Assert.Equal(1, layout.Count);
        Assert.Equal(4, layout.InitialValues[0]);
        Assert.Equal(new NumericFluentIndex(0), layout.Resolve(function, [problem.ObjectLookup["truck1"]]).Index);
        NumericField missing = layout.Resolve(function, [problem.ObjectLookup["truck2"]]);
        Assert.Null(missing.Index);
        Assert.True(double.IsNaN(missing.InitialValue));
        Assert.True(double.IsNaN(problem.InitialState.Value(problem.FunctionCall("fuel", "truck3"))));
        Assert.Equal(2, problem.Context.NumericLayout.Count);
    }

    [Fact]
    public void Layout_CopiesHigherArityKeysAndRejectsInfiniteInitialValues()
    {
        var function = new NumericFunction("amount", [new Variable("?a"), new Variable("?b"), new Variable("?c"), new Variable("?d")]);
        var first = new Constant("a", "object");
        var second = new Constant("b", "object");
        Constant[] arguments = [first, first, first, first];
        var initial = new Dictionary<NumericFunctionKey, double> { [new(function, arguments)] = 3 };
        var layout = new NumericStateLayout(initial, [function]);
        arguments[3] = second;
        Assert.Equal(3, layout.Resolve(function, [first, first, first, first]).InitialValue);
        Assert.True(double.IsNaN(layout.Resolve(function, arguments).InitialValue));
        Assert.Throws<ArgumentException>(() => new NumericStateLayout(new Dictionary<NumericFunctionKey, double>
        {
            [new(function, arguments)] = double.PositiveInfinity
        }, [function]));
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
        Domain domain = Domain.FromText("""
(define (domain numeric-storage)
  (:requirements :strips :numeric-fluents)
  (:predicates (done))
  (:functions (fuel ?truck) (distance))
  (:action finish :parameters () :precondition () :effect (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem storage) (:domain numeric-storage)
  (:objects truck1 truck2 truck3)
  (:init (= (fuel truck1) 20) (= (fuel truck2) 35) (= (distance) 7))
  (:goal ()))
""");
        if (!mutable) return problem;

        // Mutable domains cannot be loaded yet; exercise the storage contract with a classified internal layout.
        NumericFunction fuel = domain.Functions.Single(function => function.Name == "fuel");
        NumericFunction distance = domain.Functions.Single(function => function.Name == "distance");
        var values = new Dictionary<NumericFunctionKey, double>
        {
            [new(fuel, [problem.ObjectLookup["truck1"]])] = 20,
            [new(fuel, [problem.ObjectLookup["truck2"]])] = 35,
            [new(distance, [])] = 7
        };
        var context = new InstanceContext(problem, problem.AllObjects, new NumericStateLayout(values, [fuel]));
        context.SetStaticBitboardWords(Array.Empty<ulong>());
        context.InitializeDerivedClosure();
        problem.Context = context;
        return problem;
    }
}
