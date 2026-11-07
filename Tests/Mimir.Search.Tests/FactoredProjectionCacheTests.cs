using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class FactoredProjectionCacheTests
{
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");
    private static string BenchmarkPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Benchmark");

    /// <summary>
    /// A carried cache is patched from one state to the next rather than rebuilt, so a mistake in the patching
    /// would quietly change which actions are reported. Walking a search with and without one has to agree on
    /// every state, including after the delta grows large enough to force a rebuild.
    /// </summary>
    [Theory]
    [InlineData("examples", "childsnack")]
    [InlineData("examples", "gripper")]
    [InlineData("examples", "blocks_4")]
    [InlineData("examples", "ferry")]
    [InlineData("examples", "miconic")]
    [InlineData("benchmark", "strips/rovers")]
    [InlineData("benchmark", "strips/tpp")]
    [InlineData("benchmark", "strips/transport-opt14-strips")]
    [InlineData("benchmark", "strips/tidybot-opt11-strips")]
    [InlineData("benchmark", "adl/schedule")]
    public void CachedProjectionsAgreeWithFreshOnes(string root, string domainDirectory)
    {
        Problem problem = Load(root, domainDirectory);
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);
        FactoredProjectionCache cache = generator.CreateProjectionCache();

        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        int visited = 0;
        while (open.Count > 0 && visited < 80)
        {
            ExtendedState state = open.Dequeue().Expand();
            visited++;

            List<string> cached = Describe(generator.GetFactoredApplicableActions(state, cache));
            List<string> fresh = Describe(generator.GetFactoredApplicableActions(state));
            Assert.Equal(fresh, cached);

            foreach (GroundAction action in generator.GetApplicableActions(state))
            {
                State successor = state.Apply(action);
                if (closed.Add(successor)) open.Enqueue(successor);
            }
        }

        Assert.True(visited > 1, $"Only {visited} state(s) were compared for '{domainDirectory}'.");
    }

    [Fact]
    public void ClearingACacheRestoresFreshProjection()
    {
        Problem problem = Load("examples", "childsnack");
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);
        FactoredProjectionCache cache = generator.CreateProjectionCache();
        ExtendedState state = problem.InitialState.Expand();

        List<string> first = Describe(generator.GetFactoredApplicableActions(state, cache));
        cache.Clear();
        List<string> afterClear = Describe(generator.GetFactoredApplicableActions(state, cache));

        Assert.Equal(first, afterClear);
    }

    [Fact]
    public void RejectsACacheFromAnotherGenerator()
    {
        Problem problem = Load("examples", "childsnack");
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);
        FactoredProjectionCache foreign = new FactoredApplicableActionGenerator(problem).CreateProjectionCache();

        Assert.Throws<InvalidOperationException>(
            () => generator.GetFactoredApplicableActions(problem.InitialState.Expand(), foreign).ToList());
    }

    private static List<string> Describe(IEnumerable<FactoredAction> groups)
        => groups.SelectMany(group => group.Expand()).Select(action => action.ToString()!).ToList();

    private static Problem Load(string root, string domainDirectory)
    {
        string directory = root == "examples"
            ? Path.Combine(ExamplesPath, domainDirectory)
            : Path.Combine(BenchmarkPath, domainDirectory);

        string problemFile = root == "examples"
            ? "p01.pddl"
            : Directory
                .EnumerateFiles(directory, "*.pddl")
                .Where(path => !path.EndsWith("domain.pddl", StringComparison.OrdinalIgnoreCase))
                .Order(StringComparer.Ordinal)
                .First();

        var domain = Domain.FromFile(Path.Combine(directory, "domain.pddl"));
        return Problem.FromFile(
            domain,
            root == "examples" ? Path.Combine(directory, problemFile) : problemFile,
            ApplicableActionGeneratorType.Factored);
    }
}
