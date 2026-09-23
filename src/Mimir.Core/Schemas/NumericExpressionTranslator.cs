using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Core.Schemas;

internal static class NumericExpressionTranslator
{
    internal static NumericComparison TranslateComparison(
        Comparison comparison,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
        => new(
            TranslateExpression(comparison.Left, variableScope, functionLookup, mapTerm),
            comparison.Operator switch
            {
                Pddl.Ast.Expressions.ComparisonOperator.Equal => ComparisonOperator.Equal,
                Pddl.Ast.Expressions.ComparisonOperator.LessThan => ComparisonOperator.LessThan,
                Pddl.Ast.Expressions.ComparisonOperator.LessThanOrEqual => ComparisonOperator.LessThanOrEqual,
                Pddl.Ast.Expressions.ComparisonOperator.GreaterThan => ComparisonOperator.GreaterThan,
                Pddl.Ast.Expressions.ComparisonOperator.GreaterThanOrEqual => ComparisonOperator.GreaterThanOrEqual,
                _ => throw new NotSupportedException($"Unsupported comparison operator '{comparison.Operator}'.")
            },
            TranslateExpression(comparison.Right, variableScope, functionLookup, mapTerm));

    internal static NumericExpression TranslateExpression(
        INumericExpression expression,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        return expression switch
        {
            ProgrammaticNumberLiteral literal => new NumericConstant(literal.RuntimeValue),
            NumberLiteral numberLiteral => new NumericConstant((double)numberLiteral.Value),
            Negate negate => new NumericBinaryExpression(
                NumericOperator.Subtract,
                new NumericConstant(0d),
                TranslateExpression(negate.Operand, variableScope, functionLookup, mapTerm)),
            Add add => new NumericBinaryExpression(
                NumericOperator.Add,
                TranslateExpression(add.Left, variableScope, functionLookup, mapTerm),
                TranslateExpression(add.Right, variableScope, functionLookup, mapTerm)),
            Subtract subtract => new NumericBinaryExpression(
                NumericOperator.Subtract,
                TranslateExpression(subtract.Left, variableScope, functionLookup, mapTerm),
                TranslateExpression(subtract.Right, variableScope, functionLookup, mapTerm)),
            Multiply multiply => new NumericBinaryExpression(
                NumericOperator.Multiply,
                TranslateExpression(multiply.Left, variableScope, functionLookup, mapTerm),
                TranslateExpression(multiply.Right, variableScope, functionLookup, mapTerm)),
            Divide divide => new NumericBinaryExpression(
                NumericOperator.Divide,
                TranslateExpression(divide.Left, variableScope, functionLookup, mapTerm),
                TranslateExpression(divide.Right, variableScope, functionLookup, mapTerm)),
            FluentCall fluentCall => TranslateFunctionCall(fluentCall, variableScope, functionLookup, mapTerm),
            _ => throw new NotSupportedException($"Unsupported numeric expression '{expression.GetType().Name}'.")
        };
    }

    private static NumericExpression TranslateFunctionCall(
        FluentCall fluentCall,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        if (IsTotalCost(fluentCall))
            throw new NotSupportedException("total-cost can only be increased by action effects; it cannot be read.");

        if (!functionLookup.TryGetValue(fluentCall.Name, out var function))
            throw new InvalidOperationException($"Numeric function '{fluentCall.Name}' is not declared in the domain.");

        return new FunctionCall(
            function,
            fluentCall.Arguments.Select(term => mapTerm(term, variableScope)).ToList());
    }

    private static bool IsTotalCost(FluentCall fluentCall)
        => NumericFunction.IsTotalCost(fluentCall.Name);
}
