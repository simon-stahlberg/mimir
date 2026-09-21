using System.Runtime.InteropServices;
using Mimir.Core.Schemas;
using Mimir.Core.Grounding;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_function_count")]
    public static int DomainGetFunctionCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Functions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_function")]
    public static int DomainGetFunction(int handle, int index)
        => CreateHandle(() => RequireHandle<Domain>(handle).Functions[index]);

    [UnmanagedCallersOnly(EntryPoint = "mimir_function_get_name")]
    public static IntPtr FunctionGetName(int handle)
        => ReadValue(handle, IntPtr.Zero, (NumericFunction function) => AllocUtf8(function.Name));

    [UnmanagedCallersOnly(EntryPoint = "mimir_function_get_parameter_count")]
    public static int FunctionGetParameterCount(int handle)
        => ReadValue(handle, -1, (NumericFunction function) => function.Parameters.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_function_get_parameter")]
    public static int FunctionGetParameter(int handle, int index)
        => CreateHandle(() => RequireHandle<NumericFunction>(handle).Parameters[index]);

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_function_call")]
    public static int ProblemFunctionCall(int handle, IntPtr name, IntPtr arguments, int count)
        => CreateHandle(() => RequireHandle<Problem>(handle).FunctionCall(
            ReadRequiredUtf8(name, "name"), ReadUtf8Array(arguments, count, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_constant")]
    public static int NumericConstantCreate(double value) => CreateHandle(() => new NumericConstant(value));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_binary")]
    public static int NumericBinaryCreate(int operation, int left, int right)
        => CreateHandle(() => Enum.IsDefined((NumericOperator)operation)
            ? new NumericBinaryExpression((NumericOperator)operation, RequireHandle<NumericExpression>(left), RequireHandle<NumericExpression>(right))
            : throw new ArgumentOutOfRangeException(nameof(operation)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_kind")]
    public static int NumericKind(int handle) => ReadValue(handle, -1, (NumericExpression expression) => expression switch
    {
        NumericConstant => 0, FunctionCall => 1, NumericBinaryExpression => 2, GroundFunctionCall => 3,
        _ => throw new InvalidOperationException("Unknown numeric expression.")
    });

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_constant_value")]
    public static double NumericConstantValue(int handle)
        => ReadValue(handle, double.NaN, (NumericConstant expression) => expression.Value);

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_operator")]
    public static int NumericBinaryOperator(int handle)
        => ReadValue(handle, -1, (NumericBinaryExpression expression) => (int)expression.Operator);

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_operand")]
    public static int NumericOperand(int handle, int index) => CreateHandle(() => index switch
    {
        0 => RequireHandle<NumericBinaryExpression>(handle).Left,
        1 => RequireHandle<NumericBinaryExpression>(handle).Right,
        _ => throw new ArgumentOutOfRangeException(nameof(index))
    });

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_function")]
    public static int NumericCallFunction(int handle) => CreateHandle(() => RequireHandle<NumericExpression>(handle) switch
    {
        FunctionCall call => call.Function,
        GroundFunctionCall call => call.Function,
        _ => throw new ArgumentException("Expected a function call.")
    });

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_argument_count")]
    public static int NumericArgumentCount(int handle) => ReadValue(handle, -1, (NumericExpression expression) => expression switch
    {
        FunctionCall call => call.Arguments.Count,
        GroundFunctionCall call => call.Arguments.Count,
        _ => throw new ArgumentException("Expected a function call.")
    });

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_argument")]
    public static int NumericArgument(int handle, int index) => CreateHandle(() => RequireHandle<NumericExpression>(handle) switch
    {
        FunctionCall call => call.Arguments[index],
        GroundFunctionCall call => call.Arguments[index],
        _ => throw new ArgumentException("Expected a function call.")
    });

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_cost_expression")]
    public static int ActionCostExpressionGet(int handle)
        => CreateHandle(() => RequireHandle<ActionSchema>(handle).CostExpression);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_cost_expression")]
    public static int GroundActionCostExpressionGet(int handle)
        => CreateHandle(() => RequireHandle<GroundAction>(handle).CostExpression);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_numeric_value")]
    public static double StateNumericValue(int state, int expression)
        => ReadValue(state, double.NaN, (ExtendedState value) => value.State.Value(RequireHandle<NumericExpression>(expression)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_spec_comparison")]
    public static int NumericSpecComparison(int left, int operation, int right)
        => CreateHandle(() => Enum.IsDefined((ComparisonOperator)operation)
            ? new LogicalExpressionSpec(new ComparisonLogicalExpressionNode(
                RequireHandle<NumericExpressionSpec>(left).Node, (ComparisonOperator)operation,
                RequireHandle<NumericExpressionSpec>(right).Node))
            : throw new ArgumentOutOfRangeException(nameof(operation)));
    [UnmanagedCallersOnly(EntryPoint = "mimir_initial_state_set_value")]
    public static byte InitialStateSetValue(int builder, int target, double value)
        => Mutate<InitialStateBuilder>(builder, initial => initial.SetValue(RequireHandle<NumericFunctionSpec>(target), value));

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_lifted_function_call")]
    public static int ProblemLiftedFunctionCall(int problem, int function, IntPtr arguments, int count)
        => CreateHandle(() => RequireHandle<Problem>(problem).NewFunctionCall(
            RequireHandle<NumericFunction>(function), ReadHandleArray<ITerm>(arguments, count, "arguments")));

}
