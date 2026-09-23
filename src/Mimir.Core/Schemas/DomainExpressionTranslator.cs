using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

internal sealed class ExtractedCondition
{
    public List<Literal<Atom<Fluent>>> Fluent { get; } = new();
    public List<Literal<Atom<Static>>> Static { get; } = new();
    public List<Literal<Atom<Derived>>> Derived { get; } = new();
    public List<NumericComparison> NumericConditions { get; } = new();
    public bool IsEmpty => Fluent.Count == 0 && Static.Count == 0 && Derived.Count == 0 && NumericConditions.Count == 0;

    public ExtractedCondition Copy()
    {
        var copy = new ExtractedCondition();
        copy.Fluent.AddRange(Fluent);
        copy.Static.AddRange(Static);
        copy.Derived.AddRange(Derived);
        copy.NumericConditions.AddRange(NumericConditions);
        return copy;
    }
}

internal sealed class ExtractedEffects
{
    public List<ConditionalEffectBase> Effects { get; } = new();
    public List<NumericExpression> CostIncreases { get; } = new();
}

internal sealed class DomainExpressionTranslator
{
    private readonly IReadOnlyDictionary<string, Predicate> _predicates;
    private readonly IReadOnlyDictionary<string, Constant> _constants;
    private readonly NumericExpressionTranslator _numeric;

    public DomainExpressionTranslator(
        IReadOnlyDictionary<string, Predicate> predicates,
        IReadOnlyDictionary<string, NumericFunction> functions,
        IReadOnlyDictionary<string, Constant> constants)
    {
        _predicates = predicates;
        _constants = constants;
        _numeric = new NumericExpressionTranslator(functions, MapTerm);
    }

    public static (Dictionary<string, Variable> Scope, List<Variable> Variables) ExtendScope(
        Dictionary<string, Variable>? parentScope,
        IEnumerable<Parameter> parameters)
    {
        var scope = parentScope is null
            ? new Dictionary<string, Variable>(StringComparer.OrdinalIgnoreCase)
            : new Dictionary<string, Variable>(parentScope, StringComparer.OrdinalIgnoreCase);
        var localVariables = new Dictionary<string, Variable>(StringComparer.OrdinalIgnoreCase);
        var variables = new List<Variable>();

        foreach (var parameter in parameters)
        {
            if (!localVariables.TryGetValue(parameter.Name, out var variable))
            {
                variable = new Variable(parameter.Name, parameter.TypeName);
                localVariables[parameter.Name] = variable;
            }

            scope[parameter.Name] = variable;
            variables.Add(variable);
        }

        return (scope, variables);
    }

    public IGroundedExpression TranslateExpression(ILogicalExpression expr, Dictionary<string, Variable> scope)
    {
        return expr switch
        {
            Comparison comparison => _numeric.TranslateComparison(comparison, scope),
            EmptyLogic => new GroundedTrue(),
            And and => new GroundedAnd(and.Expressions.Select(e => TranslateExpression(e, scope)).ToImmutableArray()),
            Or or => new GroundedOr(or.Expressions.Select(e => TranslateExpression(e, scope)).ToImmutableArray()),
            Not not => new GroundedNot(TranslateExpression(not.Expression, scope)),
            Imply imply => new GroundedImply(TranslateExpression(imply.Antecedent, scope), TranslateExpression(imply.Consequent, scope)),
            Forall forall => TranslateForall(forall, scope),
            Exists exists => TranslateExists(exists, scope),
            Equality equality => TranslateAtom("=", [equality.Left, equality.Right], scope),
            PredicateCall pCall => TranslateAtom(pCall.Name, pCall.Arguments, scope),
            _ => throw new NotSupportedException(
                $"Core cannot execute logical expression '{expr.GetType().Name}'.")
        };
    }

    private GroundedForall TranslateForall(Forall forall, Dictionary<string, Variable> scope)
    {
        var (innerScope, variables) = ExtendScope(scope, forall.Variables);
        return new GroundedForall(variables, TranslateExpression(forall.Body, innerScope));
    }

    private GroundedExists TranslateExists(Exists exists, Dictionary<string, Variable> scope)
    {
        var (innerScope, variables) = ExtendScope(scope, exists.Variables);
        return new GroundedExists(variables, TranslateExpression(exists.Body, innerScope));
    }

    public void ExtractCondition(
        ILogicalExpression expr,
        Polarity polarity,
        Dictionary<string, Variable> scope,
        ExtractedCondition output)
    {
        switch (expr)
        {
            case Comparison comparison when polarity == Polarity.Positive:
                output.NumericConditions.Add(_numeric.TranslateComparison(comparison, scope));
                return;
            case EmptyLogic:
                return;
            case And andExpr:
                foreach (var child in andExpr.Expressions)
                    ExtractCondition(child, polarity, scope, output);
                return;
            case Not not:
                var flipped = polarity == Polarity.Positive ? Polarity.Negative : Polarity.Positive;
                ExtractCondition(not.Expression, flipped, scope, output);
                return;
            case PredicateCall pCall:
                AddLiteral(pCall.Name, pCall.Arguments, polarity, scope, output);
                return;
            case Equality equality:
                AddLiteral("=", [equality.Left, equality.Right], polarity, scope, output);
                return;
            default:
                throw new NotSupportedException(
                    $"Core cannot execute action precondition expression '{expr.GetType().Name}'.");
        }
    }

    public void ExtractEffects(IEffect effect, Dictionary<string, Variable> scope, ExtractedEffects output)
        => ExtractEffects(effect, [], scope, new ExtractedCondition(), output);

    private void ExtractEffects(
        IEffect effect,
        IReadOnlyList<Variable> quantified,
        Dictionary<string, Variable> scope,
        ExtractedCondition condition,
        ExtractedEffects output)
    {
        switch (effect)
        {
            case Mimir.Pddl.Ast.Effects.ConditionalEffect conditional:
                ExtractedCondition inner = condition.Copy();
                ExtractCondition(conditional.Condition, Polarity.Positive, scope, inner);
                ExtractEffects(conditional.Effect, quantified, scope, inner, output);
                return;
            case Mimir.Pddl.Ast.Effects.AndEffect conjunction:
                foreach (IEffect child in conjunction.Effects)
                    ExtractEffects(child, quantified, scope, condition, output);
                return;
            case Mimir.Pddl.Ast.Effects.ForallEffect forall:
                var (innerScope, variables) = ExtendScope(scope, forall.Variables);
                ExtractEffects(forall.Effect, quantified.Concat(variables).ToArray(), innerScope, condition, output);
                return;
            case Mimir.Pddl.Ast.Effects.Increase increase when NumericFunction.IsTotalCost(increase.Fluent.Name):
                if (quantified.Count > 0 || !condition.IsEmpty)
                    throw new InvalidOperationException("Validated action costs cannot be conditional or quantified.");
                output.CostIncreases.Add(_numeric.TranslateExpression(increase.Value, scope));
                return;
        }
        (FluentCall Target, INumericExpression Value, NumericUpdateOperator Operator)? update = effect switch
        {
            Mimir.Pddl.Ast.Effects.Assign e => (e.Fluent, e.Value, NumericUpdateOperator.Assign),
            Mimir.Pddl.Ast.Effects.Increase e => (e.Fluent, e.Value, NumericUpdateOperator.Increase),
            Mimir.Pddl.Ast.Effects.Decrease e => (e.Fluent, e.Value, NumericUpdateOperator.Decrease),
            Mimir.Pddl.Ast.Effects.ScaleUp e => (e.Fluent, e.Value, NumericUpdateOperator.ScaleUp),
            Mimir.Pddl.Ast.Effects.ScaleDown e => (e.Fluent, e.Value, NumericUpdateOperator.ScaleDown),
            _ => null
        };
        if (update is { } numeric)
        {
            var target = (FunctionCall)_numeric.TranslateExpression(numeric.Target, scope);
            NumericExpression value = _numeric.TranslateExpression(numeric.Value, scope);
            output.Effects.Add(new ConditionalNumericEffect(quantified, condition.Fluent, condition.Static, condition.Derived,
                condition.NumericConditions, new NumericUpdate(target, numeric.Operator, value)));
            return;
        }
        output.Effects.Add(new ConditionalEffect(quantified, condition.Fluent, condition.Static, condition.Derived,
            condition.NumericConditions, ExtractEffectLiteral(effect, scope)));
    }

    private ITerm MapTerm(Term term, Dictionary<string, Variable> scope)
    {
        if (term.IsVariable)
        {
            if (scope.TryGetValue(term.Name, out var variable)) return variable;

            throw new InvalidOperationException($"Validated expression contains undeclared variable '{term.Name}'.");
        }

        if (_constants.TryGetValue(term.Name, out var constant)) return constant;

        throw new InvalidOperationException($"Validated expression contains undeclared constant '{term.Name}'.");
    }

    private GroundedAtom TranslateAtom(string predicateName, IEnumerable<Term> arguments, Dictionary<string, Variable> scope)
    {
        if (!_predicates.TryGetValue(predicateName, out Predicate? predicate))
            throw new InvalidOperationException($"Predicate '{predicateName}' is not declared in the domain.");

        return new GroundedAtom(predicate, arguments.Select(term => MapTerm(term, scope)).ToList());
    }

    private void AddLiteral(
        string predicateName,
        IEnumerable<Term> arguments,
        Polarity polarity,
        Dictionary<string, Variable> scope,
        ExtractedCondition output)
    {
        List<ITerm> terms = arguments.Select(term => MapTerm(term, scope)).ToList();
        if (!_predicates.TryGetValue(predicateName, out Predicate? predicate))
            throw new InvalidOperationException($"Predicate '{predicateName}' is not declared in the domain.");

        switch (predicate)
        {
            case Predicate<Fluent> fluent:
                output.Fluent.Add(new Literal<Atom<Fluent>>(new Atom<Fluent>(fluent, terms), polarity));
                break;
            case Predicate<Static> stat:
                output.Static.Add(new Literal<Atom<Static>>(new Atom<Static>(stat, terms), polarity));
                break;
            case Predicate<Derived> derived:
                output.Derived.Add(new Literal<Atom<Derived>>(new Atom<Derived>(derived, terms), polarity));
                break;
            default:
                throw new InvalidOperationException($"Predicate '{predicateName}' has an unknown runtime type.");
        }
    }

    private Literal<Atom<Fluent>> ExtractEffectLiteral(IEffect effect, Dictionary<string, Variable> scope)
    {
        (PredicateCall call, Polarity polarity) = effect switch
        {
            Mimir.Pddl.Ast.Effects.AddEffect add => (add.Predicate, Polarity.Positive),
            Mimir.Pddl.Ast.Effects.DeleteEffect delete => (delete.Predicate, Polarity.Negative),
            _ => throw new InvalidOperationException($"Validated effect has unknown type '{effect.GetType().Name}'.")
        };
        if (!_predicates.TryGetValue(call.Name, out Predicate? predicate))
            throw new InvalidOperationException($"Validated effect references undeclared predicate '{call.Name}'.");
        if (predicate is not Predicate<Fluent> fluent)
            throw new InvalidOperationException($"Validated effect modifies non-fluent predicate '{call.Name}'.");

        List<ITerm> terms = call.Arguments.Select(term => MapTerm(term, scope)).ToList();
        return new Literal<Atom<Fluent>>(new Atom<Fluent>(fluent, terms), polarity);
    }
}
