using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class LogicalExpressionSpec
{
    internal LogicalExpressionNode Node { get; }

    internal LogicalExpressionSpec(LogicalExpressionNode node)
    {
        Node = node;
    }
}

public static class Logic
{
    public static LogicalExpressionSpec True()
        => new(new TrueLogicalExpressionNode());

    public static LogicalExpressionSpec False()
        => new(new FalseLogicalExpressionNode());

    public static LogicalExpressionSpec Atom(string predicateName, params string[] arguments)
    {
        BuilderName.RequirePredicateReference(predicateName, nameof(predicateName), allowEquality: false);
        return new LogicalExpressionSpec(
            new AtomLogicalExpressionNode(
                predicateName,
                BuilderName.CopyTerms(arguments, nameof(arguments))));
    }

    public static LogicalExpressionSpec Equal(string left, string right)
        => new(new EqualityLogicalExpressionNode(
            BuilderName.RequireTerm(left, nameof(left)),
            BuilderName.RequireTerm(right, nameof(right))));

    public static LogicalExpressionSpec Not(LogicalExpressionSpec expression)
    {
        ArgumentNullException.ThrowIfNull(expression);
        return new LogicalExpressionSpec(new NotLogicalExpressionNode(expression.Node));
    }

    public static LogicalExpressionSpec And(params LogicalExpressionSpec[] expressions)
        => Junction(expressions, isConjunction: true, nameof(expressions));

    public static LogicalExpressionSpec Or(params LogicalExpressionSpec[] expressions)
        => Junction(expressions, isConjunction: false, nameof(expressions));

    public static LogicalExpressionSpec Imply(
        LogicalExpressionSpec antecedent,
        LogicalExpressionSpec consequent)
    {
        ArgumentNullException.ThrowIfNull(antecedent);
        ArgumentNullException.ThrowIfNull(consequent);
        return new LogicalExpressionSpec(
            new ImplyLogicalExpressionNode(antecedent.Node, consequent.Node));
    }

    public static LogicalExpressionSpec Exists(
        IReadOnlyList<(string Name, string Type)> parameters,
        LogicalExpressionSpec body)
        => Quantified(parameters, body, isUniversal: false);

    public static LogicalExpressionSpec Forall(
        IReadOnlyList<(string Name, string Type)> parameters,
        LogicalExpressionSpec body)
        => Quantified(parameters, body, isUniversal: true);

    private static LogicalExpressionSpec Junction(
        LogicalExpressionSpec[] expressions,
        bool isConjunction,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(expressions, parameterName);
        LogicalExpressionNode[] nodes = new LogicalExpressionNode[expressions.Length];
        for (int index = 0; index < expressions.Length; index++)
        {
            LogicalExpressionSpec expression = expressions[index]
                ?? throw new ArgumentException("Expressions cannot contain null values.", parameterName);
            nodes[index] = expression.Node;
        }

        return new LogicalExpressionSpec(isConjunction
            ? new AndLogicalExpressionNode(nodes)
            : new OrLogicalExpressionNode(nodes));
    }

    private static LogicalExpressionSpec Quantified(
        IReadOnlyList<(string Name, string Type)> parameters,
        LogicalExpressionSpec body,
        bool isUniversal)
    {
        ArgumentNullException.ThrowIfNull(parameters);
        ArgumentNullException.ThrowIfNull(body);
        BuilderParameterSpec[] parameterSpecs = BuilderName.CopyParameters(parameters, nameof(parameters));
        BuilderName.RejectDuplicateParameters(parameterSpecs, nameof(parameters));
        return new LogicalExpressionSpec(isUniversal
            ? new ForallLogicalExpressionNode(parameterSpecs, body.Node)
            : new ExistsLogicalExpressionNode(parameterSpecs, body.Node));
    }
}

public class NumericExpressionSpec
{
    public static implicit operator NumericExpressionSpec(double value) => Numeric.Constant(value);
    public static NumericExpressionSpec operator +(NumericExpressionSpec left, NumericExpressionSpec right) => Numeric.Add(left, right);
    public static NumericExpressionSpec operator -(NumericExpressionSpec left, NumericExpressionSpec right) => Numeric.Subtract(left, right);
    public static NumericExpressionSpec operator *(NumericExpressionSpec left, NumericExpressionSpec right) => Numeric.Multiply(left, right);
    public static NumericExpressionSpec operator /(NumericExpressionSpec left, NumericExpressionSpec right) => Numeric.Divide(left, right);
    public static NumericExpressionSpec operator -(NumericExpressionSpec value) => Numeric.Subtract(Numeric.Constant(0), value);
    public LogicalExpressionSpec EqualTo(NumericExpressionSpec right) => Compare(ComparisonOperator.Equal, right);
    public LogicalExpressionSpec LessThan(NumericExpressionSpec right) => Compare(ComparisonOperator.LessThan, right);
    public LogicalExpressionSpec LessThanOrEqual(NumericExpressionSpec right) => Compare(ComparisonOperator.LessThanOrEqual, right);
    public LogicalExpressionSpec GreaterThan(NumericExpressionSpec right) => Compare(ComparisonOperator.GreaterThan, right);
    public LogicalExpressionSpec GreaterThanOrEqual(NumericExpressionSpec right) => Compare(ComparisonOperator.GreaterThanOrEqual, right);
    private LogicalExpressionSpec Compare(ComparisonOperator operation, NumericExpressionSpec right)
    {
        ArgumentNullException.ThrowIfNull(right);
        return new(new ComparisonLogicalExpressionNode(Node, operation, right.Node));
    }
    internal ActionCostNode Node { get; }

    internal NumericExpressionSpec(ActionCostNode node)
    {
        Node = node;
    }
}

public sealed class NumericFunctionSpec : NumericExpressionSpec
{
    internal NumericFunctionSpec(FunctionActionCostNode node) : base(node) { }
}

public static class Numeric
{
    public static NumericExpressionSpec Constant(double value)
    {
        decimal pddlValue = BuilderName.ToPddlNumber(value, nameof(value));
        return new NumericExpressionSpec(new ConstantActionCostNode(value, pddlValue));
    }

    public static NumericFunctionSpec Function(string functionName, params string[] arguments)
    {
        BuilderName.RequireName(functionName, nameof(functionName));
        if (functionName.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
        {
            throw new ArgumentException(
                "Action costs may not depend on total-cost itself.",
                nameof(functionName));
        }

        return new NumericFunctionSpec(
            new FunctionActionCostNode(
                functionName,
                BuilderName.CopyTerms(arguments, nameof(arguments))));
    }

    public static NumericExpressionSpec Add(NumericExpressionSpec left, NumericExpressionSpec right)
        => Binary(NumericOperator.Add, left, right);

    public static NumericExpressionSpec Subtract(NumericExpressionSpec left, NumericExpressionSpec right)
        => Binary(NumericOperator.Subtract, left, right);

    public static NumericExpressionSpec Multiply(NumericExpressionSpec left, NumericExpressionSpec right)
        => Binary(NumericOperator.Multiply, left, right);

    public static NumericExpressionSpec Divide(NumericExpressionSpec left, NumericExpressionSpec right)
        => Binary(NumericOperator.Divide, left, right);

    private static NumericExpressionSpec Binary(
        NumericOperator operation,
        NumericExpressionSpec left,
        NumericExpressionSpec right)
    {
        ArgumentNullException.ThrowIfNull(left);
        ArgumentNullException.ThrowIfNull(right);
        return new NumericExpressionSpec(new BinaryActionCostNode(operation, left.Node, right.Node));
    }
}

internal static class BuilderName
{
    private static readonly HashSet<string> ReservedNames = new(StringComparer.OrdinalIgnoreCase)
    {
        "and", "or", "not", "imply", "exists", "forall", "when",
        "assign", "increase", "decrease", "scale-up", "scale-down",
        "define", "domain", "problem", "minimize", "maximize", "either",
        "preference", "always", "sometime", "within", "at-most-once",
        "sometime-after", "sometime-before", "always-within", "hold-during",
        "hold-after", "probabilistic", "oneof", "is-violated"
    };

    internal static string RequireName(string? name, string parameterName)
    {
        ArgumentNullException.ThrowIfNull(name, parameterName);
        if (!HasValidNameSyntax(name) || ReservedNames.Contains(name))
            throw new ArgumentException($"'{name}' is not a valid PDDL name.", parameterName);

        return name;
    }

    internal static string RequireVariable(string? name, string parameterName)
    {
        ArgumentNullException.ThrowIfNull(name, parameterName);
        if (name.Length < 2 || name[0] != '?' || !HasValidNameSyntax(name.AsSpan(1)))
            throw new ArgumentException($"'{name}' is not a valid PDDL variable.", parameterName);

        return name;
    }

    internal static string RequireTerm(string? term, string parameterName)
    {
        ArgumentNullException.ThrowIfNull(term, parameterName);
        return term.StartsWith('?')
            ? RequireVariable(term, parameterName)
            : RequireName(term, parameterName);
    }

    internal static string RequirePredicateReference(
        string? name,
        string parameterName,
        bool allowEquality = true)
    {
        ArgumentNullException.ThrowIfNull(name, parameterName);
        if (allowEquality && name == "=") return name;
        return RequireName(name, parameterName);
    }

    internal static string[] CopyTerms(string[]? terms, string parameterName)
    {
        ArgumentNullException.ThrowIfNull(terms, parameterName);
        string[] copy = new string[terms.Length];
        for (int index = 0; index < terms.Length; index++)
            copy[index] = RequireTerm(terms[index], parameterName);

        return copy;
    }

    internal static BuilderParameterSpec[] CopyParameters(
        IReadOnlyList<(string Name, string Type)> parameters,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(parameters, parameterName);
        var result = new BuilderParameterSpec[parameters.Count];
        for (int index = 0; index < parameters.Count; index++)
        {
            (string name, string type) = parameters[index];
            result[index] = new BuilderParameterSpec(
                RequireVariable(name, parameterName),
                RequireName(type, parameterName));
        }

        RejectDuplicateParameters(result, parameterName);
        return result;
    }

    internal static void RejectDuplicateParameters(
        IReadOnlyList<BuilderParameterSpec> parameters,
        string parameterName)
    {
        var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (BuilderParameterSpec parameter in parameters)
        {
            if (!names.Add(parameter.Name))
                throw new ArgumentException($"Parameter '{parameter.Name}' occurs more than once.", parameterName);
        }
    }

    internal static decimal ToPddlNumber(double value, string parameterName)
    {
        try
        {
            if (!double.IsFinite(value)) throw new OverflowException();
            return (decimal)value;
        }
        catch (OverflowException)
        {
            throw new ArgumentOutOfRangeException(
                parameterName,
                value,
                "The numeric value cannot be represented as a PDDL number.");
        }
    }

    private static bool HasValidNameSyntax(ReadOnlySpan<char> name)
    {
        if (name.IsEmpty || !IsAsciiLetter(name[0])) return false;
        for (int index = 1; index < name.Length; index++)
        {
            char character = name[index];
            if (!IsAsciiLetter(character) && !char.IsAsciiDigit(character) && character is not '-' and not '_')
                return false;
        }

        return true;
    }

    private static bool IsAsciiLetter(char character)
        => character is >= 'A' and <= 'Z' or >= 'a' and <= 'z';
}

internal sealed record BuilderParameterSpec(string Name, string Type);
internal sealed record BuilderTypedNameSpec(string Name, string Type);
internal sealed record BuilderPredicateSpec(string Name, IReadOnlyList<BuilderParameterSpec> Parameters);
internal sealed record BuilderFunctionSpec(string Name, IReadOnlyList<BuilderParameterSpec> Parameters);
internal sealed record BuilderLiteralSpec(
    string PredicateName,
    Polarity Polarity,
    IReadOnlyList<string> Arguments);
internal sealed record BuilderConditionalEffectSpec(
    IReadOnlyList<BuilderParameterSpec> Parameters,
    IReadOnlyList<BuilderLiteralSpec> Conditions,
    BuilderLiteralSpec Effect);
internal sealed record BuilderActionSpec(
    string Name,
    IReadOnlyList<BuilderParameterSpec> Parameters,
    IReadOnlyList<BuilderLiteralSpec> Preconditions,
    IReadOnlyList<BuilderLiteralSpec> Effects,
    IReadOnlyList<BuilderConditionalEffectSpec> ConditionalEffects,
    NumericExpressionSpec Cost,
    bool HasExplicitCost);
internal sealed record BuilderDerivedPredicateSpec(string PredicateName, LogicalExpressionSpec Body);

internal abstract record LogicalExpressionNode;
internal sealed record TrueLogicalExpressionNode : LogicalExpressionNode;
internal sealed record FalseLogicalExpressionNode : LogicalExpressionNode;
internal sealed record AtomLogicalExpressionNode(
    string PredicateName,
    IReadOnlyList<string> Arguments) : LogicalExpressionNode;
internal sealed record EqualityLogicalExpressionNode(string Left, string Right) : LogicalExpressionNode;
internal sealed record NotLogicalExpressionNode(LogicalExpressionNode Expression) : LogicalExpressionNode;
internal sealed record AndLogicalExpressionNode(
    IReadOnlyList<LogicalExpressionNode> Expressions) : LogicalExpressionNode;
internal sealed record OrLogicalExpressionNode(
    IReadOnlyList<LogicalExpressionNode> Expressions) : LogicalExpressionNode;
internal sealed record ImplyLogicalExpressionNode(
    LogicalExpressionNode Antecedent,
    LogicalExpressionNode Consequent) : LogicalExpressionNode;
internal sealed record ExistsLogicalExpressionNode(
    IReadOnlyList<BuilderParameterSpec> Parameters,
    LogicalExpressionNode Body) : LogicalExpressionNode;
internal sealed record ForallLogicalExpressionNode(
    IReadOnlyList<BuilderParameterSpec> Parameters,
    LogicalExpressionNode Body) : LogicalExpressionNode;

internal abstract record ActionCostNode;
internal sealed record ConstantActionCostNode(double Value, decimal PddlValue) : ActionCostNode;
internal sealed record FunctionActionCostNode(
    string FunctionName,
    IReadOnlyList<string> Arguments) : ActionCostNode;
internal sealed record BinaryActionCostNode(
    NumericOperator Operator,
    ActionCostNode Left,
    ActionCostNode Right) : ActionCostNode;

internal sealed record ComparisonLogicalExpressionNode(ActionCostNode Left, ComparisonOperator Operator, ActionCostNode Right) : LogicalExpressionNode;
