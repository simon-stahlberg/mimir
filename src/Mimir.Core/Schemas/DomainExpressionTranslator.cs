using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Core.Schemas;

internal static class DomainExpressionTranslator
{
    public static IGroundedExpression TranslateExpression(
        ILogicalExpression expr,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        IReadOnlyDictionary<string, NumericFunction> functions,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
    {
        return expr switch
        {
            Comparison comparison => NumericExpressionTranslator.TranslateComparison(comparison, variableScope, functions, mapTerm),
            EmptyLogic => new GroundedTrue(),
            And and => new GroundedAnd(and.Expressions
                .Select(e => TranslateExpression(e, predicateLookup, functions, variableScope, mapTerm, extendVariableScope))
                .ToImmutableArray()),
            Or or => new GroundedOr(or.Expressions
                .Select(e => TranslateExpression(e, predicateLookup, functions, variableScope, mapTerm, extendVariableScope))
                .ToImmutableArray()),
            Not not => new GroundedNot(
                TranslateExpression(not.Expression, predicateLookup, functions, variableScope, mapTerm, extendVariableScope)),
            Imply imply => new GroundedImply(
                TranslateExpression(imply.Antecedent, predicateLookup, functions, variableScope, mapTerm, extendVariableScope),
                TranslateExpression(imply.Consequent, predicateLookup, functions, variableScope, mapTerm, extendVariableScope)),
            Forall forall => TranslateForall(forall, predicateLookup, functions, variableScope, mapTerm, extendVariableScope),
            Exists exists => TranslateExists(exists, predicateLookup, functions, variableScope, mapTerm, extendVariableScope),
            Equality equality => TranslateAtom(
                "=", [equality.Left, equality.Right], predicateLookup, variableScope, mapTerm),
            PredicateCall pCall => TranslateAtom(
                pCall.Name, pCall.Arguments, predicateLookup, variableScope, mapTerm),
            _ => throw new NotSupportedException(
                $"Core cannot execute logical expression '{expr.GetType().Name}'.")
        };
    }

    private static GroundedForall TranslateForall(
        Forall forall,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        IReadOnlyDictionary<string, NumericFunction> functions,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
    {
        var (innerScope, variables) = extendVariableScope(variableScope, forall.Variables);
        return new GroundedForall(
            variables,
            TranslateExpression(forall.Body, predicateLookup, functions, innerScope, mapTerm, extendVariableScope));
    }

    private static GroundedExists TranslateExists(
        Exists exists,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        IReadOnlyDictionary<string, NumericFunction> functions,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
    {
        var (innerScope, variables) = extendVariableScope(variableScope, exists.Variables);
        return new GroundedExists(
            variables,
            TranslateExpression(exists.Body, predicateLookup, functions, innerScope, mapTerm, extendVariableScope));
    }

    public static void ExtractPreconditions(
        ILogicalExpression expr,
        Polarity polarity,
        Dictionary<string, Predicate> allPredicates,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        List<Literal<Atom<Fluent>>> fPre,
        List<Literal<Atom<Static>>> sPre,
        List<Literal<Atom<Derived>>> dPre,
        IReadOnlyDictionary<string, NumericFunction> functions, List<NumericComparison> comparisons)
    {
        switch (expr)
        {
            case Comparison comparison when polarity == Polarity.Positive:
                comparisons.Add(NumericExpressionTranslator.TranslateComparison(comparison, variableScope, functions, mapTerm));
                return;
            case EmptyLogic:
                return;
            case And andExpr:
                foreach (var child in andExpr.Expressions)
                    ExtractPreconditions(child, polarity, allPredicates, variableScope, mapTerm, fPre, sPre, dPre, functions, comparisons);
                break;
            case Not not:
                var flipped = polarity == Polarity.Positive ? Polarity.Negative : Polarity.Positive;
                ExtractPreconditions(not.Expression, flipped, allPredicates, variableScope, mapTerm, fPre, sPre, dPre, functions, comparisons);
                break;
            case PredicateCall pCall:
                AddLiteral(
                    pCall.Name, pCall.Arguments, polarity, allPredicates, variableScope,
                    mapTerm, fPre, sPre, dPre);
                break;
            case Equality equality:
                AddLiteral(
                    "=", [equality.Left, equality.Right], polarity, allPredicates, variableScope,
                    mapTerm, fPre, sPre, dPre);
                break;
            default:
                throw new NotSupportedException(
                    $"Core cannot execute action precondition expression '{expr.GetType().Name}'.");
        }
    }

    public static void ExtractEffects(
        IEffect effect,
        IReadOnlyList<Variable> quantified,
        Dictionary<string, Variable> scope,
        Dictionary<string, Predicate> predicates,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendScope,
        List<ConditionalEffectBase> results,
        IReadOnlyDictionary<string, NumericFunction> functions)
        => ExtractEffects(effect, quantified, scope, predicates, mapTerm, extendScope, results, functions,
            EffectCondition.Empty);

    private sealed record EffectCondition(
        IReadOnlyList<Literal<Atom<Fluent>>> Fluent,
        IReadOnlyList<Literal<Atom<Static>>> Static,
        IReadOnlyList<Literal<Atom<Derived>>> Derived,
        IReadOnlyList<NumericComparison> NumericConditions)
    {
        public static readonly EffectCondition Empty = new([], [], [], []);
    }

    private static void ExtractEffects(
        IEffect effect,
        IReadOnlyList<Variable> quantified,
        Dictionary<string, Variable> scope,
        Dictionary<string, Predicate> predicates,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendScope,
        List<ConditionalEffectBase> results,
        IReadOnlyDictionary<string, NumericFunction> functions,
        EffectCondition outerCondition)
    {
        var fluent = new List<Literal<Atom<Fluent>>>(outerCondition.Fluent);
        var statics = new List<Literal<Atom<Static>>>(outerCondition.Static);
        var derived = new List<Literal<Atom<Derived>>>(outerCondition.Derived);
        var comparisons = new List<NumericComparison>(outerCondition.NumericConditions);
        switch (effect)
        {
            case Mimir.Pddl.Ast.Effects.ConditionalEffect conditional:
                ExtractPreconditions(conditional.Condition, Polarity.Positive, predicates, scope, mapTerm,
                    fluent, statics, derived, functions, comparisons);
                ExtractEffects(conditional.Effect, quantified, scope, predicates, mapTerm, extendScope, results, functions,
                    new EffectCondition(fluent, statics, derived, comparisons));
                return;
            case Mimir.Pddl.Ast.Effects.AndEffect conjunction:
                foreach (IEffect child in conjunction.Effects)
                    ExtractEffects(child, quantified, scope, predicates, mapTerm, extendScope, results, functions, outerCondition);
                return;
            case Mimir.Pddl.Ast.Effects.ForallEffect forall:
                var (innerScope, variables) = extendScope(scope, forall.Variables);
                ExtractEffects(forall.Effect, quantified.Concat(variables).ToArray(), innerScope, predicates,
                    mapTerm, extendScope, results, functions, outerCondition);
                return;
            case Mimir.Pddl.Ast.Effects.Increase increase when NumericFunction.IsTotalCost(increase.Fluent.Name):
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
            var target = (FunctionCall)NumericExpressionTranslator.TranslateExpression(numeric.Target, scope, functions, mapTerm);
            NumericExpression value = NumericExpressionTranslator.TranslateExpression(numeric.Value, scope, functions, mapTerm);
            results.Add(new ConditionalNumericEffect(quantified, fluent, statics, derived, comparisons,
                new NumericUpdate(target, numeric.Operator, value)));
            return;
        }
        Literal<Atom<Fluent>> literal = ExtractEffectLiteral(effect, predicates, scope, mapTerm);
        results.Add(new ConditionalEffect(quantified, fluent, statics, derived, comparisons, literal));
    }

    private static GroundedAtom TranslateAtom(
        string predicateName,
        IEnumerable<Mimir.Pddl.Ast.Models.Term> arguments,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        if (!predicateLookup.TryGetValue(predicateName, out Predicate? predicate))
            throw new InvalidOperationException($"Predicate '{predicateName}' is not declared in the domain.");

        return new GroundedAtom(
            predicate,
            arguments.Select(term => mapTerm(term, variableScope)).ToList());
    }

    private static void AddLiteral(
        string predicateName,
        IEnumerable<Mimir.Pddl.Ast.Models.Term> arguments,
        Polarity polarity,
        IReadOnlyDictionary<string, Predicate> allPredicates,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        List<Literal<Atom<Fluent>>> fPre,
        List<Literal<Atom<Static>>> sPre,
        List<Literal<Atom<Derived>>> dPre)
    {
        List<ITerm> terms = arguments.Select(term => mapTerm(term, variableScope)).ToList();
        if (!allPredicates.TryGetValue(predicateName, out Predicate? predicate))
            throw new InvalidOperationException($"Predicate '{predicateName}' is not declared in the domain.");

        switch (predicate)
        {
            case Predicate<Fluent> fluent:
                fPre.Add(new Literal<Atom<Fluent>>(new Atom<Fluent>(fluent, terms), polarity));
                break;
            case Predicate<Static> stat:
                sPre.Add(new Literal<Atom<Static>>(new Atom<Static>(stat, terms), polarity));
                break;
            case Predicate<Derived> derived:
                dPre.Add(new Literal<Atom<Derived>>(new Atom<Derived>(derived, terms), polarity));
                break;
            default:
                throw new InvalidOperationException($"Predicate '{predicateName}' has an unknown runtime type.");
        }
    }

    private static Literal<Atom<Fluent>> ExtractEffectLiteral(
        IEffect effect,
        Dictionary<string, Predicate> allPredicates,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        (PredicateCall call, Polarity polarity) = effect switch
        {
            Mimir.Pddl.Ast.Effects.AddEffect add => (add.Predicate, Polarity.Positive),
            Mimir.Pddl.Ast.Effects.DeleteEffect delete => (delete.Predicate, Polarity.Negative),
            _ => throw new InvalidOperationException($"Validated effect has unknown type '{effect.GetType().Name}'.")
        };
        if (!allPredicates.TryGetValue(call.Name, out Predicate? predicate))
            throw new InvalidOperationException($"Validated effect references undeclared predicate '{call.Name}'.");
        if (predicate is not Predicate<Fluent> fluent)
            throw new InvalidOperationException($"Validated effect modifies non-fluent predicate '{call.Name}'.");

        List<ITerm> terms = call.Arguments.Select(term => mapTerm(term, variableScope)).ToList();
        return new Literal<Atom<Fluent>>(new Atom<Fluent>(fluent, terms), polarity);
    }
}
