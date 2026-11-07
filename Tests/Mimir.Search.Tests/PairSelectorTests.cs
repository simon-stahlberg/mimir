using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class PairSelectorTests
{
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");
    private static string BenchmarkPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Benchmark");

    /// <summary>
    /// The claim the width-2 selector rests on: the members it skips are the ones IW(2) would have discarded
    /// once the members it emitted had registered their tuples. Replaying the emitted members through a fresh
    /// tracker and then asking every skipped member for a verdict checks exactly that.
    /// </summary>
    [Theory]
    [InlineData("examples", "childsnack")]
    [InlineData("examples", "gripper")]
    [InlineData("examples", "blocks_4")]
    [InlineData("examples", "delivery")]
    [InlineData("examples", "ferry")]
    [InlineData("examples", "miconic")]
    [InlineData("benchmark", "strips/rovers")]
    [InlineData("benchmark", "strips/transport-opt14-strips")]
    [InlineData("benchmark", "strips/visitall")]
    [InlineData("benchmark", "strips/tpp")]
    public void SkippedMembersWouldHaveBeenPruned(string root, string domainDirectory)
    {
        Problem problem = Load(root, domainDirectory);
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);
        FactoredExpansion expansion = Assert.IsType<FactoredExpansion>(FactoredExpansion.TryCreate(generator));

        var tracker = new NoveltyTracker(2, problem.InitialState.Context);
        tracker.IsNovel(problem.InitialState);

        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        int checkedStates = 0;
        while (open.Count > 0 && checkedStates < 25)
        {
            State parent = open.Dequeue();
            ExtendedState extended = parent.Expand();
            checkedStates++;

            GroundAction[] selected = expansion.CandidateActions(extended, 2, tracker).ToArray();
            var selectedKeys = new HashSet<string>(selected.Select(action => action.ToString()!), StringComparer.Ordinal);

            // Replay exactly what the search would do with the selection.
            foreach (GroundAction action in selected)
            {
                State successor = extended.Apply(action);
                if (tracker.IsNovel(successor, parent) && closed.Add(successor)) open.Enqueue(successor);
            }

            // Anything left out must now be non-novel; otherwise the selector dropped a state IW(2) would keep.
            foreach (GroundAction action in generator.GetApplicableActions(extended))
            {
                if (selectedKeys.Contains(action.ToString()!)) continue;

                State successor = extended.Apply(action);
                Assert.False(
                    tracker.IsNovel(successor, parent),
                    $"'{action}' was skipped in '{domainDirectory}' but IW(2) would have kept it.");
            }
        }

        Assert.True(checkedStates > 0, $"No states were checked for '{domainDirectory}'.");
    }

    [Fact]
    public void DeclinesRatherThanFailsOnNegatedGoals()
    {
        // snake states its goal with a negated literal, so the shortcut must stand aside and let IW run as usual.
        string directory = Path.Combine(BenchmarkPath, "strips/snake-opt18-strips");
        string problemFile = Directory
            .EnumerateFiles(directory, "*.pddl")
            .Where(path => !path.EndsWith("domain.pddl", StringComparison.OrdinalIgnoreCase))
            .Order(StringComparer.Ordinal)
            .First();

        var domain = Domain.FromFile(Path.Combine(directory, "domain.pddl"));
        var problem = Problem.FromFile(domain, problemFile, ApplicableActionGeneratorType.Factored);
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);

        Assert.Contains(problem.Goal, literal => !literal.IsPositive);
        Assert.Null(FactoredExpansion.TryCreate(generator));
    }

    private static Problem Load(string root, string domainDirectory)
    {
        string directory = root == "examples"
            ? Path.Combine(ExamplesPath, domainDirectory)
            : Path.Combine(BenchmarkPath, domainDirectory);

        string problemFile = root == "examples"
            ? "p01.pddl"
            : Path.GetFileName(Directory
                .EnumerateFiles(directory, "*.pddl")
                .Where(path => !path.EndsWith("domain.pddl", StringComparison.OrdinalIgnoreCase))
                .Order(StringComparer.Ordinal)
                .First())!;

        var domain = Domain.FromFile(Path.Combine(directory, "domain.pddl"));
        return Problem.FromFile(domain, Path.Combine(directory, problemFile), ApplicableActionGeneratorType.Factored);
    }
}
