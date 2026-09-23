using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Analysis;

namespace Mimir.Core.Schemas;

internal sealed class PddlDomainTranslator
{
    public IReadOnlyList<Predicate<Fluent>> Fluents { get; }
    public IReadOnlyList<Predicate<Static>> Statics { get; }
    public IReadOnlyList<Predicate<Derived>> Derived { get; }
    public IReadOnlyList<NumericFunction> Functions { get; }
    public IReadOnlyList<Constant> Constants { get; }
    public IReadOnlyList<ActionSchema> Actions { get; }
    public IReadOnlyDictionary<string, string> TypeHierarchy { get; }
    public IReadOnlyDictionary<string, IGroundedExpression> DerivedDefinitions { get; }

    internal PddlDomainTranslator(DomainDefinition astDomain)
    {
        if (!astDomain.IsCanonical())
            throw new ArgumentException("The provided AST DomainDefinition is not in canonical form.");

        var domainConstants = new Dictionary<string, Constant>(StringComparer.OrdinalIgnoreCase);
        var allPredicates = new Dictionary<string, Predicate>(StringComparer.OrdinalIgnoreCase);
        var allFunctions = new Dictionary<string, NumericFunction>(StringComparer.OrdinalIgnoreCase);

        Constant GetOrCreateConstant(string name, string type)
        {
            if (domainConstants.TryGetValue(name, out var existing))
                return existing;

            var constant = new Constant(name, type);
            domainConstants[name] = constant;
            return constant;
        }

        Predicate<T> GetOrCreatePredicate<T>(string name, IReadOnlyList<Variable> parameters)
            where T : notnull, IPredicateType
        {
            if (allPredicates.TryGetValue(name, out var existing))
                return (Predicate<T>)existing;

            var predicate = new Predicate<T>(name, parameters);
            allPredicates[name] = predicate;
            return predicate;
        }

        NumericFunction GetOrCreateFunction(string name, IReadOnlyList<Variable> parameters)
        {
            if (allFunctions.TryGetValue(name, out var existing))
                return existing;

            var function = new NumericFunction(name, parameters);
            allFunctions[name] = function;
            return function;
        }

        var expressions = new DomainExpressionTranslator(allPredicates, allFunctions, domainConstants);

        TypeHierarchy = astDomain.Types.ToDictionary(t => t.Name, t => t.ParentType, StringComparer.OrdinalIgnoreCase);

        foreach (var constant in astDomain.Constants)
            GetOrCreateConstant(constant.Name, constant.ParentType);

        Constants = domainConstants.Values.ToList();

        var functionsList = new List<NumericFunction>();
        foreach (var functionDecl in astDomain.Functions)
        {
            var (_, parameters) = DomainExpressionTranslator.ExtendScope(parentScope: null, functionDecl.Parameters);
            NumericFunction function = GetOrCreateFunction(functionDecl.Name, parameters);
            if (!NumericFunction.IsTotalCost(functionDecl.Name))
                functionsList.Add(function);
        }

        Functions = functionsList;

        var fluentsList = new List<Predicate<Fluent>>();
        var staticsList = new List<Predicate<Static>>();
        var derivedList = new List<Predicate<Derived>>();

        var derivedNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var rawDerivedRules = new Dictionary<string, List<DerivedPredicate>>(StringComparer.OrdinalIgnoreCase);
        var derivedParameters = new Dictionary<string, List<Variable>>(StringComparer.OrdinalIgnoreCase);

        foreach (var d in astDomain.DerivedPredicates)
        {
            var name = d.Signature.Name;
            derivedNames.Add(name);

            if (rawDerivedRules.TryGetValue(name, out var rules))
            {
                rules.Add(d);
            }
            else
            {
                rawDerivedRules[name] = [d];
            }
        }

        var fluentNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var action in astDomain.Actions)
            CollectFluentNames(action.Effect, fluentNames);

        foreach (var pDecl in astDomain.Predicates)
        {
            var (_, parameters) = DomainExpressionTranslator.ExtendScope(parentScope: null, pDecl.Parameters);

            if (derivedNames.Contains(pDecl.Name))
            {
                derivedParameters[pDecl.Name] = parameters;
                derivedList.Add(GetOrCreatePredicate<Derived>(pDecl.Name, parameters));
                continue;
            }

            if (fluentNames.Contains(pDecl.Name))
                fluentsList.Add(GetOrCreatePredicate<Fluent>(pDecl.Name, parameters));
            else
                staticsList.Add(GetOrCreatePredicate<Static>(pDecl.Name, parameters));
        }

        if (astDomain.Requirements.HasRequirement(PddlRequirement.Equality))
        {
            staticsList.Add(GetOrCreatePredicate<Static>("=", new[]
            {
                new Variable("?left", "object"),
                new Variable("?right", "object"),
            }));
        }

        foreach (DerivedPredicate rule in astDomain.DerivedPredicates)
        {
            string name = rule.Signature.Name;
            if (derivedParameters.ContainsKey(name)) continue;

            var (_, parameters) = DomainExpressionTranslator.ExtendScope(parentScope: null, rule.Signature.Parameters);
            derivedParameters[name] = parameters;
            derivedList.Add(GetOrCreatePredicate<Derived>(name, parameters));
        }

        Fluents = fluentsList;
        Statics = staticsList;
        Derived = derivedList;

        var groundedDefs = new Dictionary<string, IGroundedExpression>(StringComparer.OrdinalIgnoreCase);
        foreach (Predicate<Derived> predicate in derivedList)
        {
            string name = predicate.Name;
            List<DerivedPredicate> rules = rawDerivedRules[name];
            List<Variable> canonicalParameters = derivedParameters[name];
            var bodies = new List<IGroundedExpression>(rules.Count);

            foreach (DerivedPredicate rule in rules)
            {
                var ruleScope = new Dictionary<string, Variable>(StringComparer.OrdinalIgnoreCase);
                for (int index = 0; index < rule.Signature.Parameters.Length; index++)
                    ruleScope[rule.Signature.Parameters[index].Name] = canonicalParameters[index];

                bodies.Add(expressions.TranslateExpression(rule.Body, ruleScope));
            }

            groundedDefs[name] = bodies.Count == 1
                ? bodies[0]
                : new GroundedOr(bodies.ToImmutableArray());
        }

        DerivedDefinitions = groundedDefs;

        double defaultActionCost = astDomain.Requirements.HasRequirement(PddlRequirement.ActionCosts) ? 0d : 1d;
        var actionsList = new List<ActionSchema>(astDomain.Actions.Length);
        foreach (ActionDefinition action in astDomain.Actions)
        {
            var (parameterScope, parameters) = DomainExpressionTranslator.ExtendScope(parentScope: null, action.Parameters);

            var precondition = new ExtractedCondition();
            if (action.Precondition != null)
                expressions.ExtractCondition(action.Precondition, Polarity.Positive, parameterScope, precondition);

            var effects = new ExtractedEffects();
            if (action.Effect is not null)
            {
                if (action.Effect is not AndEffect andEff)
                    throw new InvalidOperationException(
                        $"Canonical action '{action.Name}' has an unexpected effect shape.");

                foreach (var child in andEff.Effects)
                {
                    if (child is not Mimir.Pddl.Ast.Effects.ConditionalEffect cond)
                        throw new InvalidOperationException(
                            $"Canonical action '{action.Name}' contains an unexpected effect '{child.GetType().Name}'.");

                    expressions.ExtractEffects(cond, parameterScope, effects);
                }
            }

            NumericExpression cost = effects.CostIncreases.Count == 0
                ? new NumericConstant(defaultActionCost)
                : effects.CostIncreases.Aggregate((left, right) => new NumericBinaryExpression(NumericOperator.Add, left, right));

            actionsList.Add(new ActionSchema(
                action.Name,
                parameters,
                precondition.Fluent,
                precondition.Static,
                precondition.Derived,
                precondition.NumericConditions,
                effects.Effects.OfType<ConditionalEffect>().ToArray(),
                effects.Effects.OfType<ConditionalNumericEffect>().ToArray(),
                cost));
        }

        Actions = actionsList;
    }

    private static void CollectFluentNames(IEffect? effect, HashSet<string> fluentNames)
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
