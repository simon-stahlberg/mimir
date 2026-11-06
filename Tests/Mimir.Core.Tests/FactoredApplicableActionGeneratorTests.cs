using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class FactoredApplicableActionGeneratorTests
{
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");
    private static string BenchmarkPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Benchmark");

    [Theory]
    [InlineData("childsnack", "p01.pddl")]
    [InlineData("blocks_4", "p01.pddl")]
    [InlineData("gripper", "p01.pddl")]
    [InlineData("delivery", "p01.pddl")]
    [InlineData("hiking", "p01.pddl")]
    [InlineData("miconic-simpleadl", "p01.pddl")]
    [InlineData("philosophers", "p01.pddl")]
    [InlineData("schedule", "p01.pddl")]
    public void MatchesTheCliqueGeneratorOnExamples(string domainDirectory, string problemFile)
        => AssertParity(Path.Combine(ExamplesPath, domainDirectory), problemFile, stateLimit: 60);

    [Theory]
    [InlineData("strips/transport-opt14-strips")]
    [InlineData("strips/tetris-opt14-strips")]
    [InlineData("strips/organic-synthesis-sat18-strips")]
    [InlineData("strips/tidybot-opt11-strips")]
    [InlineData("strips/rovers")]
    [InlineData("strips/woodworking-opt08-strips")]
    [InlineData("adl/caldera-opt18-adl")]
    [InlineData("adl/optical-telegraphs")]
    public void MatchesTheCliqueGeneratorOnBenchmarkDomains(string domainDirectory)
    {
        string directory = Path.Combine(BenchmarkPath, domainDirectory);
        string problemFile = Directory
            .EnumerateFiles(directory, "*.pddl")
            .Select(Path.GetFileName)
            .Where(name => !string.Equals(name, "domain.pddl", StringComparison.OrdinalIgnoreCase))
            .Order(StringComparer.Ordinal)
            .First()!;

        AssertParity(directory, problemFile, stateLimit: 25);
    }

    [Fact]
    public void RejectsNumericDomains()
    {
        var domain = Domain.FromFile(Path.Combine(BenchmarkPath, "numeric/counters", "domain.pddl"));
        string problemFile = Directory
            .EnumerateFiles(Path.Combine(BenchmarkPath, "numeric/counters"), "*.pddl")
            .First(path => !path.EndsWith("domain.pddl", StringComparison.OrdinalIgnoreCase));

        Assert.Throws<NotSupportedException>(
            () => Problem.FromFile(domain, problemFile, ApplicableActionGeneratorType.Factored));
    }

    [Fact]
    public void KeepsIndependentParametersSetValued()
    {
        var domain = Domain.FromFile(Path.Combine(ExamplesPath, "gripper", "domain.pddl"));
        var problem = Problem.FromFile(
            domain,
            Path.Combine(ExamplesPath, "gripper", "p01.pddl"),
            ApplicableActionGeneratorType.Factored);
        var generator = new FactoredApplicableActionGenerator(problem);

        FactoredAction[] groups = generator
            .GetFactoredApplicableActions(problem.InitialState.Expand())
            .Where(group => string.Equals(group.Schema.Name, "pick", StringComparison.OrdinalIgnoreCase))
            .ToArray();

        // `at` ties the room to the ball, but the gripper is independent, so every pick is one product.
        FactoredAction pick = Assert.Single(groups);
        Assert.Empty(pick.Exclusions);
        Assert.Equal(4, pick.MemberCountUpperBound);
        Assert.Equal(4, pick.Expand().Count());
    }

    [Theory]
    // hiking states inequalities between parameters the precondition otherwise leaves independent; schedule is
    // the only benchmark domain with negative preconditions over three or more parameters.
    [InlineData("strips/hiking-opt14-strips", true)]
    [InlineData("adl/schedule", false)]
    public void CarriesNegativePreconditionsAsExclusions(string domainDirectory, bool expectDistinctness)
    {
        string directory = Path.Combine(BenchmarkPath, domainDirectory);
        string problemFile = Directory
            .EnumerateFiles(directory, "*.pddl")
            .Order(StringComparer.Ordinal)
            .First(path => !path.EndsWith("domain.pddl", StringComparison.OrdinalIgnoreCase));

        var domain = Domain.FromFile(Path.Combine(directory, "domain.pddl"));
        var problem = Problem.FromFile(domain, problemFile, ApplicableActionGeneratorType.Factored);
        var clique = new CliqueApplicableActionGenerator(problem);
        var factored = new FactoredApplicableActionGenerator(problem);

        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        bool sawExclusion = false;
        var expected = new List<GroundAction>();
        for (int visited = 0; open.Count > 0 && visited < 40; visited++)
        {
            ExtendedState state = open.Dequeue().Expand();
            FactoredAction[] groups = factored.GetFactoredApplicableActions(state).ToArray();
            clique.CollectApplicableActions(state, expected);

            sawExclusion |= groups.Any(group =>
                group.Exclusions.Any(exclusion => exclusion.IsDistinctness == expectDistinctness));

            // Expansion must drop exactly the members the exclusions rule out.
            Assert.Equal(
                TransitionCounts(expected, problem),
                TransitionCounts(groups.SelectMany(group => group.Expand()).ToList(), problem));

            foreach (GroundAction action in expected)
            {
                State successor = state.Apply(action);
                if (closed.Add(successor)) open.Enqueue(successor);
            }
        }

        Assert.True(sawExclusion, $"No {(expectDistinctness ? "distinctness" : "forbidden-tuple")} exclusion arose.");
    }

    private static void AssertParity(string directory, string problemFile, int stateLimit)
    {
        var domain = Domain.FromFile(Path.Combine(directory, "domain.pddl"));
        var problem = Problem.FromFile(
            domain,
            Path.Combine(directory, problemFile),
            ApplicableActionGeneratorType.Factored);

        var clique = new CliqueApplicableActionGenerator(problem);
        var factored = new FactoredApplicableActionGenerator(problem);

        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        var expected = new List<GroundAction>();
        int visited = 0;
        while (open.Count > 0 && visited < stateLimit)
        {
            State state = open.Dequeue();
            ExtendedState extended = state.Expand();
            visited++;

            clique.CollectApplicableActions(extended, expected);
            List<GroundAction> actual = factored
                .GetFactoredApplicableActions(extended)
                .SelectMany(group => group.Expand())
                .ToList();

            Assert.Equal(TransitionCounts(expected, problem), TransitionCounts(actual, problem));

            foreach (GroundAction action in expected)
            {
                State successor = extended.Apply(action);
                if (closed.Add(successor)) open.Enqueue(successor);
            }
        }

        Assert.True(visited > 0, $"No states were explored for {directory}.");
    }

    // Both generators pick an arbitrary representative for parameters that do not affect the transition, so
    // parity is asserted on the projection onto the parameters that do.
    private static SortedDictionary<string, int> TransitionCounts(List<GroundAction> actions, Problem problem)
    {
        var counts = new SortedDictionary<string, int>(StringComparer.Ordinal);
        foreach (GroundAction action in actions)
        {
            bool[] relevant = problem.Context
                .GetActionGroundingPlan(action.Schema)
                .TransitionRelevantParameters;
            string key = $"{action.Schema.Name}("
                + string.Join(",", action.Arguments.Select((argument, index) => relevant[index] ? argument.Name : "_"))
                + ")";
            counts[key] = counts.GetValueOrDefault(key) + 1;
        }

        return counts;
    }
}
