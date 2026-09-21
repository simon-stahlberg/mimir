using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using PddlConditionalEffect = Mimir.Pddl.Ast.Effects.ConditionalEffect;
using PddlTerm = Mimir.Pddl.Ast.Models.Term;

namespace Mimir.Core.Schemas;

internal sealed class ProgrammaticDomainDefinitionBuilder
{
    private readonly Domain _domain;
    private readonly ImmutableArray<PddlRequirement> _requirements;
    private readonly bool _actionCostsEnabled;

    private ProgrammaticDomainDefinitionBuilder(Domain domain)
    {
        _domain = domain;
        ImmutableArray<PddlRequirement>.Builder requirements = ImmutableArray.CreateBuilder<PddlRequirement>();
        foreach (string requirement in domain.Requirements)
        {
            if (string.Equals(requirement, ":quantified-preconditions", StringComparison.OrdinalIgnoreCase))
            {
                requirements.Add(PddlRequirement.ExistentialPreconditions);
                requirements.Add(PddlRequirement.UniversalPreconditions);
                continue;
            }

            requirements.Add(ParseRequirement(requirement));
        }

        _requirements = requirements.ToImmutable();
        _actionCostsEnabled = _requirements.HasRequirement(PddlRequirement.ActionCosts);
    }

    public static DomainDefinition Build(Domain domain)
        => new ProgrammaticDomainDefinitionBuilder(
            domain ?? throw new ArgumentNullException(nameof(domain)))
            .BuildDefinition();

    private DomainDefinition BuildDefinition()
    {
        IEnumerable<Predicate> runtimePredicates = _domain.Fluents.Cast<Predicate>()
            .Concat(_domain.Statics)
            .Concat(_domain.Derived)
            .Where(predicate => !_domain.IsEqualityPredicate(predicate));

        return new DomainDefinition(
            _domain.Name,
            _requirements,
            _domain.TypeHierarchy
                .Select(type => new TypeDeclaration(type.Key, type.Value))
                .ToImmutableArray(),
            _domain.Constants
                .Select(constant => new TypeDeclaration(constant.Name, constant.Type))
                .ToImmutableArray(),
            runtimePredicates
                .Select(BuildPredicateDeclaration)
                .ToImmutableArray(),
            _domain.Functions
                .Select(function => new FunctionDeclaration(
                    function.Name,
                    BuildParameters(function.Parameters)))
                .ToImmutableArray(),
            _domain.Actions
                .Select(BuildAction)
                .ToImmutableArray(),
            _domain.Derived
                .Select(BuildDerivedPredicate)
                .ToImmutableArray());
    }

    private ActionDefinition BuildAction(ActionSchema action)
    {
        ArgumentNullException.ThrowIfNull(action);
        if (!_actionCostsEnabled && action.CostExpression is not NumericConstant { Value: 1d })
        {
            throw new ArgumentException(
                $"Action '{action.Name}' has a non-default cost but the domain does not require :action-costs.",
                nameof(action));
        }

        ValidateActionCostExpression(action.CostExpression, action.Parameters);
        HashSet<Variable> actionScope = CreateVariableScope(action.Parameters);

        ILogicalExpression precondition = BuildCondition(
            action.FluentPreconditions,
            action.StaticPreconditions,
            action.DerivedPreconditions,
            actionScope);
        ImmutableArray<IEffect>.Builder effects = ImmutableArray.CreateBuilder<IEffect>(
            action.Effects.Count + (_actionCostsEnabled ? 1 : 0));

        foreach (ConditionalEffect effect in action.Effects)
            effects.Add(BuildEffect(effect, actionScope));

        if (_actionCostsEnabled)
        {
            effects.Add(new PddlConditionalEffect(
                new EmptyLogic(),
                new Increase(
                    new FluentCall("total-cost", ImmutableArray<PddlTerm>.Empty),
                    BuildActionCostExpression(action.CostExpression, actionScope))));
        }
        return new ActionDefinition(
            action.Name,
            BuildParameters(action.Parameters),
            precondition,
            new AndEffect(effects.ToImmutable()));
    }

    private DerivedPredicate BuildDerivedPredicate(Predicate<Derived> predicate)
    {
        if (!_domain.DerivedDefinitions.TryGetValue(predicate.Name, out IGroundedExpression? body))
        {
            throw new InvalidOperationException(
                $"Derived predicate '{predicate.Name}' has no definition.");
        }

        return new DerivedPredicate(
            BuildPredicateDeclaration(predicate),
            BuildExpression(body, CreateVariableScope(predicate.Parameters)));
    }

    private static PredicateDeclaration BuildPredicateDeclaration(Predicate predicate)
        => new(
            predicate.Name,
            BuildParameters(predicate.Parameters));

    private ILogicalExpression BuildExpression(
        IGroundedExpression expression,
        IReadOnlySet<Variable> variableScope)
    {
        ArgumentNullException.ThrowIfNull(expression);

        return expression switch
        {
            GroundedTrue => new EmptyLogic(),
            GroundedAtom atom => BuildAtomExpression(atom.Predicate, atom.Arguments, variableScope),
            GroundedNot not => new Not(BuildExpression(not.Expression, variableScope)),
            GroundedAnd and => new And(and.Expressions
                .Select(child => BuildExpression(child, variableScope))
                .ToImmutableArray()),
            GroundedOr or => new Or(or.Expressions
                .Select(child => BuildExpression(child, variableScope))
                .ToImmutableArray()),
            GroundedImply imply => new Imply(
                BuildExpression(imply.Antecedent, variableScope),
                BuildExpression(imply.Consequent, variableScope)),
            GroundedForall forall => new Forall(
                BuildParameters(forall.Variables),
                BuildExpression(forall.Body, ExtendVariableScope(variableScope, forall.Variables))),
            GroundedExists exists => new Exists(
                BuildParameters(exists.Variables),
                BuildExpression(exists.Body, ExtendVariableScope(variableScope, exists.Variables))),
            _ => throw new InvalidOperationException(
                $"Unsupported grounded expression '{expression.GetType().Name}'.")
        };
    }

    private ILogicalExpression BuildAtomExpression(
        Predicate predicate,
        IReadOnlyList<ITerm> arguments,
        IReadOnlySet<Variable> variableScope)
    {
        if (_domain.IsEqualityPredicate(predicate))
        {
            if (arguments.Count != 2)
                throw new InvalidOperationException("The built-in equality predicate must have two arguments.");
            return new Equality(
                BuildTerm(arguments[0], variableScope),
                BuildTerm(arguments[1], variableScope));
        }

        return BuildPredicateCall(predicate, arguments, variableScope);
    }

    private PredicateCall BuildPredicateCall(
        Predicate predicate,
        IReadOnlyList<ITerm> arguments,
        IReadOnlySet<Variable> variableScope)
    {
        ArgumentNullException.ThrowIfNull(predicate);
        ArgumentNullException.ThrowIfNull(arguments);
        ValidatePredicateIdentity(predicate);

        return new PredicateCall(
            predicate.Name,
            arguments.Select(term => BuildTerm(term, variableScope)).ToImmutableArray());
    }

    private ILogicalExpression BuildCondition(
        IReadOnlyList<Literal<Atom<Fluent>>> fluentLiterals,
        IReadOnlyList<Literal<Atom<Static>>> staticLiterals,
        IReadOnlyList<Literal<Atom<Derived>>> derivedLiterals,
        IReadOnlySet<Variable> variableScope)
    {
        var expressions = new List<ILogicalExpression>(
            fluentLiterals.Count + staticLiterals.Count + derivedLiterals.Count);

        AddLiterals(fluentLiterals, expressions, variableScope);
        AddLiterals(staticLiterals, expressions, variableScope);
        AddLiterals(derivedLiterals, expressions, variableScope);

        return expressions.Count switch
        {
            0 => new EmptyLogic(),
            1 => expressions[0],
            _ => new And(expressions.ToImmutableArray())
        };
    }

    private void AddLiterals<T>(
        IReadOnlyList<Literal<Atom<T>>> literals,
        List<ILogicalExpression> expressions,
        IReadOnlySet<Variable> variableScope)
        where T : notnull, IPredicateType
    {
        foreach (Literal<Atom<T>> literal in literals)
        {
            ILogicalExpression atomExpression = BuildAtomExpression(
                literal.Value.Predicate,
                literal.Value.Arguments,
                variableScope);
            expressions.Add(literal.Polarity switch
            {
                Polarity.Positive => atomExpression,
                Polarity.Negative => new Not(atomExpression),
                _ => throw new InvalidOperationException(
                    $"Unsupported literal polarity '{literal.Polarity}'.")
            });
        }
    }

    private IEffect BuildEffect(
        ConditionalEffect effect,
        IReadOnlySet<Variable> actionScope)
    {
        ArgumentNullException.ThrowIfNull(effect);

        HashSet<Variable> effectScope = ExtendVariableScope(actionScope, effect.QuantifiedVariables);
        ILogicalExpression condition = BuildCondition(
            effect.FluentConditions,
            effect.StaticConditions,
            effect.DerivedConditions,
            effectScope);
        IEffect literalEffect = effect.EffectLiteral.Polarity switch
        {
            Polarity.Positive => new AddEffect(BuildPredicateCall(
                effect.EffectLiteral.Value.Predicate,
                effect.EffectLiteral.Value.Arguments,
                effectScope)),
            Polarity.Negative => new DeleteEffect(BuildPredicateCall(
                effect.EffectLiteral.Value.Predicate,
                effect.EffectLiteral.Value.Arguments,
                effectScope)),
            _ => throw new InvalidOperationException(
                $"Unsupported effect polarity '{effect.EffectLiteral.Polarity}'.")
        };

        if (effect.QuantifiedVariables.Count == 0)
            return new PddlConditionalEffect(condition, literalEffect);

        IEffect quantifiedEffect = LogicalExpressionSemantics.IsAlwaysTrue(condition)
            ? literalEffect
            : new PddlConditionalEffect(condition, literalEffect);

        return new PddlConditionalEffect(
            new EmptyLogic(),
            new ForallEffect(
                BuildParameters(effect.QuantifiedVariables),
                quantifiedEffect));
    }

    private INumericExpression BuildActionCostExpression(
        NumericExpression expression,
        IReadOnlySet<Variable> variableScope)
    {
        ArgumentNullException.ThrowIfNull(expression);

        return expression switch
        {
            NumericConstant constant => BuildNumberLiteral(constant.Value),
            NumericBinaryExpression binary => binary.Operator switch
            {
                NumericOperator.Add => new Add(
                    BuildActionCostExpression(binary.Left, variableScope),
                    BuildActionCostExpression(binary.Right, variableScope)),
                NumericOperator.Subtract => new Subtract(
                    BuildActionCostExpression(binary.Left, variableScope),
                    BuildActionCostExpression(binary.Right, variableScope)),
                NumericOperator.Multiply => new Multiply(
                    BuildActionCostExpression(binary.Left, variableScope),
                    BuildActionCostExpression(binary.Right, variableScope)),
                NumericOperator.Divide => new Divide(
                    BuildActionCostExpression(binary.Left, variableScope),
                    BuildActionCostExpression(binary.Right, variableScope)),
                _ => throw new InvalidOperationException(
                    $"Unsupported action cost operator '{binary.Operator}'.")
            },
            FunctionCall function => BuildNumericFunctionCall(function, variableScope),
            _ => throw new InvalidOperationException(
                $"Unsupported action cost expression '{expression.GetType().Name}'.")
        };
    }

    private FluentCall BuildNumericFunctionCall(
        FunctionCall expression,
        IReadOnlySet<Variable> variableScope)
    {
        if (expression.Function == null)
            throw new ArgumentException("Action cost numeric function cannot be null.", nameof(expression));

        return new FluentCall(
            expression.Function.Name,
            expression.Arguments.Select(term => BuildTerm(term, variableScope)).ToImmutableArray());
    }

    private static NumberLiteral BuildNumberLiteral(double value)
    {
        try
        {
            return new NumberLiteral((decimal)value);
        }
        catch (OverflowException exception)
        {
            throw new ArgumentException(
                $"Action cost '{value}' cannot be represented as a PDDL number.",
                nameof(value),
                exception);
        }
    }

    private void ValidateActionCostExpression(
        NumericExpression expression,
        IReadOnlyList<Variable> actionParameters)
    {
        ArgumentNullException.ThrowIfNull(expression);

        switch (expression)
        {
            case NumericConstant constant:
                BuildNumberLiteral(constant.Value);
                return;
            case NumericBinaryExpression binary:
                if (!Enum.IsDefined(binary.Operator))
                {
                    throw new InvalidOperationException(
                        $"Unsupported action cost operator '{binary.Operator}'.");
                }
                ValidateActionCostExpression(binary.Left, actionParameters);
                ValidateActionCostExpression(binary.Right, actionParameters);
                return;
            case FunctionCall functionExpression:
                ValidateNumericFunctionCost(functionExpression, actionParameters);
                return;
            default:
                throw new InvalidOperationException(
                    $"Unsupported action cost expression '{expression.GetType().Name}'.");
        }
    }

    private void ValidateNumericFunctionCost(
        FunctionCall expression,
        IReadOnlyList<Variable> actionParameters)
    {
        if (expression.Function == null)
            throw new ArgumentException("Action cost numeric function cannot be null.", nameof(expression));

        if (expression.Function.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
            throw new ArgumentException("Action costs may not depend on total-cost itself.", nameof(expression));

        NumericFunction? declaredFunction = _domain.Functions.FirstOrDefault(function =>
            function.Name.Equals(expression.Function.Name, StringComparison.OrdinalIgnoreCase));
        if (declaredFunction == null)
        {
            throw new ArgumentException(
                $"Undeclared numeric fluent '{expression.Function.Name}' in action cost.");
        }
        if (!ReferenceEquals(declaredFunction, expression.Function))
        {
            throw new ArgumentException(
                $"Numeric function '{expression.Function.Name}' must use the exact declared instance.");
        }

        if (declaredFunction.Parameters.Count != expression.Arguments.Count)
        {
            throw new ArgumentException(
                $"Arity mismatch for numeric fluent '{expression.Function.Name}' in action cost. " +
                $"Expected {declaredFunction.Parameters.Count}, got {expression.Arguments.Count}.");
        }

        for (int i = 0; i < expression.Arguments.Count; i++)
        {
            ValidateCostTerm(
                expression.Arguments[i],
                declaredFunction.Parameters[i].Type,
                actionParameters);
        }
    }

    private void ValidateCostTerm(
        ITerm term,
        string expectedType,
        IReadOnlyList<Variable> actionParameters)
    {
        ArgumentNullException.ThrowIfNull(term);

        string actualType;
        if (term is Variable variable)
        {
            Variable? declaredVariable = actionParameters.FirstOrDefault(parameter =>
                parameter.Name.Equals(variable.Name, StringComparison.OrdinalIgnoreCase));
            if (declaredVariable == null)
                throw new ArgumentException($"Undeclared variable '{variable.Name}' in action cost.");
            if (!ReferenceEquals(declaredVariable, variable))
                throw new ArgumentException(
                    $"Variable '{variable.Name}' in action cost must use the exact action parameter instance.");
            actualType = declaredVariable.Type;
        }
        else if (term is Constant constant)
        {
            Constant? declaredConstant = _domain.Constants.FirstOrDefault(candidate =>
                candidate.Name.Equals(constant.Name, StringComparison.OrdinalIgnoreCase));
            if (declaredConstant == null)
                throw new ArgumentException($"Undeclared constant '{constant.Name}' in action cost.");
            if (!ReferenceEquals(declaredConstant, constant))
                throw new ArgumentException(
                    $"Constant '{constant.Name}' must use the exact declared instance.");
            actualType = declaredConstant.Type;
        }
        else
        {
            throw new InvalidOperationException($"Unsupported term '{term.GetType().Name}'.");
        }

        if (!_domain.IsCompatible(actualType, expectedType))
        {
            throw new ArgumentException(
                $"Type mismatch for '{term.Name}' in action cost. " +
                $"Expected '{expectedType}', got '{actualType}'.");
        }
    }

    private static PddlRequirement ParseRequirement(string requirement)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(requirement);

        return requirement.ToLowerInvariant() switch
        {
            ":strips" => PddlRequirement.Strips,
            ":typing" => PddlRequirement.Typing,
            ":equality" => PddlRequirement.Equality,
            ":negative-preconditions" => PddlRequirement.NegativePreconditions,
            ":disjunctive-preconditions" => PddlRequirement.DisjunctivePreconditions,
            ":fluents" or ":numeric-fluents" => PddlRequirement.NumericFluents,
            ":conditional-effects" => PddlRequirement.ConditionalEffects,
            ":existential-preconditions" => PddlRequirement.ExistentialPreconditions,
            ":universal-preconditions" => PddlRequirement.UniversalPreconditions,
            ":adl" => PddlRequirement.Adl,
            ":derived-predicates" => PddlRequirement.DerivedPredicates,
            ":action-costs" => PddlRequirement.ActionCosts,
            _ => throw new ArgumentException(
                $"Unsupported PDDL requirement '{requirement}'.",
                nameof(requirement))
        };
    }

    private static ImmutableArray<Parameter> BuildParameters(IEnumerable<Variable> variables)
        => variables
            .Select(variable => new Parameter(variable.Name, variable.Type))
            .ToImmutableArray();

    private PddlTerm BuildTerm(ITerm term, IReadOnlySet<Variable> variableScope)
    {
        ArgumentNullException.ThrowIfNull(term);

        if (term is Variable variable)
        {
            if (!variableScope.Contains(variable))
            {
                Variable? sameName = variableScope.FirstOrDefault(candidate =>
                    candidate.Name.Equals(variable.Name, StringComparison.OrdinalIgnoreCase));
                if (sameName != null)
                    throw new ArgumentException(
                        $"Variable '{variable.Name}' must use the exact variable instance from the active scope.");

                throw new ArgumentException($"Undeclared variable '{variable.Name}'.");
            }

            return PddlTerm.Variable(variable.Name);
        }

        if (term is Constant constant)
        {
            Constant? declaredConstant = _domain.Constants.FirstOrDefault(candidate =>
                candidate.Name.Equals(constant.Name, StringComparison.OrdinalIgnoreCase));
            if (declaredConstant == null)
                throw new ArgumentException($"Undeclared constant '{constant.Name}'.");
            if (!ReferenceEquals(declaredConstant, constant))
                throw new ArgumentException(
                    $"Constant '{constant.Name}' must use the exact declared instance.");

            return PddlTerm.Constant(constant.Name);
        }

        throw new InvalidOperationException(
            $"Unsupported term '{term.GetType().Name}'.");
    }

    private void ValidatePredicateIdentity(Predicate predicate)
    {
        Predicate? declaredPredicate = _domain.Fluents.Cast<Predicate>()
            .Concat(_domain.Statics)
            .Concat(_domain.Derived)
            .FirstOrDefault(candidate =>
                candidate.Name.Equals(predicate.Name, StringComparison.OrdinalIgnoreCase));

        if (declaredPredicate == null)
            throw new ArgumentException($"Undeclared predicate '{predicate.Name}'.");

        string expectedType = GetPredicateType(declaredPredicate);
        string actualType = GetPredicateType(predicate);
        if (expectedType != actualType)
        {
            throw new ArgumentException(
                $"Predicate '{predicate.Name}' type mismatch. " +
                $"Expected {expectedType}, got {actualType}.");
        }

        if (!ReferenceEquals(declaredPredicate, predicate))
            throw new ArgumentException(
                $"Predicate '{predicate.Name}' must use the exact declared instance.");
    }

    private static HashSet<Variable> CreateVariableScope(IEnumerable<Variable> variables)
    {
        var scope = new HashSet<Variable>(ReferenceEqualityComparer.Instance);
        foreach (Variable variable in variables)
        {
            ArgumentNullException.ThrowIfNull(variable);
            if (!scope.Add(variable))
                throw new ArgumentException($"Variable '{variable.Name}' is declared more than once in the same scope.");
        }

        return scope;
    }

    private static HashSet<Variable> ExtendVariableScope(
        IReadOnlySet<Variable> parentScope,
        IEnumerable<Variable> variables)
    {
        var scope = new HashSet<Variable>(parentScope, ReferenceEqualityComparer.Instance);
        foreach (Variable variable in variables)
        {
            ArgumentNullException.ThrowIfNull(variable);
            if (!scope.Add(variable))
                throw new ArgumentException($"Variable '{variable.Name}' is already active in the enclosing scope.");
        }

        return scope;
    }

    private static string GetPredicateType(Predicate predicate)
        => predicate switch
        {
            Predicate<Fluent> => "fluent",
            Predicate<Static> => "static",
            Predicate<Derived> => "derived",
            _ => throw new InvalidOperationException(
                $"Unsupported predicate type '{predicate.GetType().Name}'.")
        };
}
