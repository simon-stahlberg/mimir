using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

internal static class DerivedDependencyValidator
{
    public static void Validate(DomainDefinition domain)
    {
        var derivedNames = domain.DerivedPredicates
            .Select(definition => definition.Signature.Name)
            .ToHashSet(StringComparer.OrdinalIgnoreCase);
        var dependencies = derivedNames.ToDictionary(
            name => name,
            _ => new List<Dependency>(),
            StringComparer.OrdinalIgnoreCase);

        foreach (DerivedPredicate definition in domain.DerivedPredicates)
        {
            CollectDependencies(
                definition.Body,
                derivedNames,
                dependencies[definition.Signature.Name],
                isNegative: false);
        }

        ValidateGraph(dependencies);
    }

    public static void Validate(IReadOnlyDictionary<string, IGroundedExpression> definitions)
    {
        var names = definitions.Keys.ToHashSet(StringComparer.OrdinalIgnoreCase);
        var dependencies = names.ToDictionary(
            name => name,
            _ => new List<Dependency>(),
            StringComparer.OrdinalIgnoreCase);

        foreach ((string name, IGroundedExpression body) in definitions)
            CollectDependencies(body, dependencies[name], isNegative: false);

        ValidateGraph(dependencies);
    }

    private static void CollectDependencies(
        ILogicalExpression expression,
        IReadOnlySet<string> derivedNames,
        List<Dependency> dependencies,
        bool isNegative)
    {
        switch (expression)
        {
            case PredicateCall predicateCall when derivedNames.Contains(predicateCall.Name):
                dependencies.Add(new Dependency(predicateCall.Name, isNegative));
                return;
            case PredicateCall or Equality or Comparison or EmptyLogic:
                return;
            case Not not:
                CollectDependencies(not.Expression, derivedNames, dependencies, !isNegative);
                return;
            case And and:
                foreach (ILogicalExpression child in and.Expressions)
                    CollectDependencies(child, derivedNames, dependencies, isNegative);
                return;
            case Or or:
                foreach (ILogicalExpression child in or.Expressions)
                    CollectDependencies(child, derivedNames, dependencies, isNegative);
                return;
            case Imply imply:
                CollectDependencies(imply.Antecedent, derivedNames, dependencies, !isNegative);
                CollectDependencies(imply.Consequent, derivedNames, dependencies, isNegative);
                return;
            case Forall forall:
                CollectDependencies(forall.Body, derivedNames, dependencies, isNegative);
                return;
            case Exists exists:
                CollectDependencies(exists.Body, derivedNames, dependencies, isNegative);
                return;
            default:
                throw new InvalidOperationException(
                    $"Unknown derived expression '{expression.GetType().Name}'.");
        }
    }

    private static void CollectDependencies(
        IGroundedExpression expression,
        List<Dependency> dependencies,
        bool isNegative)
    {
        switch (expression)
        {
            case GroundedAtom { Predicate: Predicate<Derived> predicate }:
                dependencies.Add(new Dependency(predicate.Name, isNegative));
                return;
            case GroundedAtom or GroundedTrue:
                return;
            case GroundedNot not:
                CollectDependencies(not.Expression, dependencies, !isNegative);
                return;
            case GroundedAnd and:
                foreach (IGroundedExpression child in and.Expressions)
                    CollectDependencies(child, dependencies, isNegative);
                return;
            case GroundedOr or:
                foreach (IGroundedExpression child in or.Expressions)
                    CollectDependencies(child, dependencies, isNegative);
                return;
            case GroundedImply imply:
                CollectDependencies(imply.Antecedent, dependencies, !isNegative);
                CollectDependencies(imply.Consequent, dependencies, isNegative);
                return;
            case GroundedForall forall:
                CollectDependencies(forall.Body, dependencies, isNegative);
                return;
            case GroundedExists exists:
                CollectDependencies(exists.Body, dependencies, isNegative);
                return;
            default:
                throw new InvalidOperationException(
                    $"Unknown grounded derived expression '{expression.GetType().Name}'.");
        }
    }

    private static void ValidateGraph(IReadOnlyDictionary<string, List<Dependency>> dependencies)
    {
        foreach ((string name, List<Dependency> predicateDependencies) in dependencies)
        {
            foreach (Dependency dependency in predicateDependencies)
            {
                if (!dependencies.ContainsKey(dependency.Name))
                    throw new InvalidOperationException(dependency.Name);
            }
        }

        Dictionary<string, int> components = FindComponents(dependencies);
        foreach ((string name, List<Dependency> predicateDependencies) in dependencies)
        {
            foreach (Dependency dependency in predicateDependencies)
            {
                if (dependency.IsNegative
                    && components[name] == components[dependency.Name])
                {
                    throw new NotSupportedException(
                        $"Unstratified recursive derived predicates are not supported. "
                        + $"Negative dependency: '{name}' -> '{dependency.Name}'.");
                }
            }
        }
    }

    private static Dictionary<string, int> FindComponents(
        IReadOnlyDictionary<string, List<Dependency>> dependencies)
    {
        int nextIndex = 0;
        int nextComponent = 0;
        var indices = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);
        var lowLinks = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);
        var onStack = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var stack = new Stack<string>();
        var components = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);

        foreach (string name in dependencies.Keys)
        {
            if (!indices.ContainsKey(name))
            {
                Visit(
                    name,
                    dependencies,
                    indices,
                    lowLinks,
                    onStack,
                    stack,
                    components,
                    ref nextIndex,
                    ref nextComponent);
            }
        }

        return components;
    }

    private static void Visit(
        string name,
        IReadOnlyDictionary<string, List<Dependency>> dependencies,
        Dictionary<string, int> indices,
        Dictionary<string, int> lowLinks,
        HashSet<string> onStack,
        Stack<string> stack,
        Dictionary<string, int> components,
        ref int nextIndex,
        ref int nextComponent)
    {
        int index = nextIndex++;
        indices.Add(name, index);
        lowLinks.Add(name, index);
        stack.Push(name);
        onStack.Add(name);

        foreach (Dependency dependency in dependencies[name])
        {
            if (!indices.ContainsKey(dependency.Name))
            {
                Visit(
                    dependency.Name,
                    dependencies,
                    indices,
                    lowLinks,
                    onStack,
                    stack,
                    components,
                    ref nextIndex,
                    ref nextComponent);
                lowLinks[name] = Math.Min(lowLinks[name], lowLinks[dependency.Name]);
            }
            else if (onStack.Contains(dependency.Name))
            {
                lowLinks[name] = Math.Min(lowLinks[name], indices[dependency.Name]);
            }
        }

        if (lowLinks[name] != indices[name])
            return;

        while (true)
        {
            string member = stack.Pop();
            onStack.Remove(member);
            components.Add(member, nextComponent);
            if (member.Equals(name, StringComparison.OrdinalIgnoreCase))
                break;
        }

        nextComponent++;
    }

    private readonly record struct Dependency(string Name, bool IsNegative);

}
