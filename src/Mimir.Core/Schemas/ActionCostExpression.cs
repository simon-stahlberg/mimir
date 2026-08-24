namespace Mimir.Core.Schemas;

using Mimir.Core;

public abstract record ActionCostExpression;

public sealed record ConstantActionCostExpression(double Value) : ActionCostExpression;

public enum ActionCostBinaryOperator
{
    Add,
    Subtract,
    Multiply,
    Divide
}

public sealed record BinaryActionCostExpression(
    ActionCostBinaryOperator Operator,
    ActionCostExpression Left,
    ActionCostExpression Right) : ActionCostExpression
{
    private ActionCostExpression _left = Left ?? throw new ArgumentNullException(nameof(Left));
    private ActionCostExpression _right = Right ?? throw new ArgumentNullException(nameof(Right));

    public ActionCostExpression Left
    {
        get => _left;
        init => _left = value ?? throw new ArgumentNullException(nameof(Left));
    }

    public ActionCostExpression Right
    {
        get => _right;
        init => _right = value ?? throw new ArgumentNullException(nameof(Right));
    }
}

public sealed record NumericFunctionActionCostExpression(
    NumericFunction Function,
    IReadOnlyList<ITerm> Arguments) : ActionCostExpression
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

    public bool Equals(NumericFunctionActionCostExpression? other)
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
