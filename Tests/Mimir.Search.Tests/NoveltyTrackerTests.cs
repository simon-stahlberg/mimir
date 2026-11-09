using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class NoveltyTrackerTests
{
    /// <summary>
    /// The incremental overload inspects only the atoms a successor adds, on the argument that every other atom
    /// was registered with the parent. Two trackers fed the same states, one incrementally and one in full, must
    /// therefore agree on every verdict; if they ever diverge the argument is wrong somewhere.
    /// </summary>
    [Theory]
    [InlineData("gripper", 1)]
    [InlineData("gripper", 2)]
    [InlineData("delivery", 1)]
    [InlineData("delivery", 2)]
    [InlineData("blocks_3", 2)]
    [InlineData("blocks_3", 3)]
    [InlineData("ferry", 2)]
    [InlineData("childsnack", 2)]
    [InlineData("miconic", 2)]
    public void IncrementalVerdictsMatchFullVerdicts(string domainDirectory, int width)
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            domainDirectory,
            generatorType: ApplicableActionGeneratorType.Lifted);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);

        var incremental = new NoveltyTracker(width, problem.InitialState.Context);
        var full = new NoveltyTracker(width, problem.InitialState.Context);
        incremental.IsNovel(problem.InitialState);
        full.IsNovel(problem.InitialState);

        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        int compared = 0;
        while (open.Count > 0 && compared < 400)
        {
            State parent = open.Dequeue();
            ExtendedState extended = parent.Expand();
            foreach (GroundAction action in generator.GetApplicableActions(extended))
            {
                State successor = extended.Apply(action);
                if (!closed.Add(successor)) continue;

                // Order matters: both trackers must see the same state before either registers it.
                bool incrementalVerdict = incremental.IsNovel(successor, parent);
                bool fullVerdict = full.IsNovel(successor);
                Assert.Equal(fullVerdict, incrementalVerdict);
                compared++;

                if (fullVerdict) open.Enqueue(successor);
            }
        }

        Assert.True(compared > 0, $"No successors were compared for '{domainDirectory}'.");
    }

    [Fact]
    public void ASuccessorThatAddsNothingIsNeverNovel()
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            "gripper",
            generatorType: ApplicableActionGeneratorType.Lifted);
        var tracker = new NoveltyTracker(1, problem.InitialState.Context);
        tracker.IsNovel(problem.InitialState);

        Assert.False(tracker.IsNovel(problem.InitialState, problem.InitialState));
    }

    /// <summary>
    /// The cover records which atom sets a given atom has already been triple-registered against, so that only
    /// triples reaching outside it are enumerated again. It is a shortcut, never a different verdict.
    /// </summary>
    [Theory]
    [InlineData("gripper")]
    [InlineData("childsnack")]
    [InlineData("logistics")]
    public void TheTripleCoverShortcutAgreesWithPlainEnumeration(string domainDirectory)
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            domainDirectory,
            generatorType: ApplicableActionGeneratorType.Factored);
        var generator = problem.GetApplicableActionGenerator(problem.InitialState);

        var stream = new List<(State Parent, State Child)>();
        var queue = new Queue<State>();
        var seen = new HashSet<State> { problem.InitialState };
        queue.Enqueue(problem.InitialState);
        while (queue.Count > 0 && stream.Count < 400)
        {
            State parent = queue.Dequeue();
            ExtendedState extended = parent.Expand();
            foreach (GroundAction action in generator.GetApplicableActions(extended))
            {
                State child = extended.Apply(action);
                stream.Add((parent, child));
                if (seen.Add(child)) queue.Enqueue(child);
                if (stream.Count >= 400) break;
            }
        }

        var withCover = new NoveltyTracker(3, problem.InitialState.Context) { UseCoverFastPath = true };
        var without = new NoveltyTracker(3, problem.InitialState.Context) { UseCoverFastPath = false };
        withCover.IsNovel(problem.InitialState);
        without.IsNovel(problem.InitialState);

        Assert.NotEmpty(stream);
        foreach ((State parent, State child) in stream)
        {
            Assert.Equal(
                without.IsNovel(child, parent),
                withCover.IsNovel(child, parent));
        }
    }
}
