namespace Mimir.Core.Schemas;

using Mimir.Core;
using Mimir.Core.Grounding;

public abstract record NumericExpression
{
    public static implicit operator NumericExpression(double value) => new NumericConstant(value);
    public static NumericExpression operator +(NumericExpression left, NumericExpression right) => new NumericBinaryExpression(NumericOperator.Add, left, right);
    public static NumericExpression operator -(NumericExpression left, NumericExpression right) => new NumericBinaryExpression(NumericOperator.Subtract, left, right);
    public static NumericExpression operator *(NumericExpression left, NumericExpression right) => new NumericBinaryExpression(NumericOperator.Multiply, left, right);
    public static NumericExpression operator /(NumericExpression left, NumericExpression right) => new NumericBinaryExpression(NumericOperator.Divide, left, right);
    public static NumericExpression operator -(NumericExpression value) => new NumericBinaryExpression(NumericOperator.Subtract, new NumericConstant(0), value);

    public NumericComparison EqualTo(NumericExpression right) => new(this, ComparisonOperator.Equal, right);
    public NumericComparison LessThan(NumericExpression right) => new(this, ComparisonOperator.LessThan, right);
    public NumericComparison LessThanOrEqual(NumericExpression right) => new(this, ComparisonOperator.LessThanOrEqual, right);
    public NumericComparison GreaterThan(NumericExpression right) => new(this, ComparisonOperator.GreaterThan, right);
    public NumericComparison GreaterThanOrEqual(NumericExpression right) => new(this, ComparisonOperator.GreaterThanOrEqual, right);

    internal NumericExpression Ground(Problem problem, IReadOnlyDictionary<Variable, Constant> bindings)
        => this switch
        {
            NumericConstant => this,
            FunctionCall call => new GroundFunctionCall(problem, call.Function,
                call.Arguments.Select(argument => argument switch
                {
                    Constant constant => constant,
                    Variable variable => bindings.TryGetValue(variable, out Constant? value)
                        ? value : throw new ArgumentException($"Unbound variable '{variable.Name}'."),
                    _ => throw new ArgumentException("Unsupported numeric argument.")
                }).ToArray()),
            GroundFunctionCall call when ReferenceEquals(call.Problem, problem) => call,
            NumericBinaryExpression binary => new NumericBinaryExpression(binary.Operator,
                binary.Left.Ground(problem, bindings), binary.Right.Ground(problem, bindings)),
            _ => throw new ArgumentException("Numeric expression belongs to another problem.")
        };
}


public sealed record NumericConstant : NumericExpression
{
    public double Value { get; }
    public NumericConstant(double value)
    {
        if (!double.IsFinite(value)) throw new ArgumentOutOfRangeException(nameof(value));
        Value = value;
    }
}

public enum NumericOperator
{
    Add,
    Subtract,
    Multiply,
    Divide
}

public sealed record NumericBinaryExpression(
    NumericOperator Operator,
    NumericExpression Left,
    NumericExpression Right) : NumericExpression
{
    private NumericOperator _operator = RequireOperator(Operator);
    public NumericOperator Operator
    {
        get => _operator;
        init => _operator = RequireOperator(value);
    }

    private static NumericOperator RequireOperator(NumericOperator operation)
    {
        if (!Enum.IsDefined(operation)) throw new ArgumentOutOfRangeException(nameof(operation));
        return operation;
    }

    private NumericExpression _left = Left ?? throw new ArgumentNullException(nameof(Left));
    private NumericExpression _right = Right ?? throw new ArgumentNullException(nameof(Right));

    public NumericExpression Left
    {
        get => _left;
        init => _left = value ?? throw new ArgumentNullException(nameof(Left));
    }

    public NumericExpression Right
    {
        get => _right;
        init => _right = value ?? throw new ArgumentNullException(nameof(Right));
    }
}

public sealed record FunctionCall(
    NumericFunction Function,
    IReadOnlyList<ITerm> Arguments) : NumericExpression
{
    private NumericFunction _function = Function ?? throw new ArgumentNullException(nameof(Function));
    private IReadOnlyList<ITerm> _arguments = CopyArguments(Arguments, nameof(Arguments));

    public NumericFunction Function
    {
        get => _function;
        init => _function = value ?? throw new ArgumentNullException(nameof(Function));
    }

    public IReadOnlyList<ITerm> Arguments
    {
        get => _arguments;
        init => _arguments = CopyArguments(value, nameof(Arguments));
    }

    private static IReadOnlyList<ITerm> CopyArguments(
        IReadOnlyList<ITerm> arguments,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(arguments, parameterName);

        ITerm[] copy = arguments.ToArray();
        if (copy.Any(argument => argument is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return Array.AsReadOnly(copy);
    }

    public bool Equals(FunctionCall? other)
        => other is not null
        && ReferenceEquals(Function, other.Function)
        && ValueSequence.ReferenceEqualsItems(Arguments, other.Arguments);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(Function));
        ValueSequence.AddReferencesToHash(ref hash, Arguments);
        return hash.ToHashCode();
    }
}


public enum ComparisonOperator { Equal, LessThan, LessThanOrEqual, GreaterThan, GreaterThanOrEqual }
public enum NumericUpdateOperator { Assign, Increase, Decrease }

public record NumericComparison
{
    public NumericExpression Left { get; }
    public ComparisonOperator Operator { get; }
    public NumericExpression Right { get; }

    public NumericComparison(NumericExpression left, ComparisonOperator operation, NumericExpression right)
    {
        ArgumentNullException.ThrowIfNull(left);
        ArgumentNullException.ThrowIfNull(right);
        if (!Enum.IsDefined(operation)) throw new ArgumentOutOfRangeException(nameof(operation));
        Left = left;
        Operator = operation;
        Right = right;
    }
}

public sealed record GroundNumericComparison : NumericComparison
{
    public GroundNumericComparison(NumericExpression left, ComparisonOperator operation, NumericExpression right)
        : base(left, operation, right)
    {
        NumericEvaluation.RequireGround(left);
        NumericEvaluation.RequireGround(right);
    }
}

public record NumericUpdate
{
    public FunctionCall Target { get; }
    public NumericUpdateOperator Operator { get; }
    public NumericExpression Expression { get; }

    public NumericUpdate(FunctionCall target, NumericUpdateOperator operation, NumericExpression expression)
    {
        ArgumentNullException.ThrowIfNull(target);
        ArgumentNullException.ThrowIfNull(expression);
        if (!Enum.IsDefined(operation)) throw new ArgumentOutOfRangeException(nameof(operation));
        Target = target;
        Operator = operation;
        Expression = expression;
    }
}

public sealed record GroundNumericUpdate
{
    public GroundFunctionCall Target { get; }
    public NumericUpdateOperator Operator { get; }
    public NumericExpression Expression { get; }

    public GroundNumericUpdate(GroundFunctionCall target, NumericUpdateOperator operation, NumericExpression expression)
    {
        ArgumentNullException.ThrowIfNull(target);
        ArgumentNullException.ThrowIfNull(expression);
        if (!Enum.IsDefined(operation)) throw new ArgumentOutOfRangeException(nameof(operation));
        NumericEvaluation.RequireGround(expression);
        Target = target;
        Operator = operation;
        Expression = expression;
    }
}

public sealed record GroundFunctionCall : NumericExpression
{
    public Problem Problem { get; }
    public NumericFunction Function { get; }
    public IReadOnlyList<Constant> Arguments { get; }

    public GroundFunctionCall(Problem problem, NumericFunction function, IReadOnlyList<Constant> arguments)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(function);
        ArgumentNullException.ThrowIfNull(arguments);
        problem.ValidateNumericFunctionArguments(function, arguments);
        Problem = problem;
        Function = function;
        Arguments = Array.AsReadOnly(arguments.ToArray());
    }

    public bool Equals(GroundFunctionCall? other) => other is not null
        && ReferenceEquals(Problem, other.Problem) && ReferenceEquals(Function, other.Function)
        && ValueSequence.ReferenceEqualsItems(Arguments, other.Arguments);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(Problem);
        hash.Add(Function);
        ValueSequence.AddReferencesToHash(ref hash, Arguments);
        return hash.ToHashCode();
    }
}

internal static class NumericEvaluation
{
    internal static void RequireGround(NumericExpression expression)
    {
        switch (expression)
        {
            case NumericConstant or GroundFunctionCall: return;
            case NumericBinaryExpression binary:
                RequireGround(binary.Left);
                RequireGround(binary.Right);
                return;
            default: throw new ArgumentException("Expected a grounded numeric expression.");
        }
    }

    internal static double Evaluate(State state, NumericExpression expression)
    {
        ArgumentNullException.ThrowIfNull(expression);
        double result;
        switch (expression)
        {
            case NumericConstant constant: return constant.Value;
            case GroundFunctionCall call:
                if (!ReferenceEquals(call.Problem, state.Context.Problem))
                    throw new ArgumentException("Numeric expression belongs to a different problem.");
                return call.Problem.GetNumericFunctionValue(call.Function, call.Arguments);
            case NumericBinaryExpression binary:
                double left = Evaluate(state, binary.Left);
                double right = Evaluate(state, binary.Right);
                result = binary.Operator switch
                {
                    NumericOperator.Add => left + right,
                    NumericOperator.Subtract => left - right,
                    NumericOperator.Multiply => left * right,
                    NumericOperator.Divide when right == 0 => throw new DivideByZeroException(),
                    NumericOperator.Divide => left / right,
                    _ => throw new ArgumentException("Unknown numeric operator.")
                };
                break;
            default: throw new ArgumentException("State.Value requires a grounded numeric expression.");
        }
        if (!double.IsFinite(result)) throw new ArithmeticException("Numeric evaluation produced a non-finite value.");
        return result;
    }
}
