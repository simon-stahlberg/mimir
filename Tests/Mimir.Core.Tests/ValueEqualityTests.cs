using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

public class ValueEqualityTests
{
    private sealed class StructurallyEqualTerm(string name) : ITerm
    {
        public string Name { get; } = name;

        public override bool Equals(object? obj) => obj is StructurallyEqualTerm other && other.Name == Name;
        public override int GetHashCode() => Name.GetHashCode(StringComparison.Ordinal);
    }

    [Fact]
    public void GroundedExpressions_CompareIndependentOrderedCollectionsByValue()
    {
        Variable first = new("?first");
        Variable second = new("?second");
        var predicate = new Mimir.Core.Schemas.Predicate<Fluent>("linked", [first, second]);
        GroundedAtom atom = new(predicate, new ITerm[] { first, second });
        GroundedAtom equalAtom = new(predicate, new List<ITerm> { first, second });
        GroundedAnd and = new(ImmutableArray.Create<IGroundedExpression>(atom, new GroundedTrue()));
        GroundedAnd equalAnd = new(ImmutableArray.Create<IGroundedExpression>(equalAtom, new GroundedTrue()));
        GroundedOr or = new(ImmutableArray.Create<IGroundedExpression>(atom, new GroundedTrue()));
        GroundedOr equalOr = new(ImmutableArray.Create<IGroundedExpression>(equalAtom, new GroundedTrue()));
        GroundedForall forall = new(new[] { first, second }, and);
        GroundedForall equalForall = new(new List<Variable> { first, second }, equalAnd);
        GroundedExists exists = new(new[] { first, second }, or);
        GroundedExists equalExists = new(new List<Variable> { first, second }, equalOr);

        AssertEqualAndHashCompatible(atom, equalAtom);
        AssertEqualAndHashCompatible(and, equalAnd);
        AssertEqualAndHashCompatible(or, equalOr);
        AssertEqualAndHashCompatible(forall, equalForall);
        AssertEqualAndHashCompatible(exists, equalExists);
    }

    [Fact]
    public void GroundedExpressions_RespectReferenceIdentityOrderAndElements()
    {
        Variable first = new("?value");
        Variable sameNameButForeign = new("?value");
        Variable second = new("?second");
        var predicate = new Mimir.Core.Schemas.Predicate<Fluent>("linked", [first, second]);
        var foreignPredicate = new Mimir.Core.Schemas.Predicate<Fluent>("linked", [first, second]);
        GroundedAtom atom = new(predicate, new ITerm[] { first, second });

        Assert.NotEqual(atom, new GroundedAtom(foreignPredicate, new ITerm[] { first, second }));
        Assert.NotEqual(atom, new GroundedAtom(predicate, new ITerm[] { second, first }));
        Assert.NotEqual(atom, new GroundedAtom(predicate, new ITerm[] { sameNameButForeign, second }));

        GroundedAnd and = new(ImmutableArray.Create<IGroundedExpression>(atom, new GroundedTrue()));
        GroundedOr or = new(ImmutableArray.Create<IGroundedExpression>(atom, new GroundedTrue()));
        Assert.NotEqual(
            and,
            new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(new GroundedTrue(), atom)));
        Assert.NotEqual(
            or,
            new GroundedOr(ImmutableArray.Create<IGroundedExpression>(new GroundedTrue(), atom)));
        Assert.NotEqual(new GroundedForall(new[] { first, second }, and), new GroundedForall(new[] { second, first }, and));
        Assert.NotEqual(new GroundedExists(new[] { first, second }, or), new GroundedExists(new[] { second, first }, or));
    }

    [Fact]
    public void NumericFunctionActionCostExpression_UsesFunctionIdentityAndOrderedArguments()
    {
        Variable first = new("?first");
        Variable second = new("?second");
        var function = new NumericFunction("cost", [first, second]);
        var foreignFunction = new NumericFunction("cost", [first, second]);
        FunctionCall expression = new(function, new ITerm[] { first, second });
        FunctionCall equalExpression = new(
            function,
            new List<ITerm> { first, second });

        AssertEqualAndHashCompatible(expression, equalExpression);
        Assert.NotEqual(
            expression,
            new FunctionCall(function, new ITerm[] { second, first }));
        Assert.NotEqual(
            expression,
            new FunctionCall(foreignFunction, new ITerm[] { first, second }));
    }

    [Fact]
    public void GroundedTermValues_IgnoreCustomStructuralEquality()
    {
        ITerm first = new StructurallyEqualTerm("same");
        ITerm second = new StructurallyEqualTerm("same");
        Variable parameter = new("?value");
        var predicate = new Mimir.Core.Schemas.Predicate<Fluent>("p", [parameter]);
        var function = new NumericFunction("cost", [parameter]);

        Assert.NotSame(first, second);
        Assert.True(first.Equals(second));
        Assert.NotEqual(new GroundedAtom(predicate, [first]), new GroundedAtom(predicate, [second]));
        Assert.NotEqual(
            new FunctionCall(function, [first]),
            new FunctionCall(function, [second]));
    }

    private static void AssertEqualAndHashCompatible<T>(T left, T right)
        where T : notnull
    {
        Assert.Equal(left, right);
        Assert.Equal(left.GetHashCode(), right.GetHashCode());
        Assert.Single(new HashSet<T> { left, right });
    }
}
