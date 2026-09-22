namespace Mimir.Core.Engines;

using Grounding;
using Schemas;

internal sealed record DerivedPredicateComponent(
    IReadOnlyList<Predicate<Derived>> Predicates,
    bool IsStateDependent,
    bool RequiresFixedPoint,
    IReadOnlyList<IReadOnlyList<int>> PositiveDependentRuleIndicesByPredicate);

internal readonly record struct DerivedDependency(
    Predicate<Derived> Predicate,
    bool IsNegative);

internal sealed class DerivedPredicatePlan
{
    private readonly Dictionary<Predicate<Derived>, bool> _stateDependent;
    private readonly Dictionary<Predicate<Derived>, DerivedPredicateComponent> _componentsByPredicate;

    internal IReadOnlyList<DerivedPredicateComponent> Components { get; }
    internal bool HasNumericConditions { get; }

    private DerivedPredicatePlan(
        IReadOnlyList<DerivedPredicateComponent> components,
        Dictionary<Predicate<Derived>, bool> stateDependent,
        bool hasNumericConditions)
    {
        Components = components;
        HasNumericConditions = hasNumericConditions;
        _stateDependent = stateDependent;
        _componentsByPredicate = new Dictionary<Predicate<Derived>, DerivedPredicateComponent>(
            ReferenceEqualityComparer.Instance);
        foreach (DerivedPredicateComponent component in components)
        {
            foreach (Predicate<Derived> predicate in component.Predicates)
                _componentsByPredicate.Add(predicate, component);
        }
    }

    internal bool IsStateDependent(Predicate<Derived> predicate)
        => _stateDependent[predicate];

    internal DerivedPredicateComponent GetComponent(Predicate<Derived> predicate)
        => _componentsByPredicate[predicate];

    internal static DerivedPredicatePlan Create(Domain domain)
    {
        var dependencies =
            new Dictionary<Predicate<Derived>, List<DerivedDependency>>(
                ReferenceEqualityComparer.Instance);
        var directlyStateDependent =
            new Dictionary<Predicate<Derived>, bool>(
                ReferenceEqualityComparer.Instance);

        bool hasNumericConditions = false;
        foreach (Predicate<Derived> predicate in domain.Derived)
        {
            var predicateDependencies = new List<DerivedDependency>();
            dependencies.Add(predicate, predicateDependencies);
            var comparisons = new List<NumericComparison>();
            bool readsFluents = CollectDependencies(domain.DerivedDefinitions[predicate.Name],
                predicateDependencies, comparisons, isNegative: false);
            directlyStateDependent.Add(predicate, readsFluents || comparisons.Any(comparison =>
                NumericEvaluation.DependsOnState(comparison, domain.ChangingFunctions)));
            hasNumericConditions |= comparisons.Count > 0;
        }

        IReadOnlyList<IReadOnlyList<Predicate<Derived>>> componentPredicates =
            FindComponents(domain.Derived, dependencies);
        var components = new List<DerivedPredicateComponent>(componentPredicates.Count);
        var stateDependent =
            new Dictionary<Predicate<Derived>, bool>(
                ReferenceEqualityComparer.Instance);

        for (int i = 0; i < componentPredicates.Count; i++)
        {
            IReadOnlyList<Predicate<Derived>> members = componentPredicates[i];
            var memberSet = new HashSet<Predicate<Derived>>(
                members,
                ReferenceEqualityComparer.Instance);
            bool componentIsStateDependent = false;

            foreach (Predicate<Derived> predicate in members)
            {
                componentIsStateDependent |= directlyStateDependent[predicate];
                foreach (DerivedDependency dependency in dependencies[predicate])
                {
                    if (memberSet.Contains(dependency.Predicate))
                    {
                        RejectNegativeRecursion(predicate, dependency);
                        continue;
                    }

                    componentIsStateDependent |= stateDependent[dependency.Predicate];
                }
            }

            components.Add(new DerivedPredicateComponent(
                members.ToArray(),
                componentIsStateDependent,
                RequiresFixedPoint(members, dependencies),
                BuildPositiveDependentRuleIndices(members, dependencies)));
            foreach (Predicate<Derived> predicate in members)
                stateDependent.Add(predicate, componentIsStateDependent);
        }

        return new DerivedPredicatePlan(components.AsReadOnly(), stateDependent, hasNumericConditions);
    }

    private static IReadOnlyList<IReadOnlyList<int>> BuildPositiveDependentRuleIndices(
        IReadOnlyList<Predicate<Derived>> members,
        IReadOnlyDictionary<Predicate<Derived>, List<DerivedDependency>> dependencies)
    {
        var memberIndices = new Dictionary<Predicate<Derived>, int>(
            ReferenceEqualityComparer.Instance);
        var dependents = new List<int>[members.Count];
        for (int i = 0; i < members.Count; i++)
        {
            memberIndices.Add(members[i], i);
            dependents[i] = new List<int>();
        }

        for (int sourceIndex = 0; sourceIndex < members.Count; sourceIndex++)
        {
            foreach (DerivedDependency dependency in dependencies[members[sourceIndex]])
            {
                if (dependency.IsNegative
                    || !memberIndices.TryGetValue(dependency.Predicate, out int targetIndex)
                    || dependents[targetIndex].Contains(sourceIndex))
                {
                    continue;
                }

                dependents[targetIndex].Add(sourceIndex);
            }
        }

        var result = new IReadOnlyList<int>[dependents.Length];
        for (int i = 0; i < dependents.Length; i++)
            result[i] = Array.AsReadOnly(dependents[i].ToArray());
        return Array.AsReadOnly(result);
    }

    private static bool RequiresFixedPoint(
        IReadOnlyList<Predicate<Derived>> members,
        IReadOnlyDictionary<Predicate<Derived>, List<DerivedDependency>> dependencies)
    {
        if (members.Count > 1)
            return true;

        Predicate<Derived> predicate = members[0];
        foreach (DerivedDependency dependency in dependencies[predicate])
        {
            if (ReferenceEquals(dependency.Predicate, predicate))
                return true;
        }

        return false;
    }

    private static bool CollectDependencies(
        IGroundedExpression expression,
        List<DerivedDependency> dependencies,
        List<NumericComparison> comparisons,
        bool isNegative)
    {
        switch (expression)
        {
            case NumericComparison comparison:
                comparisons.Add(comparison);
                return false;
            case GroundedTrue:
                return false;
            case GroundedAtom { Predicate: Predicate<Fluent> }:
                return true;
            case GroundedAtom { Predicate: Predicate<Derived> predicate }:
                dependencies.Add(new DerivedDependency(predicate, isNegative));
                return false;
            case GroundedAtom:
                return false;
            case GroundedNot not:
                return CollectDependencies(not.Expression, dependencies, comparisons, !isNegative);
            case GroundedAnd and:
                return AnyStateDependency(and.Expressions, dependencies, comparisons, isNegative);
            case GroundedOr or:
                return AnyStateDependency(or.Expressions, dependencies, comparisons, isNegative);
            case GroundedImply imply:
                bool antecedent = CollectDependencies(
                    imply.Antecedent,
                    dependencies,
                    comparisons,
                    !isNegative);
                bool consequent = CollectDependencies(
                    imply.Consequent,
                    dependencies,
                    comparisons,
                    isNegative);
                return antecedent || consequent;
            case GroundedForall forall:
                return CollectDependencies(forall.Body, dependencies, comparisons, isNegative);
            case GroundedExists exists:
                return CollectDependencies(exists.Body, dependencies, comparisons, isNegative);
            default:
                throw new InvalidOperationException(expression.GetType().Name);
        }
    }

    private static bool AnyStateDependency(
        IEnumerable<IGroundedExpression> expressions,
        List<DerivedDependency> dependencies,
        List<NumericComparison> comparisons,
        bool isNegative)
    {
        bool found = false;
        foreach (IGroundedExpression expression in expressions)
            found |= CollectDependencies(expression, dependencies, comparisons, isNegative);
        return found;
    }

    private static IReadOnlyList<IReadOnlyList<Predicate<Derived>>> FindComponents(
        IReadOnlyList<Predicate<Derived>> predicates,
        IReadOnlyDictionary<Predicate<Derived>, List<DerivedDependency>> dependencies)
    {
        int nextIndex = 0;
        var indices = new Dictionary<Predicate<Derived>, int>(
            ReferenceEqualityComparer.Instance);
        var lowLinks = new Dictionary<Predicate<Derived>, int>(
            ReferenceEqualityComparer.Instance);
        var onStack = new HashSet<Predicate<Derived>>(
            ReferenceEqualityComparer.Instance);
        var stack = new Stack<Predicate<Derived>>();
        var components = new List<IReadOnlyList<Predicate<Derived>>>();

        foreach (Predicate<Derived> predicate in predicates)
        {
            if (!indices.ContainsKey(predicate))
            {
                Visit(
                    predicate,
                    dependencies,
                    indices,
                    lowLinks,
                    onStack,
                    stack,
                    components,
                    ref nextIndex);
            }
        }

        return components;
    }

    private static void Visit(
        Predicate<Derived> predicate,
        IReadOnlyDictionary<Predicate<Derived>, List<DerivedDependency>> dependencies,
        Dictionary<Predicate<Derived>, int> indices,
        Dictionary<Predicate<Derived>, int> lowLinks,
        HashSet<Predicate<Derived>> onStack,
        Stack<Predicate<Derived>> stack,
        List<IReadOnlyList<Predicate<Derived>>> components,
        ref int nextIndex)
    {
        int index = nextIndex++;
        indices.Add(predicate, index);
        lowLinks.Add(predicate, index);
        stack.Push(predicate);
        onStack.Add(predicate);

        foreach (DerivedDependency dependency in dependencies[predicate])
        {
            Predicate<Derived> target = dependency.Predicate;
            if (!indices.ContainsKey(target))
            {
                Visit(
                    target, dependencies, indices, lowLinks,
                    onStack, stack, components, ref nextIndex);
                lowLinks[predicate] = Math.Min(lowLinks[predicate], lowLinks[target]);
            }
            else if (onStack.Contains(target))
            {
                lowLinks[predicate] = Math.Min(lowLinks[predicate], indices[target]);
            }
        }

        if (lowLinks[predicate] != indices[predicate])
            return;

        var component = new List<Predicate<Derived>>();
        Predicate<Derived> member;
        do
        {
            member = stack.Pop();
            onStack.Remove(member);
            component.Add(member);
        }
        while (!ReferenceEquals(member, predicate));

        components.Add(component.AsReadOnly());
    }

    private static void RejectNegativeRecursion(
        Predicate<Derived> predicate,
        DerivedDependency dependency)
    {
        if (!dependency.IsNegative)
            return;

        throw new NotSupportedException(
            $"Unstratified recursive derived predicates are not supported. "
            + $"Negative dependency: '{predicate.Name}' -> '{dependency.Predicate.Name}'.");
    }
}
