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
}
