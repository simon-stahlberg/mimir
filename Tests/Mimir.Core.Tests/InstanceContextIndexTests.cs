using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

public class InstanceContextIndexTests
{
    [Fact]
    public void RegisterFactArityOverloadsReuseFactsAndPreserveValidation()
    {
        Domain domain = new DomainBuilder("fact-arities")
            .Requirements().Add(":strips").Add(":typing").Close()
            .Types().Add("thing").Add("other").Close()
            .Predicates()
                .Add("p0")
                .Add("p1", ("?a", "thing"))
                .Add("p2", ("?a", "thing"), ("?b", "thing"))
                .Add("p3", ("?a", "thing"), ("?b", "thing"), ("?c", "thing"))
                .Add("p4", ("?a", "thing"), ("?b", "thing"), ("?c", "thing"), ("?d", "thing"))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "fact-arities-problem")
            .Objects()
                .Add("a", "thing").Add("b", "thing").Add("c", "thing").Add("d", "thing")
                .Add("wrong", "other")
                .Close()
            .Build();
        InstanceContext context = problem.Context;
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Constant c = problem.ObjectLookup["c"];
        Constant d = problem.ObjectLookup["d"];
        Constant wrong = problem.ObjectLookup["wrong"];
        Mimir.Core.Schemas.Predicate<Static> p0 = GetStatic(domain, "p0");
        Mimir.Core.Schemas.Predicate<Static> p1 = GetStatic(domain, "p1");
        Mimir.Core.Schemas.Predicate<Static> p2 = GetStatic(domain, "p2");
        Mimir.Core.Schemas.Predicate<Static> p3 = GetStatic(domain, "p3");
        Mimir.Core.Schemas.Predicate<Static> p4 = GetStatic(domain, "p4");

        Fact<Static>[] first =
        [
            context.RegisterFact(p0, Array.Empty<Constant>()),
            context.RegisterFact(p1, a),
            context.RegisterFact(p2, a, b),
            context.RegisterFact(p3, a, b, c),
            context.RegisterFact(p4, [a, b, c, d])
        ];
        Fact<Static>[] second =
        [
            context.RegisterFact(p0, Array.Empty<Constant>()),
            context.RegisterFact(p1, a),
            context.RegisterFact(p2, a, b),
            context.RegisterFact(p3, a, b, c),
            context.RegisterFact(p4, [a, b, c, d])
        ];

        for (int i = 0; i < first.Length; i++)
            Assert.Same(first[i], second[i]);
        Assert.Equal(5, context.AllFacts.Count);

        Assert.Throws<ArgumentException>(() => context.RegisterFact(p2, a));
        Assert.Throws<ArgumentException>(() => context.RegisterFact(p1, wrong));
        Assert.Throws<ArgumentException>(() => context.RegisterFact(p1, (Constant)null!));
        Assert.Equal(5, context.AllFacts.Count);
    }

    private static Mimir.Core.Schemas.Predicate<Static> GetStatic(Domain domain, string name)
        => Assert.Single(domain.Statics, predicate => predicate.Name == name);
}
