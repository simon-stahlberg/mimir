using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Core.Schemas;

internal static class ProblemGoalExtractor
{
    public static void ExtractGoalLiterals(
        ILogicalExpression expression,
        IReadOnlyDictionary<string, Predicate> allPredicates,
        IReadOnlyDictionary<string, Constant> objectLookup,
        InstanceContext context,
        List<Literal<Fact>> results,
        IReadOnlyDictionary<string, NumericFunction> functions, List<NumericComparison> comparisons)
    {
        switch (expression)
        {
            case Comparison comparison:
                var scope = new Dictionary<string, Variable>();
                ITerm MapTerm(Mimir.Pddl.Ast.Models.Term term, Dictionary<string, Variable> _) => objectLookup[term.Name];
                comparisons.Add(NumericExpressionTranslator.TranslateComparison(comparison, scope, functions, MapTerm));
                return;
            case EmptyLogic:
                return;

            case And and:
                foreach (var child in and.Expressions)
                    ExtractGoalLiterals(child, allPredicates, objectLookup, context, results, functions, comparisons);
                return;

            case PredicateCall predicateCall:
                results.Add(CreateLiteral(
                    predicateCall.Name,
                    predicateCall.Arguments,
                    Polarity.Positive,
                    allPredicates,
                    objectLookup,
                    context));
                return;

            case Not { Expression: PredicateCall predicateCall }:
                results.Add(CreateLiteral(
                    predicateCall.Name,
                    predicateCall.Arguments,
                    Polarity.Negative,
                    allPredicates,
                    objectLookup,
                    context));
                return;

            case Equality equality:
                results.Add(CreateLiteral(
                    "=",
                    [equality.Left, equality.Right],
                    Polarity.Positive,
                    allPredicates,
                    objectLookup,
                    context));
                return;

            case Not { Expression: Equality equality }:
                results.Add(CreateLiteral(
                    "=",
                    [equality.Left, equality.Right],
                    Polarity.Negative,
                    allPredicates,
                    objectLookup,
                    context));
                return;

            default:
                throw new NotSupportedException(
                    $"Core problem goals must be conjunctions of predicate literals and numeric comparisons; '{expression.GetType().Name}' cannot be represented.");
        }
    }

    private static Literal<Fact> CreateLiteral(
        string predicateName,
        IEnumerable<Mimir.Pddl.Ast.Models.Term> terms,
        Polarity polarity,
        IReadOnlyDictionary<string, Predicate> allPredicates,
        IReadOnlyDictionary<string, Constant> objectLookup,
        InstanceContext context)
    {
        if (!allPredicates.TryGetValue(predicateName, out Predicate? predicate))
        {
            throw new InvalidOperationException(
                $"Validated problem goal references undeclared predicate '{predicateName}'.");
        }

        var arguments = new List<Constant>();
        foreach (var argument in terms)
        {
            if (!objectLookup.TryGetValue(argument.Name, out Constant? constant))
            {
                throw new InvalidOperationException(
                    $"Validated problem goal references undeclared object '{argument.Name}'.");
            }

            arguments.Add(constant);
        }

        Fact fact = predicate switch
        {
            Predicate<Fluent> fluent => context.RegisterFact(fluent, arguments),
            Predicate<Static> stat => context.RegisterFact(stat, arguments),
            Predicate<Derived> derived => context.RegisterFact(derived, arguments),
            _ => throw new InvalidOperationException(
                $"Predicate '{predicate.Name}' has unsupported runtime type '{predicate.GetType().Name}'.")
        };

        return new Literal<Fact>(fact, polarity);
    }
}
