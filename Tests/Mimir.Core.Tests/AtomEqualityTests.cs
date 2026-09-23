using Mimir.Core.Schemas;
using Xunit;

namespace Mimir.Core.Tests;

public class AtomEqualityTests
{
    private static Mimir.Core.Schemas.Predicate<Fluent> MakePredicate(string name, params Variable[] parameters)
        => new(name, parameters);

    private static Variable Var(string name) => new Variable(name);

    [Fact]
    public void Equals_Null_ReturnsFalse()
    {
        var pred = MakePredicate("p");
        var atom = new Atom<Fluent>(pred, Array.Empty<ITerm>());
        Assert.False(atom.Equals(null));
    }

    [Fact]
    public void Equals_DifferentPredicate_ReturnsFalse()
    {
        var pred0 = MakePredicate("p");
        var pred1 = MakePredicate("p");
        var atom0 = new Atom<Fluent>(pred0, Array.Empty<ITerm>());
        var atom1 = new Atom<Fluent>(pred1, Array.Empty<ITerm>());
        Assert.False(atom0.Equals(atom1));
    }

    [Fact]
    public void Equals_ArityMismatch_ReturnsFalse()
    {
        var pred = MakePredicate("p", Var("?x"));
        var atom0 = new Atom<Fluent>(pred, Array.Empty<ITerm>());
        var atom1 = new Atom<Fluent>(pred, new ITerm[] { Var("?x") });
        Assert.False(atom0.Equals(atom1));
    }

    [Fact]
    public void Equals_ArgumentNameMismatch_ReturnsFalse()
    {
        var pred = MakePredicate("p", Var("?x"));
        var atom0 = new Atom<Fluent>(pred, new ITerm[] { Var("?x") });
        var atom1 = new Atom<Fluent>(pred, new ITerm[] { Var("?y") });
        Assert.False(atom0.Equals(atom1));
    }

    [Fact]
    public void Equals_SamePredicateAndTermReferences_ReturnsTrue()
    {
        Variable variable = Var("?x");
        var pred = MakePredicate("p", variable);
        var atom0 = new Atom<Fluent>(pred, new ITerm[] { variable });
        var atom1 = new Atom<Fluent>(pred, new ITerm[] { variable });
        Assert.True(atom0.Equals(atom1));
    }

    [Fact]
    public void GetHashCode_EqualAtoms_ReturnSameHash()
    {
        Variable variable = Var("?x");
        var pred = MakePredicate("p", variable);
        var atom0 = new Atom<Fluent>(pred, new ITerm[] { variable });
        var atom1 = new Atom<Fluent>(pred, new ITerm[] { variable });
        Assert.Equal(atom0.GetHashCode(), atom1.GetHashCode());
    }

}
