using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public class EqualityPredicateTests
{
    [Fact]
    public void EqualityIsAnOrdinaryStaticPredicate()
    {
        Problem problem = CreateProblem("a b");
        StaticPredicate equals = Assert.IsType<StaticPredicate>(problem.AllPredicates["="]);

        Assert.Contains(equals, problem.Domain.Statics);
        Assert.Equal(2, equals.Parameters.Count);
        Assert.All(equals.Parameters, parameter => Assert.Equal("object", parameter.Type));
    }

    [Fact]
    public void NegativeEqualityLiteralFiltersBindingsLikeAnyOtherStaticLiteral()
    {
        Problem problem = CreateProblem("a b c d");
        StaticPredicate equals = Assert.IsType<StaticPredicate>(problem.AllPredicates["="]);
        Variable x = problem.NewVariable("?x");
        Variable y = problem.NewVariable("?y");
        Variable z = problem.NewVariable("?z");
        Literal<Atom<Static>> xy = problem.NewLiteral(problem.NewAtom(equals, [x, y]), false);
        Literal<Atom<Static>> xz = problem.NewLiteral(problem.NewAtom(equals, [x, z]), false);
        Literal<Atom<Static>> yz = problem.NewLiteral(problem.NewAtom(equals, [y, z]), false);
        ConjunctiveCondition condition = problem.NewConjunctiveCondition(
            [x, y, z],
            [xy, xz, yz],
            []);
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition compiled = generator.Compile(condition, problem);

        var bindings = new List<Constant[]>();
        ExtendedState initialState = problem.InitialState.Expand();
        int count = generator.EnumerateBindings(compiled, initialState, binding =>
        {
            bindings.Add(binding.ToArray());
            return true;
        });

        Assert.Equal(24, count);
        Assert.All(bindings, binding => Assert.Equal(3, binding.Distinct(ReferenceEqualityComparer.Instance).Count()));
        Assert.Equal(2, generator.CountBindings(compiled, initialState, 2));
    }

    [Fact]
    public void PositiveAndNegativeEqualityUseIdentityTruth()
    {
        Problem problem = CreateProblem("a b");
        StaticPredicate equals = Assert.IsType<StaticPredicate>(problem.AllPredicates["="]);
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Fact<Static> aa = problem.Context.RegisterFact(equals, [a, a]);
        Fact<Static> ab = problem.Context.RegisterFact(equals, [a, b]);

        ExtendedState initialState = problem.InitialState.Expand();
        Assert.True(initialState.IsTrue(aa));
        Assert.False(initialState.IsTrue(ab));
    }

    [Fact]
    public void DomainWithoutEqualityHasNoEqualityPredicate()
    {
        Domain domain = new DomainBuilder("no-equality")
            .Requirements().Add(":strips").Close()
            .Actions().Add("noop").Close().Close()
            .Build();

        Assert.DoesNotContain(domain.Statics, predicate => predicate.Name == "=");
    }

    private static Problem CreateProblem(string objects)
    {
        Domain domain = new DomainBuilder("inequalities")
            .Requirements().Add(":strips").Add(":equality").Close()
            .Actions().Add("noop").Close().Close()
            .Build();
        var problemBuilder = new ProblemBuilder(domain, "inequality-problem");
        ProblemObjectListBuilder objectBuilder = problemBuilder.Objects();
        foreach (string name in objects.Split(' ', StringSplitOptions.RemoveEmptyEntries))
            objectBuilder.Add(name);
        objectBuilder.Close();
        return problemBuilder.Build();
    }
}
