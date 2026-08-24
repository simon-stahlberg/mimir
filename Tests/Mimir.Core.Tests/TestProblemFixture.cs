using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

internal sealed class TestProblemFixture
{
    private readonly Dictionary<string, Mimir.Core.Schemas.Predicate<Fluent>> _fluents;
    private readonly Dictionary<string, Mimir.Core.Schemas.Predicate<Static>> _statics;

    private TestProblemFixture(Problem problem)
    {
        Problem = problem;
        _fluents = problem.Domain.Fluents.ToDictionary(predicate => predicate.Name);
        _statics = problem.Domain.Statics.ToDictionary(predicate => predicate.Name);
    }

    public Problem Problem { get; }

    public InstanceContext Context => Problem.Context;

    public ActionSchema ActionSchema => Assert.Single(Problem.Domain.Actions);

    public Fact<Fluent> RegisterFluent(string name) =>
        Context.RegisterFact(_fluents[name], Array.Empty<Constant>());

    public Fact<Static> RegisterStatic(string name) =>
        Context.RegisterFact(_statics[name], Array.Empty<Constant>());

    public static TestProblemFixture Create(
        IEnumerable<string>? fluentNames = null,
        IEnumerable<string>? staticNames = null)
    {
        string[] fluents = fluentNames?.ToArray() ?? Array.Empty<string>();
        string[] statics = staticNames?.ToArray() ?? Array.Empty<string>();
        var domainBuilder = new DomainBuilder("fixture");
        domainBuilder.Requirements().Add(":strips").Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        foreach (string name in fluents.Concat(statics))
            predicates.Add(name);
        predicates.Close();

        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("fixture-action");
        foreach (string name in fluents)
            action.AddEffect(name);
        action.Close();
        actions.Close();

        Domain domain = domainBuilder.Build();
        Problem problem = new ProblemBuilder(domain, "fixture-problem").Build();

        return new TestProblemFixture(problem);
    }
}
