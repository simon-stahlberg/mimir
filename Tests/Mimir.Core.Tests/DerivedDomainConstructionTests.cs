using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public sealed class DerivedDomainConstructionTests
{
    [Fact]
    public void ManualDomainPairsDerivedPredicatesWithDefinitions()
    {
        var predicate = new DerivedPredicate("ready", Array.Empty<Variable>());
        var definition = new DerivedPredicateDefinition(predicate, new GroundedTrue());

        Domain domain = CreateDomain([definition]);

        Assert.Same(predicate, Assert.Single(domain.Derived));
        Assert.IsType<GroundedTrue>(domain.DerivedDefinitions["READY"]);
    }

    [Fact]
    public void ManualDomainRejectsDuplicateDerivedNames()
    {
        var first = new DerivedPredicate("ready", Array.Empty<Variable>());
        var second = new DerivedPredicate("READY", Array.Empty<Variable>());

        ArgumentException exception = Assert.Throws<ArgumentException>(() => CreateDomain([
            new DerivedPredicateDefinition(first, new GroundedTrue()),
            new DerivedPredicateDefinition(second, new GroundedTrue())
        ]));

        Assert.Contains("defined more than once", exception.Message);
    }

    [Fact]
    public void ManualDomainAcceptsPositiveRecursiveDerivedDefinition()
    {
        var predicate = new DerivedPredicate("loop", Array.Empty<Variable>());
        var body = new GroundedAtom(predicate, Array.Empty<ITerm>());

        Domain domain = CreateDomain([
            new DerivedPredicateDefinition(predicate, body)
        ]);

        Assert.Same(predicate, Assert.Single(domain.Derived));
        Assert.Same(body, domain.DerivedDefinitions["loop"]);
    }

    [Fact]
    public void ManualDomainRejectsNegativeRecursiveDerivedDefinition()
    {
        var predicate = new DerivedPredicate("loop", Array.Empty<Variable>());
        var body = new GroundedNot(
            new GroundedAtom(predicate, Array.Empty<ITerm>()));

        NotSupportedException exception = Assert.Throws<NotSupportedException>(() => CreateDomain([
            new DerivedPredicateDefinition(predicate, body)
        ]));

        Assert.Contains("Unstratified recursive derived predicates", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsReferenceWithoutDefinition()
    {
        var defined = new DerivedPredicate("defined", Array.Empty<Variable>());
        var missing = new DerivedPredicate("missing", Array.Empty<Variable>());
        var body = new GroundedAtom(missing, Array.Empty<ITerm>());

        ArgumentException exception = Assert.Throws<ArgumentException>(() => CreateDomain([
            new DerivedPredicateDefinition(defined, body)
        ]));

        Assert.Contains("Undeclared predicate 'missing'", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsObjectHierarchyEntry()
    {
        var hierarchy = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["OBJECT"] = "other"
        };

        ArgumentException exception = Assert.Throws<ArgumentException>(() => CreateDomain([], hierarchy));

        Assert.Contains("built-in root type 'object'", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsUndeclaredParentType()
    {
        var hierarchy = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["child"] = "missing"
        };

        ArgumentException exception = Assert.Throws<ArgumentException>(() => CreateDomain([], hierarchy));

        Assert.Contains("Undeclared type 'missing'", exception.Message);
    }

    [Fact]
    public void ManualDomainPreservesFluentAndStaticReferences()
    {
        var fluent = new FluentPredicate("fluent", Array.Empty<Variable>());
        var stat = new StaticPredicate("static", Array.Empty<Variable>());

        Domain domain = new(
            "reference-identity",
            [fluent],
            [stat],
            Array.Empty<DerivedPredicateDefinition>(),
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            new Dictionary<string, string>());

        Assert.Same(fluent, Assert.Single(domain.Fluents));
        Assert.Same(stat, Assert.Single(domain.Statics));
    }

    [Fact]
    public void ManualDomainRejectsCaseInsensitiveDuplicateActionNames()
    {
        static ActionSchema CreateAction(string name) => new(
            name,
            Array.Empty<Variable>(),
            Array.Empty<Literal<Atom<Fluent>>>(),
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            Array.Empty<ConditionalEffect>(),
            new ConstantActionCostExpression(1d));

        ArgumentException exception = Assert.Throws<ArgumentException>(() => new Domain(
            "duplicate-actions",
            Array.Empty<FluentPredicate>(),
            Array.Empty<StaticPredicate>(),
            Array.Empty<DerivedPredicateDefinition>(),
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            [CreateAction("move"), CreateAction("MOVE")],
            new Dictionary<string, string>()));

        Assert.Contains("Duplicate action declaration 'MOVE'", exception.Message);
    }

    private static Domain CreateDomain(
        IReadOnlyList<DerivedPredicateDefinition> definitions,
        IReadOnlyDictionary<string, string>? hierarchy = null)
    {
        return new Domain(
            "manual",
            Array.Empty<FluentPredicate>(),
            Array.Empty<StaticPredicate>(),
            definitions,
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            hierarchy ?? new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase));
    }
}
