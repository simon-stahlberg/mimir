using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

internal static class CorePddlSupportValidator
{
    public static void ValidateDomain(DomainDefinition domain)
    {
        ArgumentNullException.ThrowIfNull(domain);

        bool actionCostsEnabled = domain.Requirements.HasRequirement(PddlRequirement.ActionCosts);
        foreach (ActionDefinition action in domain.Actions)
        {
            if (action.Precondition is not null)
            {
                ValidateLogicalExpression(
                    action.Precondition,
                    $"precondition of action '{action.Name}'");
            }

            if (action.Effect is not null)
            {
                ValidateEffect(
                    action.Effect,
                    actionCostsEnabled,
                    isConditional: false,
                    isQuantified: false,
                    $"effect of action '{action.Name}'");
            }
        }

        foreach (DerivedPredicate derivedPredicate in domain.DerivedPredicates)
        {
            ValidateLogicalExpression(
                derivedPredicate.Body,
                $"definition of derived predicate '{derivedPredicate.Signature.Name}'");
        }
    }

    public static void ValidateProblem(DomainDefinition domain, ProblemDefinition problem)
    {
        ArgumentNullException.ThrowIfNull(domain);
        ArgumentNullException.ThrowIfNull(problem);

        bool actionCostsEnabled = domain.Requirements.HasRequirement(PddlRequirement.ActionCosts);
        bool totalCostInitialized = false;
        foreach (IProblemInitElement element in problem.Init)
        {
            if (element is not NumericInitialization numericInitialization
                || !IsTotalCost(numericInitialization.Fluent))
            {
                continue;
            }

            ValidateTotalCostInitialization(
                numericInitialization,
                actionCostsEnabled,
                totalCostInitialized);
            totalCostInitialized = true;
        }

        ValidateGoal(problem.Goal);

        if (problem.Metric is not null)
        {
            ValidateMetric(problem.Metric, actionCostsEnabled);
        }
    }

    private static void ValidateLogicalExpression(ILogicalExpression expression, string context)
    {
        switch (expression)
        {
            case And andExpression:
                foreach (ILogicalExpression child in andExpression.Expressions)
                {
                    ValidateLogicalExpression(child, context);
                }
                return;
            case Or orExpression:
                foreach (ILogicalExpression child in orExpression.Expressions)
                {
                    ValidateLogicalExpression(child, context);
                }
                return;
            case Not notExpression:
                ValidateLogicalExpression(notExpression.Expression, context);
                return;
            case Imply implyExpression:
                ValidateLogicalExpression(implyExpression.Antecedent, context);
                ValidateLogicalExpression(implyExpression.Consequent, context);
                return;
            case Forall forallExpression:
                ValidateLogicalExpression(forallExpression.Body, context);
                return;
            case Exists existsExpression:
                ValidateLogicalExpression(existsExpression.Body, context);
                return;
            case Comparison:
                throw new NotSupportedException($"Numeric comparisons are not supported in the {context}.");
            case PredicateCall:
            case Equality:
            case EmptyLogic:
                return;
            case AmbiguousEquality:
                throw new InvalidOperationException(
                    $"Validated PDDL contains an unresolved ambiguous equality in the {context}.");
            default:
                throw new NotSupportedException(
                    $"Logical expression '{expression.GetType().Name}' is not supported in the {context}.");
        }
    }

    private static void ValidateEffect(
        IEffect effect,
        bool actionCostsEnabled,
        bool isConditional,
        bool isQuantified,
        string context)
    {
        switch (effect)
        {
            case AndEffect andEffect:
                foreach (IEffect child in andEffect.Effects)
                {
                    ValidateEffect(
                        child,
                        actionCostsEnabled,
                        isConditional,
                        isQuantified,
                        context);
                }
                return;
            case Mimir.Pddl.Ast.Effects.ConditionalEffect conditionalEffect:
                ValidateLogicalExpression(conditionalEffect.Condition, $"condition in the {context}");
                ValidateEffect(
                    conditionalEffect.Effect,
                    actionCostsEnabled,
                    isConditional || !LogicalExpressionSemantics.IsAlwaysTrue(conditionalEffect.Condition),
                    isQuantified,
                    context);
                return;
            case ForallEffect forallEffect:
                ValidateEffect(
                    forallEffect.Effect,
                    actionCostsEnabled,
                    isConditional,
                    isQuantified: true,
                    context);
                return;
            case Increase increase:
                ValidateTotalCostIncrease(
                    increase,
                    actionCostsEnabled,
                    isConditional,
                    isQuantified,
                    context);
                return;
            case Assign assign:
                ThrowUnsupportedNumericMutation("assign", assign.Fluent.Name, context);
                return;
            case Decrease decrease:
                ThrowUnsupportedNumericMutation("decrease", decrease.Fluent.Name, context);
                return;
            case ScaleUp scaleUp:
                ThrowUnsupportedNumericMutation("scale-up", scaleUp.Fluent.Name, context);
                return;
            case ScaleDown scaleDown:
                ThrowUnsupportedNumericMutation("scale-down", scaleDown.Fluent.Name, context);
                return;
            case AddEffect:
            case DeleteEffect:
                return;
            default:
                throw new NotSupportedException(
                    $"Effect '{effect.GetType().Name}' is not supported in the {context}.");
        }
    }

    private static void ValidateTotalCostIncrease(
        Increase increase,
        bool actionCostsEnabled,
        bool isConditional,
        bool isQuantified,
        string context)
    {
        if (!IsTotalCost(increase.Fluent))
        {
            ThrowUnsupportedNumericMutation("increase", increase.Fluent.Name, context);
        }
        if (!actionCostsEnabled)
        {
            throw new NotSupportedException(
                "Increasing total-cost requires the :action-costs requirement.");
        }
        if (!increase.Fluent.Arguments.IsDefaultOrEmpty)
        {
            throw new NotSupportedException("The built-in total-cost function may not take arguments.");
        }
        if (isConditional)
        {
            throw new NotSupportedException("Conditional action costs are not supported.");
        }
        if (isQuantified)
        {
            throw new NotSupportedException("Quantified action costs are not supported.");
        }

        ValidateActionCostExpression(increase.Value);
    }

    private static void ValidateActionCostExpression(INumericExpression expression)
    {
        switch (expression)
        {
            case NumberLiteral:
                return;
            case FluentCall fluentCall:
                if (IsTotalCost(fluentCall))
                {
                    throw new NotSupportedException("Action costs may not depend on total-cost itself.");
                }
                return;
            case Negate negate:
                ValidateActionCostExpression(negate.Operand);
                return;
            case Add add:
                ValidateActionCostExpression(add.Left);
                ValidateActionCostExpression(add.Right);
                return;
            case Subtract subtract:
                ValidateActionCostExpression(subtract.Left);
                ValidateActionCostExpression(subtract.Right);
                return;
            case Multiply multiply:
                ValidateActionCostExpression(multiply.Left);
                ValidateActionCostExpression(multiply.Right);
                return;
            case Divide divide:
                ValidateActionCostExpression(divide.Left);
                ValidateActionCostExpression(divide.Right);
                return;
            default:
                throw new NotSupportedException(
                    $"Action cost expression '{expression.GetType().Name}' is not supported.");
        }
    }

    private static void ValidateTotalCostInitialization(
        NumericInitialization initialization,
        bool actionCostsEnabled,
        bool alreadyInitialized)
    {
        if (!actionCostsEnabled)
        {
            throw new NotSupportedException(
                "Initializing total-cost requires the :action-costs requirement.");
        }
        if (!initialization.Fluent.Arguments.IsDefaultOrEmpty)
        {
            throw new NotSupportedException("The built-in total-cost function may not take arguments.");
        }
        if (initialization.Value.Value != 0m)
        {
            throw new NotSupportedException("Only an initial total-cost of 0 is supported.");
        }
        if (alreadyInitialized)
        {
            throw new NotSupportedException("The built-in total-cost function may only be initialized once.");
        }
    }

    private static void ValidateMetric(Metric metric, bool actionCostsEnabled)
    {
        if (!actionCostsEnabled)
        {
            throw new NotSupportedException(
                "A total-cost metric requires the :action-costs requirement.");
        }
        if (metric.Direction != MetricDirection.Minimize
            || metric.Expression is not FluentCall fluentCall
            || !IsTotalCost(fluentCall)
            || !fluentCall.Arguments.IsDefaultOrEmpty)
        {
            throw new NotSupportedException(
                "Core only supports the metric 'minimize (total-cost)'.");
        }
    }

    private static void ValidateGoal(ILogicalExpression goal)
    {
        switch (goal)
        {
            case EmptyLogic:
            case PredicateCall:
            case Equality:
                return;
            case And andExpression:
                foreach (ILogicalExpression child in andExpression.Expressions)
                {
                    ValidateGoal(child);
                }
                return;
            case Not { Expression: PredicateCall }:
            case Not { Expression: Equality }:
                return;
            default:
                throw new NotSupportedException(
                    "A Core problem goal must be a conjunction of positive or negative predicate literals.");
        }
    }

    private static void ThrowUnsupportedNumericMutation(
        string operation,
        string fluentName,
        string context)
    {
        throw new NotSupportedException(
            $"Numeric mutation '({operation} ({fluentName}) ...)' is not supported in the {context}. "
            + "Only '(increase (total-cost) <expr>)' is supported.");
    }

    private static bool IsTotalCost(FluentCall fluentCall)
        => fluentCall.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase);
}
