using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Interop;
using Xunit;

namespace Mimir.Search.Tests;

public unsafe class InteropNumericStateTests
{
    [Theory]
    [InlineData(2, true)]
    [InlineData(1, false)]
    [InlineData(0, false)]
    public void StateConstruction_TransfersCompleteNumericValuations(int count, bool succeeds)
    {
        Domain domain = Domain.FromText("""
(define (domain numeric-interop) (:requirements :strips :numeric-fluents)
  (:functions (fuel ?truck)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem storage) (:domain numeric-interop) (:objects a b)
  (:init (= (fuel a) 20) (= (fuel b) 35)) (:goal ()))
""");
        NumericFunction function = Assert.Single(domain.Functions);
        var initialValues = new Dictionary<NumericFunctionKey, double>
        {
            [new(function, [problem.ObjectLookup["a"]])] = 20,
            [new(function, [problem.ObjectLookup["b"]])] = 35
        };
        var context = new InstanceContext(problem, problem.AllObjects, new NumericStateLayout(initialValues, [function]));
        context.SetStaticBitboardWords(Array.Empty<ulong>());
        context.InitializeDerivedClosure();
        problem.Context = context;
        GroundFunctionCall first = problem.FunctionCall("fuel", "a");
        GroundFunctionCall second = problem.FunctionCall("fuel", "b");
        int problemHandle = ObjectRegistry.Store(problem);
        int firstHandle = ObjectRegistry.Store(first);
        int secondHandle = ObjectRegistry.Store(second);
        int result = 0;
        Exports.NativeError error = default;
        try
        {
            int* fields = stackalloc int[] { firstHandle, secondHandle };
            double* values = stackalloc double[] { 4, 0 };
            delegate* unmanaged<int, IntPtr, int, IntPtr, IntPtr, int, int> create = &Exports.ProblemNewState;
            result = create(problemHandle, IntPtr.Zero, 0, (IntPtr)fields, (IntPtr)values, count);
            delegate* unmanaged<IntPtr, byte> takeError = &Exports.TakeLastError;
            if (!succeeds)
            {
                Assert.Equal(0, result);
                Assert.Equal(1, takeError((IntPtr)(&error)));
                Assert.Equal((int)Exports.NativeErrorCode.InvalidArgument, error.Code);
                return;
            }
            Assert.NotEqual(0, result);
            Assert.Equal(0, takeError((IntPtr)(&error)));
            values[0] = 900;
            delegate* unmanaged<int, int, double> evaluate = &Exports.StateNumericValue;
            Assert.Equal(4, evaluate(result, firstHandle));
            Assert.Equal(0, evaluate(result, secondHandle));
            State state = ObjectRegistry.Get<ExtendedState>(result)!.State;
            Assert.Equal(StateFactory.Default.Create(context, [], new Dictionary<GroundFunctionCall, double>
            {
                [first] = 4, [second] = 0
            }), state);
        }
        finally
        {
            ObjectRegistry.Release(result);
            ObjectRegistry.Release(firstHandle);
            ObjectRegistry.Release(secondHandle);
            ObjectRegistry.Release(problemHandle);
            delegate* unmanaged<IntPtr, void> freeString = &Exports.FreeString;
            freeString(error.Message);
            freeString(error.SourcePath);
        }
    }
}
