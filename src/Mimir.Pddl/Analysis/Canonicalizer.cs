using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Builders;

namespace Mimir.Pddl.Analysis;

public static class Canonicalizer
{
    public static DomainDefinition Compile(DomainDefinition domain)
    {
        ArgumentNullException.ThrowIfNull(domain);
        DomainDefinition validatedDomain = SemanticValidator.ValidateDomain(domain);
        return new Compiler(validatedDomain).Compile();
    }

    private sealed class Compiler
    {
        private readonly DomainDefinition _originalDomain;
        private readonly DomainBuilder _builder;
        private readonly HashSet<string> _reservedSymbols;
        private int _axiomCounter;

        public Compiler(DomainDefinition domain)
        {
            _originalDomain = domain;
            _builder = new DomainBuilder(domain.Name);
            _reservedSymbols = domain.Predicates
                .Select(predicate => predicate.Name)
                .Concat(domain.DerivedPredicates.Select(derived => derived.Signature.Name))
                .Concat(domain.Functions.Select(function => function.Name))
                .ToHashSet(StringComparer.OrdinalIgnoreCase);
        }

        public DomainDefinition Compile()
        {
            // Copy basic structures
            foreach (var req in _originalDomain.Requirements) _builder.AddRequirement(req);
            foreach (var t in _originalDomain.Types) _builder.AddType(t.Name, t.ParentType);
            foreach (var c in _originalDomain.Constants) _builder.AddConstant(c.Name, c.ParentType);
            foreach (var p in _originalDomain.Predicates) _builder.AddPredicate(p.Name, p.Parameters.ToArray());
            foreach (var f in _originalDomain.Functions) _builder.AddFunction(f.Name, f.Parameters.ToArray());
            foreach (var d in _originalDomain.DerivedPredicates)
            {
                var compiledBody = CompilePrecondition(d.Body, d.Signature.Parameters, true);
                _builder.AddDerivedPredicate(d.Signature, compiledBody);
            }

            // Compile actions
            foreach (var action in _originalDomain.Actions)
            {
                var newAction = CompileAction(action);
                _builder.AddAction(newAction);
            }

            return _builder.Build();
        }

        private ActionDefinition CompileAction(ActionDefinition action)
        {
            var builder = new ActionBuilder(action.Name);

            foreach (var p in action.Parameters)
                builder.AddParameter(p.Name, p.TypeName);

            ILogicalExpression flatPre = CompilePrecondition(
                action.Precondition ?? new EmptyLogic(),
                action.Parameters);
            builder.WithPrecondition(flatPre);

            IEffect flatEff = CompileEffect(
                action.Effect ?? new AndEffect(ImmutableArray<IEffect>.Empty),
                action.Parameters);
            builder.WithEffect(flatEff);

            return builder.Build();
        }

        private ILogicalExpression CompilePrecondition(ILogicalExpression expr, ImmutableArray<Parameter> availableParams, bool canBeComplex = false)
        {
            expr = Simplify(expr);

            if (!canBeComplex && IsComplex(expr))
            {
                return ExtractAxiom(expr, availableParams);
            }

            switch (expr)
            {
                case And a:
                    return new And(a.Expressions.Select(e => CompilePrecondition(e, availableParams, false)).ToImmutableArray());
                case Not n:
                    return new Not(CompilePrecondition(n.Expression, availableParams, false));
                case Or o:
                    return new Or(o.Expressions.Select(e => CompilePrecondition(e, availableParams, false)).ToImmutableArray());
                case Exists e:
                    return new Exists(e.Variables, CompilePrecondition(e.Body, availableParams.AddRange(e.Variables), false));
                case Forall f:
                    return new Forall(f.Variables, CompilePrecondition(f.Body, availableParams.AddRange(f.Variables), false));
                default:
                    return expr;
            }
        }

        private ILogicalExpression Simplify(ILogicalExpression expression)
        {
            switch (expression)
            {
                case Imply imply:
                {
                    ILogicalExpression antecedent = Simplify(imply.Antecedent);
                    ILogicalExpression consequent = Simplify(imply.Consequent);
                    return SimplifyOr([
                        SimplifyNot(antecedent),
                        consequent
                    ]);
                }
                case Not not:
                    return SimplifyNot(Simplify(not.Expression));
                case And and:
                    return SimplifyAnd(and.Expressions.Select(Simplify));
                case Or or:
                    return SimplifyOr(or.Expressions.Select(Simplify));
                case Exists exists:
                    return SimplifyExists(exists);
                case Forall forall:
                    return SimplifyForall(forall);
                case Equality equality:
                    return SimplifyEquality(equality);
                default:
                    return expression;
            }
        }

        private ILogicalExpression SimplifyAnd(IEnumerable<ILogicalExpression> expressions)
        {
            var terms = new List<ILogicalExpression>();
            foreach (ILogicalExpression expression in expressions)
            {
                switch (GetConstantValue(expression))
                {
                    case LogicalConstant.False:
                        return FalseExpression();
                    case LogicalConstant.True:
                        continue;
                }

                if (expression is And nested)
                    terms.AddRange(nested.Expressions);
                else
                    terms.Add(expression);
            }

            return terms.Count switch
            {
                0 => new EmptyLogic(),
                1 => terms[0],
                _ => new And(terms.ToImmutableArray())
            };
        }

        private ILogicalExpression SimplifyOr(IEnumerable<ILogicalExpression> expressions)
        {
            var terms = new List<ILogicalExpression>();
            foreach (ILogicalExpression expression in expressions)
            {
                switch (GetConstantValue(expression))
                {
                    case LogicalConstant.True:
                        return new EmptyLogic();
                    case LogicalConstant.False:
                        continue;
                }

                if (expression is Or nested)
                    terms.AddRange(nested.Expressions);
                else
                    terms.Add(expression);
            }

            return terms.Count switch
            {
                0 => FalseExpression(),
                1 => terms[0],
                _ => new Or(terms.ToImmutableArray())
            };
        }

        private static ILogicalExpression SimplifyNot(ILogicalExpression expression)
        {
            return GetConstantValue(expression) switch
            {
                LogicalConstant.True => FalseExpression(),
                LogicalConstant.False => new EmptyLogic(),
                _ => new Not(expression)
            };
        }

        private ILogicalExpression SimplifyExists(Exists exists)
        {
            ILogicalExpression body = Simplify(exists.Body);
            if (exists.Variables.IsDefaultOrEmpty)
                return body;

            return GetConstantValue(body) == LogicalConstant.False
                ? FalseExpression()
                : new Exists(exists.Variables, body);
        }

        private ILogicalExpression SimplifyForall(Forall forall)
        {
            ILogicalExpression body = Simplify(forall.Body);
            if (forall.Variables.IsDefaultOrEmpty)
                return body;

            return GetConstantValue(body) == LogicalConstant.True
                ? new EmptyLogic()
                : new Forall(forall.Variables, body);
        }

        private static ILogicalExpression SimplifyEquality(Equality equality)
        {
            if (equality.Left.IsVariable == equality.Right.IsVariable
                && equality.Left.Name.Equals(equality.Right.Name, StringComparison.OrdinalIgnoreCase))
            {
                return new EmptyLogic();
            }

            return !equality.Left.IsVariable && !equality.Right.IsVariable
                ? FalseExpression()
                : equality;
        }

        private static LogicalConstant GetConstantValue(ILogicalExpression expression)
        {
            return expression switch
            {
                EmptyLogic => LogicalConstant.True,
                Or value when value.Expressions.IsDefaultOrEmpty => LogicalConstant.False,
                _ => LogicalConstant.Unknown
            };
        }

        private static Or FalseExpression()
            => new(ImmutableArray<ILogicalExpression>.Empty);

        private enum LogicalConstant
        {
            Unknown,
            False,
            True
        }

        private bool IsComplex(ILogicalExpression expr)
        {
            if (expr is Not not)
                return not.Expression is not PredicateCall and not Equality;

            return expr is Or || expr is Exists || expr is Forall;
        }

        private ILogicalExpression ExtractAxiom(ILogicalExpression body, ImmutableArray<Parameter> availableParams)
        {
            var freeVars = GetFreeVariables(body)
                .Where(v => v.StartsWith("?"))
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .ToList();

            var axiomParams = new List<Parameter>();
            foreach (var fv in freeVars)
            {
                var p = availableParams.FirstOrDefault(ap => ap.Name.Equals(fv, StringComparison.OrdinalIgnoreCase));
                if (p is null)
                    throw new InvalidOperationException($"Validated expression contains unavailable free variable '{fv}'.");

                axiomParams.Add(p);
            }

            // Now compile the body as a complex expression (allowing the top-level operator)
            var compiledBody = CompilePrecondition(body, axiomParams.ToImmutableArray(), true);

            string axiomName = $"axiom_{_axiomCounter++}";
            if (!_reservedSymbols.Add(axiomName))
            {
                // This fail-fast collision policy is intentional; revisit it if real domains encounter it.
                throw new PddlValidationException(
                    $"Generated axiom name '{axiomName}' clashes with an existing predicate or numeric fluent.");
            }
            var sig = new PredicateDeclaration(axiomName, axiomParams.ToImmutableArray());

            _builder.AddPredicate(axiomName, axiomParams.ToArray());
            _builder.AddDerivedPredicate(sig, compiledBody);

            var args = axiomParams.Select(ap => Term.Variable(ap.Name)).ToImmutableArray();
            return new PredicateCall(axiomName, args);
        }

        private IEnumerable<string> GetFreeVariables(ILogicalExpression expr)
        {
            switch (expr)
            {
                case PredicateCall p: return p.Arguments.Where(a => a.IsVariable).Select(a => a.Name);
                case Equality eq: return new[] { eq.Left, eq.Right }.Where(a => a.IsVariable).Select(a => a.Name);
                case Comparison comparison:
                    return GetFreeVariables(comparison.Left).Concat(GetFreeVariables(comparison.Right));
                case Not n: return GetFreeVariables(n.Expression);
                case And a: return a.Expressions.SelectMany(GetFreeVariables);
                case Or o: return o.Expressions.SelectMany(GetFreeVariables);
                case Imply i: return GetFreeVariables(i.Antecedent).Concat(GetFreeVariables(i.Consequent));
                case Exists e: return GetFreeVariables(e.Body).Except(e.Variables.Select(p => p.Name), StringComparer.OrdinalIgnoreCase);
                case Forall f: return GetFreeVariables(f.Body).Except(f.Variables.Select(p => p.Name), StringComparer.OrdinalIgnoreCase);
                default: return Enumerable.Empty<string>();
            }
        }

        private IEnumerable<string> GetFreeVariables(INumericExpression expression)
        {
            switch (expression)
            {
                case FluentCall fluent:
                    return fluent.Arguments.Where(argument => argument.IsVariable).Select(argument => argument.Name);
                case Negate negate:
                    return GetFreeVariables(negate.Operand);
                case Add add:
                    return GetFreeVariables(add.Left).Concat(GetFreeVariables(add.Right));
                case Subtract subtract:
                    return GetFreeVariables(subtract.Left).Concat(GetFreeVariables(subtract.Right));
                case Multiply multiply:
                    return GetFreeVariables(multiply.Left).Concat(GetFreeVariables(multiply.Right));
                case Divide divide:
                    return GetFreeVariables(divide.Left).Concat(GetFreeVariables(divide.Right));
                default:
                    return Enumerable.Empty<string>();
            }
        }

    private IEffect CompileEffect(IEffect effect, ImmutableArray<Parameter> actionParams)
    {
        effect = CompileNestedEffectConditions(effect, actionParams);
        var conditionalEffects = new List<ConditionalEffect>();
        ExtractConditionalEffects(effect, new EmptyLogic(), conditionalEffects);

            // Map conditions through CompilePrecondition to extract axioms
            var compiledEffects = conditionalEffects.Select(c =>
                new ConditionalEffect(CompilePrecondition(c.Condition, actionParams), c.Effect)
            ).Cast<IEffect>().ToImmutableArray();

            // Now conditionalEffects contains a flat list of ConditionalEffect objects
        return new AndEffect(compiledEffects);
    }

    private IEffect CompileNestedEffectConditions(
        IEffect effect,
        ImmutableArray<Parameter> availableParameters)
    {
        return effect switch
        {
            AndEffect andEffect => new AndEffect(andEffect.Effects
                .Select(child => CompileNestedEffectConditions(child, availableParameters))
                .ToImmutableArray()),
            ForallEffect forallEffect => new ForallEffect(
                forallEffect.Variables,
                CompileNestedEffectConditions(
                    forallEffect.Effect,
                    availableParameters.AddRange(forallEffect.Variables))),
            ConditionalEffect conditionalEffect => new ConditionalEffect(
                CompilePrecondition(conditionalEffect.Condition, availableParameters),
                CompileNestedEffectConditions(conditionalEffect.Effect, availableParameters)),
            _ => effect
        };
    }

    private void ExtractConditionalEffects(IEffect current, ILogicalExpression currentCondition, List<ConditionalEffect> results)
        {
            switch (current)
            {
                case AndEffect a:
                    foreach (var e in a.Effects)
                    {
                        ExtractConditionalEffects(e, currentCondition, results);
                    }
                    break;
                case ConditionalEffect c:
                    var combinedCondition = CombineConditions(currentCondition, c.Condition);
                    ExtractConditionalEffects(c.Effect, combinedCondition, results);
                    break;
                case ForallEffect f:
                    results.Add(new ConditionalEffect(currentCondition, f));
                    break;
                case Increase inc:
                    results.Add(new ConditionalEffect(currentCondition, inc));
                    break;
                default:
                    results.Add(new ConditionalEffect(currentCondition, current));
                    break;
            }
        }

        private ILogicalExpression CombineConditions(ILogicalExpression c1, ILogicalExpression c2)
        {
            if (c1 is EmptyLogic) return c2;
            if (c2 is EmptyLogic) return c1;

            var terms = new List<ILogicalExpression>();
            if (c1 is And a1) terms.AddRange(a1.Expressions); else terms.Add(c1);
            if (c2 is And a2) terms.AddRange(a2.Expressions); else terms.Add(c2);

            return new And(terms.ToImmutableArray());
        }
    }
}

public static class CanonicalizationExtensions
{
    public static bool IsCanonical(this DomainDefinition domain)
    {
        return domain.Actions.All(a => a.IsCanonical());
    }

    public static bool IsCanonical(this ActionDefinition action)
    {
        if (action.Precondition == null || !IsCanonicalPrecondition(action.Precondition))
            return false;

        if (action.Effect == null || !IsCanonicalEffect(action.Effect))
            return false;

        return true;
    }

    private static bool IsCanonicalPrecondition(ILogicalExpression expr)
    {
        if (expr is Or || expr is Exists || expr is Forall || expr is Imply)
            return false;

        if (expr is And and)
            return and.Expressions.All(IsCanonicalPrecondition);

        if (expr is Not not)
            return not.Expression is PredicateCall or Equality;

        return true;
    }

    private static bool IsCanonicalEffect(IEffect effect)
    {
        if (effect is not AndEffect andEff)
            return false;

        foreach (var child in andEff.Effects)
        {
            if (child is not ConditionalEffect cond)
                return false;

            if (!IsCanonicalPrecondition(cond.Condition))
                return false;

            if (cond.Effect is ConditionalEffect || cond.Effect is AndEffect)
                return false; // must be a simple effect
        }

        return true;
    }
}
