using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

internal sealed class NumericExpressionTranslator(
    IReadOnlyDictionary<string, NumericFunction> functions,
    Func<Term, Dictionary<string, Variable>, ITerm> mapTerm)
{
    public NumericComparison TranslateComparison(Comparison comparison, Dictionary<string, Variable> scope)
        => new(
            TranslateExpression(comparison.Left, scope),
            comparison.Operator switch
            {
                Pddl.Ast.Expressions.ComparisonOperator.Equal => ComparisonOperator.Equal,
                Pddl.Ast.Expressions.ComparisonOperator.LessThan => ComparisonOperator.LessThan,
                Pddl.Ast.Expressions.ComparisonOperator.LessThanOrEqual => ComparisonOperator.LessThanOrEqual,
                Pddl.Ast.Expressions.ComparisonOperator.GreaterThan => ComparisonOperator.GreaterThan,
                Pddl.Ast.Expressions.ComparisonOperator.GreaterThanOrEqual => ComparisonOperator.GreaterThanOrEqual,
                _ => throw new NotSupportedException($"Unsupported comparison operator '{comparison.Operator}'.")
            },
            TranslateExpression(comparison.Right, scope));

    public NumericExpression TranslateExpression(INumericExpression expression, Dictionary<string, Variable> scope)
    {
        return expression switch
        {
            ProgrammaticNumberLiteral literal => new NumericConstant(literal.RuntimeValue),
            NumberLiteral numberLiteral => new NumericConstant((double)numberLiteral.Value),
            Negate negate => new NumericBinaryExpression(
                NumericOperator.Subtract, new NumericConstant(0d), TranslateExpression(negate.Operand, scope)),
            Add add => Binary(NumericOperator.Add, add.Left, add.Right, scope),
            Subtract subtract => Binary(NumericOperator.Subtract, subtract.Left, subtract.Right, scope),
            Multiply multiply => Binary(NumericOperator.Multiply, multiply.Left, multiply.Right, scope),
            Divide divide => Binary(NumericOperator.Divide, divide.Left, divide.Right, scope),
            FluentCall fluentCall => TranslateFunctionCall(fluentCall, scope),
            _ => throw new NotSupportedException($"Unsupported numeric expression '{expression.GetType().Name}'.")
        };
    }

    private NumericBinaryExpression Binary(
        NumericOperator operation,
        INumericExpression left,
        INumericExpression right,
        Dictionary<string, Variable> scope)
        => new(operation, TranslateExpression(left, scope), TranslateExpression(right, scope));

    private FunctionCall TranslateFunctionCall(FluentCall fluentCall, Dictionary<string, Variable> scope)
    {
        if (NumericFunction.IsTotalCost(fluentCall.Name))
            throw new NotSupportedException("total-cost can only be increased by action effects; it cannot be read.");

        if (!functions.TryGetValue(fluentCall.Name, out NumericFunction? function))
            throw new InvalidOperationException($"Numeric function '{fluentCall.Name}' is not declared in the domain.");

        return new FunctionCall(function, fluentCall.Arguments.Select(term => mapTerm(term, scope)).ToList());
    }
}
