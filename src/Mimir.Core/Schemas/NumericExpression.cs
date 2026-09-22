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
        if (!double.IsFinite(value)) throw new ArgumentOutOfRangeException(nameof(value), value, "Numeric constants must be finite.");
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
public enum NumericUpdateOperator { Assign, Increase, Decrease, ScaleUp, ScaleDown }

public record NumericComparison : IGroundedExpression
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
    internal NumericField Field { get; }
    internal InstanceContext Context { get; }
    public NumericFunction Function { get; }
    public IReadOnlyList<Constant> Arguments { get; }

    public GroundFunctionCall(Problem problem, NumericFunction function, IReadOnlyList<Constant> arguments)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(function);
        ArgumentNullException.ThrowIfNull(arguments);
        problem.ValidateNumericFunctionArguments(function, arguments);
        Context = problem.Context;
        Field = Context.NumericLayout.Resolve(function, arguments);
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
    internal static bool IsGround(NumericExpression expression) => expression switch
    {
        NumericConstant or GroundFunctionCall => true,
        NumericBinaryExpression binary => IsGround(binary.Left) && IsGround(binary.Right),
        _ => false
    };

    internal static void RequireGround(NumericExpression expression)
    {
        if (!IsGround(expression)) throw new ArgumentException("Expected a grounded numeric expression.");
    }

    internal static double Evaluate(State state, NumericExpression expression)
    {
        ArgumentNullException.ThrowIfNull(expression);
        switch (expression)
        {
            case NumericConstant constant: return constant.Value;
            case GroundFunctionCall call:
                if (!ReferenceEquals(call.Context, state.Context))
                    throw new ArgumentException("Numeric expression belongs to a different problem.");
                return call.Field.Index is NumericFluentIndex index
                    ? state.NumericValues[index.Value]
                    : call.Field.InitialValue;
            case NumericBinaryExpression binary:
                return Apply(binary.Operator, Evaluate(state, binary.Left), Evaluate(state, binary.Right));
            default: throw new ArgumentException("State.Value requires a grounded numeric expression.");
        }
    }

    internal static bool DependsOnState(NumericExpression expression, IReadOnlySet<NumericFunction> changingFunctions)
        => expression switch
        {
            NumericConstant => false,
            FunctionCall call => changingFunctions.Contains(call.Function),
            GroundFunctionCall call => changingFunctions.Contains(call.Function),
            NumericBinaryExpression binary => DependsOnState(binary.Left, changingFunctions)
                || DependsOnState(binary.Right, changingFunctions),
            _ => throw new InvalidOperationException($"Unknown numeric expression '{expression.GetType().Name}'.")
        };

    internal static bool DependsOnState(NumericComparison comparison, IReadOnlySet<NumericFunction> changingFunctions)
        => DependsOnState(comparison.Left, changingFunctions) || DependsOnState(comparison.Right, changingFunctions);

    // PDDL 2.1 leaves unassigned fluents and division by zero undefined; NaN encodes "undefined" and makes
    // every comparison and update that reads it fail, which renders the enclosing action inapplicable.
    internal const double Undefined = double.NaN;
    internal const double Epsilon = 1e-9;

    // 1e9 is exact in binary, so value * GridScale / GridScale lands on the double nearest to each decimal grid
    // point (0.3 stays 0.3). Above the limit value * GridScale would lose integer precision.
    private const double GridScale = 1e9;
    private const double QuantizationLimit = 1e6;

    internal static double Apply(NumericOperator operation, double left, double right)
    {
        double result = operation switch
        {
            NumericOperator.Add => left + right,
            NumericOperator.Subtract => left - right,
            NumericOperator.Multiply => left * right,
            NumericOperator.Divide when right == 0d => Undefined,
            NumericOperator.Divide => left / right,
            _ => throw new ArgumentException("Unknown numeric operator.")
        };
        return double.IsFinite(result) ? result : Undefined;
    }

    internal static bool Compare(ComparisonOperator operation, double left, double right)
    {
        if (double.IsNaN(left) || double.IsNaN(right)) return false;
        return operation switch
        {
            ComparisonOperator.Equal => Math.Abs(left - right) <= Epsilon,
            ComparisonOperator.LessThan => left < right - Epsilon,
            ComparisonOperator.LessThanOrEqual => left <= right + Epsilon,
            ComparisonOperator.GreaterThan => left > right + Epsilon,
            ComparisonOperator.GreaterThanOrEqual => left >= right - Epsilon,
            _ => throw new ArgumentException("Unknown comparison operator.")
        };
    }

    // Snapping state values to the epsilon grid keeps state equality and hashing exact and transitive while
    // still merging values that differ only by floating-point noise.
    internal static double Quantize(double value)
    {
        if (double.IsNaN(value)) return Undefined;
        if (double.IsInfinity(value)) throw new ArgumentException("Numeric values must be finite or undefined.");
        if (Math.Abs(value) >= QuantizationLimit) return value;
        double quantized = Math.Round(value * GridScale) / GridScale;
        return quantized == 0d ? 0d : quantized;
    }
}
