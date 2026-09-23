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

        HashSet<Variable> actionScope = CreateVariableScope(action.Parameters);
        INumericExpression cost = BuildNumericExpression(action.CostExpression, actionScope);

        ILogicalExpression precondition = BuildCondition(
            action.FluentPreconditions,
            action.StaticPreconditions,
            action.DerivedPreconditions,
            action.NumericPreconditions,
            actionScope);
        ImmutableArray<IEffect>.Builder effects = ImmutableArray.CreateBuilder<IEffect>(
            action.Effects.Count + action.NumericEffects.Count + (_actionCostsEnabled ? 1 : 0));

        foreach (ConditionalEffectBase effect in action.Effects.Concat<ConditionalEffectBase>(action.NumericEffects))
            effects.Add(BuildEffect(effect, actionScope));

        if (_actionCostsEnabled)
        {
            effects.Add(new PddlConditionalEffect(
                new EmptyLogic(),
                new Increase(
                    new FluentCall(NumericFunction.TotalCostName, ImmutableArray<PddlTerm>.Empty),
                    cost)));
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
            NumericComparison comparison => BuildComparison(comparison, variableScope),
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

    private Comparison BuildComparison(NumericComparison comparison, IReadOnlySet<Variable> scope)
        => new(
            comparison.Operator switch
            {
                ComparisonOperator.Equal => Mimir.Pddl.Ast.Expressions.ComparisonOperator.Equal,
                ComparisonOperator.LessThan => Mimir.Pddl.Ast.Expressions.ComparisonOperator.LessThan,
                ComparisonOperator.LessThanOrEqual => Mimir.Pddl.Ast.Expressions.ComparisonOperator.LessThanOrEqual,
                ComparisonOperator.GreaterThan => Mimir.Pddl.Ast.Expressions.ComparisonOperator.GreaterThan,
                ComparisonOperator.GreaterThanOrEqual => Mimir.Pddl.Ast.Expressions.ComparisonOperator.GreaterThanOrEqual,
                _ => throw new InvalidOperationException($"Unsupported comparison operator '{comparison.Operator}'.")
            },
            BuildNumericExpression(comparison.Left, scope),
            BuildNumericExpression(comparison.Right, scope));

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
        IReadOnlyList<NumericComparison> numericConditions,
        IReadOnlySet<Variable> variableScope)
    {
        var expressions = new List<ILogicalExpression>(
            fluentLiterals.Count + staticLiterals.Count + derivedLiterals.Count);

        AddLiterals(fluentLiterals, expressions, variableScope);
        AddLiterals(staticLiterals, expressions, variableScope);
        AddLiterals(derivedLiterals, expressions, variableScope);
        foreach (NumericComparison comparison in numericConditions)
            expressions.Add(BuildComparison(comparison, variableScope));

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
        ConditionalEffectBase effect,
        IReadOnlySet<Variable> actionScope)
    {
        ArgumentNullException.ThrowIfNull(effect);

        HashSet<Variable> effectScope = ExtendVariableScope(actionScope, effect.QuantifiedVariables);
        ILogicalExpression condition = BuildCondition(
            effect.FluentConditions,
            effect.StaticConditions,
            effect.DerivedConditions,
            effect.NumericConditions,
            effectScope);
        IEffect innerEffect = effect switch
        {
            ConditionalEffect literal => BuildLiteralEffect(literal.Effect, effectScope),
            ConditionalNumericEffect numeric => BuildNumericUpdate(numeric.Effect, effectScope),
            _ => throw new InvalidOperationException($"Unknown effect '{effect.GetType().Name}'.")
        };

        if (effect.QuantifiedVariables.Count == 0)
            return new PddlConditionalEffect(condition, innerEffect);

        IEffect quantifiedEffect = LogicalExpressionSemantics.IsAlwaysTrue(condition)
            ? innerEffect
            : new PddlConditionalEffect(condition, innerEffect);

        return new PddlConditionalEffect(
            new EmptyLogic(),
            new ForallEffect(
                BuildParameters(effect.QuantifiedVariables),
                quantifiedEffect));
    }

    private IEffect BuildLiteralEffect(Literal<Atom<Fluent>> literal, IReadOnlySet<Variable> scope)
    {
        PredicateCall call = BuildPredicateCall(literal.Value.Predicate, literal.Value.Arguments, scope);
        return literal.Polarity switch
        {
            Polarity.Positive => new AddEffect(call),
            Polarity.Negative => new DeleteEffect(call),
            _ => throw new InvalidOperationException($"Unsupported effect polarity '{literal.Polarity}'.")
        };
    }

    private IEffect BuildNumericUpdate(NumericUpdate update, IReadOnlySet<Variable> scope)
    {
        FluentCall target = BuildNumericFunctionCall(update.Target, scope);
        INumericExpression value = BuildNumericExpression(update.Expression, scope);
        return update.Operator switch
        {
            NumericUpdateOperator.Assign => new Assign(target, value),
            NumericUpdateOperator.Increase => new Increase(target, value),
            NumericUpdateOperator.Decrease => new Decrease(target, value),
            NumericUpdateOperator.ScaleUp => new ScaleUp(target, value),
            NumericUpdateOperator.ScaleDown => new ScaleDown(target, value),
            _ => throw new ArgumentOutOfRangeException(nameof(update))
        };
    }

    private INumericExpression BuildNumericExpression(
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
                    BuildNumericExpression(binary.Left, variableScope),
                    BuildNumericExpression(binary.Right, variableScope)),
                NumericOperator.Subtract => new Subtract(
                    BuildNumericExpression(binary.Left, variableScope),
                    BuildNumericExpression(binary.Right, variableScope)),
                NumericOperator.Multiply => new Multiply(
                    BuildNumericExpression(binary.Left, variableScope),
                    BuildNumericExpression(binary.Right, variableScope)),
                NumericOperator.Divide => new Divide(
                    BuildNumericExpression(binary.Left, variableScope),
                    BuildNumericExpression(binary.Right, variableScope)),
                _ => throw new InvalidOperationException(
                    $"Unsupported numeric operator '{binary.Operator}'.")
            },
            FunctionCall function => BuildNumericFunctionCall(function, variableScope),
            _ => throw new InvalidOperationException(
                $"Unsupported numeric expression '{expression.GetType().Name}'.")
        };
    }

    private FluentCall BuildNumericFunctionCall(
        FunctionCall expression,
        IReadOnlySet<Variable> variableScope)
    {
        NumericFunction function = expression.Function;
        if (NumericFunction.IsTotalCost(function.Name))
            throw new ArgumentException("total-cost is maintained through action costs and cannot be referenced in numeric expressions.", nameof(expression));
        if (!_domain.Functions.Contains(function))
        {
            bool sameName = _domain.Functions.Any(declared => declared.Name.Equals(function.Name, StringComparison.OrdinalIgnoreCase));
            throw new ArgumentException(sameName
                ? $"Numeric function '{function.Name}' must use the exact declared instance."
                : $"Undeclared numeric function '{function.Name}'.", nameof(expression));
        }
        if (function.Parameters.Count != expression.Arguments.Count)
        {
            throw new ArgumentException(
                $"Numeric function '{function.Name}' expects {function.Parameters.Count} arguments, got {expression.Arguments.Count}.",
                nameof(expression));
        }

        var arguments = ImmutableArray.CreateBuilder<PddlTerm>(expression.Arguments.Count);
        for (int i = 0; i < expression.Arguments.Count; i++)
        {
            ITerm argument = expression.Arguments[i];
            arguments.Add(BuildTerm(argument, variableScope));
            string actualType = argument is Variable variable ? variable.Type : ((Constant)argument).Type;
            if (!_domain.IsCompatible(actualType, function.Parameters[i].Type))
            {
                throw new ArgumentException(
                    $"Type mismatch for '{argument.Name}' in numeric function '{function.Name}'. " +
                    $"Expected '{function.Parameters[i].Type}', got '{actualType}'.",
                    nameof(expression));
            }
        }

        return new FluentCall(function.Name, arguments.MoveToImmutable());
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
                $"Numeric value '{value}' cannot be represented as a PDDL number.",
                nameof(value),
                exception);
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
