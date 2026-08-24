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
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
    {
        return expr switch
        {
            EmptyLogic => new GroundedTrue(),
            And and => new GroundedAnd(and.Expressions
                .Select(e => TranslateExpression(e, predicateLookup, variableScope, mapTerm, extendVariableScope))
                .ToImmutableArray()),
            Or or => new GroundedOr(or.Expressions
                .Select(e => TranslateExpression(e, predicateLookup, variableScope, mapTerm, extendVariableScope))
                .ToImmutableArray()),
            Not not => new GroundedNot(
                TranslateExpression(not.Expression, predicateLookup, variableScope, mapTerm, extendVariableScope)),
            Imply imply => new GroundedImply(
                TranslateExpression(imply.Antecedent, predicateLookup, variableScope, mapTerm, extendVariableScope),
                TranslateExpression(imply.Consequent, predicateLookup, variableScope, mapTerm, extendVariableScope)),
            Forall forall => TranslateForall(forall, predicateLookup, variableScope, mapTerm, extendVariableScope),
            Exists exists => TranslateExists(exists, predicateLookup, variableScope, mapTerm, extendVariableScope),
            Equality equality => TranslateAtom(
                "=", [equality.Left, equality.Right], predicateLookup, variableScope, mapTerm),
            PredicateCall pCall => TranslateAtom(
                pCall.Name, pCall.Arguments, predicateLookup, variableScope, mapTerm),
            _ => throw new NotSupportedException(
                $"Core cannot execute logical expression '{expr.GetType().Name}'.")
        };
    }

    public static IGroundedExpression TranslateExpressionInternal(
        ILogicalExpression expr,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
        => TranslateExpression(expr, predicateLookup, variableScope, mapTerm, extendVariableScope);

    private static GroundedForall TranslateForall(
        Forall forall,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
    {
        var (innerScope, variables) = extendVariableScope(variableScope, forall.Variables);
        return new GroundedForall(
            variables,
            TranslateExpression(forall.Body, predicateLookup, innerScope, mapTerm, extendVariableScope));
    }

    private static GroundedExists TranslateExists(
        Exists exists,
        IReadOnlyDictionary<string, Predicate> predicateLookup,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope)
    {
        var (innerScope, variables) = extendVariableScope(variableScope, exists.Variables);
        return new GroundedExists(
            variables,
            TranslateExpression(exists.Body, predicateLookup, innerScope, mapTerm, extendVariableScope));
    }

    public static void ExtractPreconditions(
        ILogicalExpression expr,
        Polarity polarity,
        Dictionary<string, Predicate> allPredicates,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        List<Literal<Atom<Fluent>>> fPre,
        List<Literal<Atom<Static>>> sPre,
        List<Literal<Atom<Derived>>> dPre)
    {
        switch (expr)
        {
            case EmptyLogic:
                return;
            case And andExpr:
                foreach (var child in andExpr.Expressions)
                    ExtractPreconditions(child, polarity, allPredicates, variableScope, mapTerm, fPre, sPre, dPre);
                break;
            case Not not:
                var flipped = polarity == Polarity.Positive ? Polarity.Negative : Polarity.Positive;
                ExtractPreconditions(not.Expression, flipped, allPredicates, variableScope, mapTerm, fPre, sPre, dPre);
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
        Mimir.Pddl.Ast.Effects.ConditionalEffect cond,
        IReadOnlyList<Variable> outerQuantifiedVars,
        Dictionary<string, Variable> variableScope,
        Dictionary<string, Predicate> allPredicates,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope,
        List<ConditionalEffect> results)
    {
        var condFPre = new List<Literal<Atom<Fluent>>>();
        var condSPre = new List<Literal<Atom<Static>>>();
        var condDPre = new List<Literal<Atom<Derived>>>();

        if (!LogicalExpressionSemantics.IsAlwaysTrue(cond.Condition))
            ExtractPreconditions(cond.Condition, Polarity.Positive, allPredicates, variableScope, mapTerm, condFPre, condSPre, condDPre);

        var innerEffect = cond.Effect;

        if (innerEffect is Mimir.Pddl.Ast.Effects.ForallEffect forall)
        {
            var (innerScope, innerVariables) = extendVariableScope(variableScope, forall.Variables);
            var quantifiedVars = new List<Variable>(outerQuantifiedVars.Count + innerVariables.Count);
            quantifiedVars.AddRange(outerQuantifiedVars);
            quantifiedVars.AddRange(innerVariables);

            ExtractForallInner(
                forall.Effect,
                quantifiedVars,
                innerScope,
                condFPre,
                condSPre,
                condDPre,
                allPredicates,
                mapTerm,
                extendVariableScope,
                results);
        }
        else
        {
            var effectLiteral = ExtractSingleEffectLiteral(innerEffect, allPredicates, variableScope, mapTerm);
            if (effectLiteral != null)
                results.Add(new ConditionalEffect(outerQuantifiedVars, condFPre, condSPre, condDPre, effectLiteral));
        }
    }

    private static void ExtractForallInner(
        IEffect innerEffect,
        IReadOnlyList<Variable> quantifiedVars,
        Dictionary<string, Variable> variableScope,
        List<Literal<Atom<Fluent>>> outerCondF,
        List<Literal<Atom<Static>>> outerCondS,
        List<Literal<Atom<Derived>>> outerCondD,
        Dictionary<string, Predicate> allPredicates,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm,
        Func<Dictionary<string, Variable>?, IEnumerable<Mimir.Pddl.Ast.Models.Parameter>, (Dictionary<string, Variable> Scope, List<Variable> Variables)> extendVariableScope,
        List<ConditionalEffect> results)
    {
        switch (innerEffect)
        {
            case Mimir.Pddl.Ast.Effects.ConditionalEffect innerCond:
            {
                var condF = new List<Literal<Atom<Fluent>>>(outerCondF);
                var condS = new List<Literal<Atom<Static>>>(outerCondS);
                var condD = new List<Literal<Atom<Derived>>>(outerCondD);

                if (!LogicalExpressionSemantics.IsAlwaysTrue(innerCond.Condition))
                    ExtractPreconditions(innerCond.Condition, Polarity.Positive, allPredicates, variableScope, mapTerm, condF, condS, condD);

                ExtractForallInner(
                    innerCond.Effect,
                    quantifiedVars,
                    variableScope,
                    condF,
                    condS,
                    condD,
                    allPredicates,
                    mapTerm,
                    extendVariableScope,
                    results);
                return;
            }
            case Mimir.Pddl.Ast.Effects.AndEffect andEffect:
            {
                foreach (IEffect child in andEffect.Effects)
                {
                    ExtractForallInner(
                        child,
                        quantifiedVars,
                        variableScope,
                        outerCondF,
                        outerCondS,
                        outerCondD,
                        allPredicates,
                        mapTerm,
                        extendVariableScope,
                        results);
                }
                return;
            }
            case Mimir.Pddl.Ast.Effects.ForallEffect forall:
            {
                var (innerScope, innerVariables) = extendVariableScope(variableScope, forall.Variables);
                var nestedQuantifiedVariables = new List<Variable>(quantifiedVars.Count + innerVariables.Count);
                nestedQuantifiedVariables.AddRange(quantifiedVars);
                nestedQuantifiedVariables.AddRange(innerVariables);

                ExtractForallInner(
                    forall.Effect,
                    nestedQuantifiedVariables,
                    innerScope,
                    outerCondF,
                    outerCondS,
                    outerCondD,
                    allPredicates,
                    mapTerm,
                    extendVariableScope,
                    results);
                return;
            }
            default:
            {
                Literal<Atom<Fluent>>? literal =
                    ExtractSingleEffectLiteral(innerEffect, allPredicates, variableScope, mapTerm);
                if (literal is not null)
                {
                    results.Add(new ConditionalEffect(
                        quantifiedVars,
                        outerCondF,
                        outerCondS,
                        outerCondD,
                        literal));
                }
                return;
            }
        }
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

    private static Literal<Atom<Fluent>>? ExtractSingleEffectLiteral(
        IEffect effect,
        Dictionary<string, Predicate> allPredicates,
        Dictionary<string, Variable> variableScope,
        Func<Mimir.Pddl.Ast.Models.Term, Dictionary<string, Variable>, ITerm> mapTerm)
    {
        switch (effect)
        {
            case Mimir.Pddl.Ast.Effects.AddEffect add:
            {
                var terms = add.Predicate.Arguments.Select(term => mapTerm(term, variableScope)).ToList();
                if (!allPredicates.TryGetValue(add.Predicate.Name, out var predicate))
                    throw new InvalidOperationException(
                        $"Validated effect references undeclared predicate '{add.Predicate.Name}'.");
                if (predicate is not Predicate<Fluent> fluent)
                    throw new InvalidOperationException(
                        $"Validated effect modifies non-fluent predicate '{add.Predicate.Name}'.");

                return new Literal<Atom<Fluent>>(new Atom<Fluent>(fluent, terms), Polarity.Positive);
            }
            case Mimir.Pddl.Ast.Effects.DeleteEffect del:
            {
                var terms = del.Predicate.Arguments.Select(term => mapTerm(term, variableScope)).ToList();
                if (!allPredicates.TryGetValue(del.Predicate.Name, out var predicate))
                    throw new InvalidOperationException(
                        $"Validated effect references undeclared predicate '{del.Predicate.Name}'.");
                if (predicate is not Predicate<Fluent> fluent)
                    throw new InvalidOperationException(
                        $"Validated effect modifies non-fluent predicate '{del.Predicate.Name}'.");

                return new Literal<Atom<Fluent>>(new Atom<Fluent>(fluent, terms), Polarity.Negative);
            }
            case Mimir.Pddl.Ast.Effects.Increase increase
                when increase.Fluent.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase):
                return null;
            case Mimir.Pddl.Ast.Effects.Assign:
            case Mimir.Pddl.Ast.Effects.Increase:
            case Mimir.Pddl.Ast.Effects.Decrease:
            case Mimir.Pddl.Ast.Effects.ScaleUp:
            case Mimir.Pddl.Ast.Effects.ScaleDown:
                throw new NotSupportedException("Core does not support ordinary numeric effects.");
            default:
                throw new InvalidOperationException(
                    $"Validated effect has unknown type '{effect.GetType().Name}'.");
        }
    }
}
