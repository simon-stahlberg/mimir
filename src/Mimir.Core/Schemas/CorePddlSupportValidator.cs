using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

internal static class CorePddlSupportValidator
{
    // The canonicalizer simplifies structurally true conditions such as (and (and)) or (= ?x ?x) away, which would
    // hide a guarded total-cost increase; validating the source domain first treats every `when` as a condition.
    public static void ValidateSourceDomain(DomainDefinition domain)
        => ValidateDomain(domain, everyWhenIsConditional: true);

    public static void ValidateDomain(DomainDefinition domain)
        => ValidateDomain(domain, everyWhenIsConditional: false);

    private static void ValidateDomain(DomainDefinition domain, bool everyWhenIsConditional)
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
                    everyWhenIsConditional,
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
            case Comparison comparison:
                ValidateNumericExpression(comparison.Left);
                ValidateNumericExpression(comparison.Right);
                return;
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
        bool everyWhenIsConditional,
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
                        everyWhenIsConditional,
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
                    everyWhenIsConditional,
                    actionCostsEnabled,
                    isConditional || everyWhenIsConditional || !LogicalExpressionSemantics.IsAlwaysTrue(conditionalEffect.Condition),
                    isQuantified,
                    context);
                return;
            case ForallEffect forallEffect:
                ValidateEffect(
                    forallEffect.Effect,
                    everyWhenIsConditional,
                    actionCostsEnabled,
                    isConditional,
                    isQuantified: true,
                    context);
                return;
            case Increase increase when IsTotalCost(increase.Fluent):
                ValidateTotalCostIncrease(increase, actionCostsEnabled, isConditional, isQuantified);
                return;
            case Increase increase:
                ValidateNumericUpdate(increase.Fluent, increase.Value);
                return;
            case Assign assign:
                ValidateNumericUpdate(assign.Fluent, assign.Value);
                return;
            case Decrease decrease:
                ValidateNumericUpdate(decrease.Fluent, decrease.Value);
                return;
            case ScaleUp scaleUp:
                ValidateNumericUpdate(scaleUp.Fluent, scaleUp.Value);
                return;
            case ScaleDown scaleDown:
                ValidateNumericUpdate(scaleDown.Fluent, scaleDown.Value);
                return;
            case AddEffect:
            case DeleteEffect:
                return;
            default:
                throw new NotSupportedException(
                    $"Effect '{effect.GetType().Name}' is not supported in the {context}.");
        }
    }

    private static void ValidateNumericUpdate(FluentCall fluent, INumericExpression value)
    {
        if (IsTotalCost(fluent))
            throw new NotSupportedException("Only increase is supported for total-cost.");
        ValidateNumericExpression(value);
    }

    private static void ValidateTotalCostIncrease(
        Increase increase,
        bool actionCostsEnabled,
        bool isConditional,
        bool isQuantified)
    {
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

        ValidateNumericExpression(increase.Value);
    }

    private static void ValidateNumericExpression(INumericExpression expression)
    {
        switch (expression)
        {
            case NumberLiteral:
                return;
            case FluentCall fluentCall:
                if (IsTotalCost(fluentCall))
                {
                    throw new NotSupportedException("total-cost can only be increased by action effects; it cannot be read.");
                }
                return;
            case Negate negate:
                ValidateNumericExpression(negate.Operand);
                return;
            case Add add:
                ValidateNumericExpression(add.Left);
                ValidateNumericExpression(add.Right);
                return;
            case Subtract subtract:
                ValidateNumericExpression(subtract.Left);
                ValidateNumericExpression(subtract.Right);
                return;
            case Multiply multiply:
                ValidateNumericExpression(multiply.Left);
                ValidateNumericExpression(multiply.Right);
                return;
            case Divide divide:
                ValidateNumericExpression(divide.Left);
                ValidateNumericExpression(divide.Right);
                return;
            default:
                throw new NotSupportedException(
                    $"Numeric expression '{expression.GetType().Name}' is not supported.");
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
            case Comparison comparison:
                ValidateNumericExpression(comparison.Left);
                ValidateNumericExpression(comparison.Right);
                return;
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
                    "A Core problem goal must be a conjunction of predicate literals and numeric comparisons.");
        }
    }

    private static bool IsTotalCost(FluentCall fluentCall)
        => NumericFunction.IsTotalCost(fluentCall.Name);
}
