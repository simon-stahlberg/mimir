using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Core.Schemas;

internal static class DomainCostTranslator
{
    public static ActionCostExpression? ExtractActionCostExpression(
        IEffect? effect,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        if (effect == null)
            return null;

        var expressions = new List<ActionCostExpression>();
        CollectActionCostExpressions(effect, isConditional: false, isQuantified: false, variableScope, functionLookup, mapTerm, expressions);
        if (expressions.Count == 0)
            return null;

        var combined = expressions[0];
        for (int i = 1; i < expressions.Count; i++)
        {
            combined = new BinaryActionCostExpression(ActionCostBinaryOperator.Add, combined, expressions[i]);
        }

        return combined;
    }

    private static void CollectActionCostExpressions(
        IEffect effect,
        bool isConditional,
        bool isQuantified,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        List<ActionCostExpression> results)
    {
        switch (effect)
        {
            case Pddl.Ast.Effects.AndEffect andEffect:
                foreach (var child in andEffect.Effects)
                {
                    CollectActionCostExpressions(child, isConditional, isQuantified, variableScope, functionLookup, mapTerm, results);
                }
                break;
            case Pddl.Ast.Effects.ConditionalEffect conditionalEffect:
                CollectActionCostExpressions(
                    conditionalEffect.Effect,
                    isConditional || !LogicalExpressionSemantics.IsAlwaysTrue(conditionalEffect.Condition),
                    isQuantified,
                    variableScope,
                    functionLookup,
                    mapTerm,
                    results);
                break;
            case Pddl.Ast.Effects.ForallEffect forallEffect:
                CollectActionCostExpressions(forallEffect.Effect, isConditional, isQuantified: true, variableScope, functionLookup, mapTerm, results);
                break;
            case Pddl.Ast.Effects.Increase increase:
                if (!IsTotalCost(increase.Fluent))
                {
                    throw new NotSupportedException(
                        $"Numeric mutation of '{increase.Fluent.Name}' is not supported. Only '(increase (total-cost) <expr>)' is supported.");
                }
                if (isConditional)
                    throw new NotSupportedException("Conditional action costs are not supported.");
                if (isQuantified)
                    throw new NotSupportedException("Quantified action costs are not supported.");
                if (!increase.Fluent.Arguments.IsDefaultOrEmpty)
                    throw new NotSupportedException("The built-in total-cost function may not take arguments.");

                results.Add(TranslateActionCostExpression(increase.Value, variableScope, functionLookup, mapTerm));
                break;
            case Pddl.Ast.Effects.Assign:
            case Pddl.Ast.Effects.Decrease:
            case Pddl.Ast.Effects.ScaleUp:
            case Pddl.Ast.Effects.ScaleDown:
                throw new NotSupportedException("Action costs only support '(increase (total-cost) <expr>)'.");
            case Pddl.Ast.Effects.AddEffect:
            case Pddl.Ast.Effects.DeleteEffect:
                break;
            default:
                throw new NotSupportedException($"Unsupported effect '{effect.GetType().Name}'.");
        }
    }

    private static ActionCostExpression TranslateActionCostExpression(
        INumericExpression expression,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        return expression switch
        {
            NumberLiteral numberLiteral => new ConstantActionCostExpression((double)numberLiteral.Value),
            Negate negate => new BinaryActionCostExpression(
                ActionCostBinaryOperator.Subtract,
                new ConstantActionCostExpression(0d),
                TranslateActionCostExpression(negate.Operand, variableScope, functionLookup, mapTerm)),
            Add add => new BinaryActionCostExpression(
                ActionCostBinaryOperator.Add,
                TranslateActionCostExpression(add.Left, variableScope, functionLookup, mapTerm),
                TranslateActionCostExpression(add.Right, variableScope, functionLookup, mapTerm)),
            Subtract subtract => new BinaryActionCostExpression(
                ActionCostBinaryOperator.Subtract,
                TranslateActionCostExpression(subtract.Left, variableScope, functionLookup, mapTerm),
                TranslateActionCostExpression(subtract.Right, variableScope, functionLookup, mapTerm)),
            Multiply multiply => new BinaryActionCostExpression(
                ActionCostBinaryOperator.Multiply,
                TranslateActionCostExpression(multiply.Left, variableScope, functionLookup, mapTerm),
                TranslateActionCostExpression(multiply.Right, variableScope, functionLookup, mapTerm)),
            Divide divide => new BinaryActionCostExpression(
                ActionCostBinaryOperator.Divide,
                TranslateActionCostExpression(divide.Left, variableScope, functionLookup, mapTerm),
                TranslateActionCostExpression(divide.Right, variableScope, functionLookup, mapTerm)),
            FluentCall fluentCall => TranslateActionCostFunction(fluentCall, variableScope, functionLookup, mapTerm),
            _ => throw new NotSupportedException($"Unsupported action cost expression '{expression.GetType().Name}'.")
        };
    }

    private static ActionCostExpression TranslateActionCostFunction(
        FluentCall fluentCall,
        Dictionary<string, Variable> variableScope,
        IReadOnlyDictionary<string, NumericFunction> functionLookup,
        Func<Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        if (IsTotalCost(fluentCall))
            throw new NotSupportedException("Action costs may not depend on total-cost itself.");

        if (!functionLookup.TryGetValue(fluentCall.Name, out var function))
            throw new InvalidOperationException($"Numeric function '{fluentCall.Name}' is not declared in the domain.");

        return new NumericFunctionActionCostExpression(
            function,
            fluentCall.Arguments.Select(term => mapTerm(term, variableScope)).ToList());
    }

    private static bool IsTotalCost(FluentCall fluentCall)
        => fluentCall.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase);

    public static void CollectFluentNames(IEffect? effect, HashSet<string> fluentNames)
    {
        if (effect == null) return;
        switch (effect)
        {
            case Pddl.Ast.Effects.AndEffect a:
                foreach (var e in a.Effects) CollectFluentNames(e, fluentNames);
                break;
            case Pddl.Ast.Effects.ConditionalEffect c:
                CollectFluentNames(c.Effect, fluentNames);
                break;
            case Pddl.Ast.Effects.ForallEffect f:
                CollectFluentNames(f.Effect, fluentNames);
                break;
            case Pddl.Ast.Effects.AddEffect add:
                fluentNames.Add(add.Predicate.Name);
                break;
            case Pddl.Ast.Effects.DeleteEffect del:
                fluentNames.Add(del.Predicate.Name);
                break;
            case Pddl.Ast.Effects.Assign:
            case Pddl.Ast.Effects.Increase:
            case Pddl.Ast.Effects.Decrease:
            case Pddl.Ast.Effects.ScaleUp:
            case Pddl.Ast.Effects.ScaleDown:
                break;
            default:
                throw new NotSupportedException($"Unsupported effect '{effect.GetType().Name}'.");
        }
    }
}
