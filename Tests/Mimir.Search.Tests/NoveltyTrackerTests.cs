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
                bool incrementalVerdict = incremental.IsNovel(successor, parent, action);
                bool fullVerdict = full.IsNovel(successor);
                Assert.Equal(fullVerdict, incrementalVerdict);
                compared++;

                if (fullVerdict) open.Enqueue(successor);
            }
        }

        Assert.True(compared > 0, $"No successors were compared for '{domainDirectory}'.");
    }

    [Fact]
    public void ASuccessorThatAddsNothingIsNeverNovelUnderAtomNovelty()
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            "gripper",
            generatorType: ApplicableActionGeneratorType.Lifted);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        GroundAction action = generator.GetApplicableActions(problem.InitialState.Expand()).First();

        var tracker = new NoveltyTracker(1, NoveltyKind.Atoms, problem.InitialState.Context);
        tracker.IsNovel(problem.InitialState);

        Assert.False(tracker.IsNovel(problem.InitialState, problem.InitialState, action));
    }

    /// <summary>
    /// Argument novelty sees a parameter that atom novelty cannot: childsnack's make_sandwich adds only
    /// at_kitchen_sandwich(?s), so which bread it consumes changes no atom that becomes true.
    /// </summary>
    [Fact]
    public void ArgumentNoveltySeparatesBindingsAtomNoveltyCannot()
    {
        Problem problem = SearchTestHelpers.LoadProblem(
            "childsnack",
            generatorType: ApplicableActionGeneratorType.Lifted);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        ExtendedState state = problem.InitialState.Expand();

        GroundAction[] sandwiches = generator
            .GetApplicableActions(state)
            .Where(action => action.Schema.Name.StartsWith("make_sandwich", StringComparison.OrdinalIgnoreCase))
            .ToArray();
        Assert.NotEmpty(sandwiches);

        var atoms = new NoveltyTracker(1, NoveltyKind.Atoms, state.State.Context);
        var arguments = new NoveltyTracker(1, NoveltyKind.Arguments, state.State.Context);
        atoms.IsNovel(problem.InitialState);
        arguments.IsNovel(problem.InitialState);

        int atomNovel = 0;
        int argumentNovel = 0;
        foreach (GroundAction action in sandwiches)
        {
            State successor = state.Apply(action);
            if (atoms.IsNovel(successor, problem.InitialState, action)) atomNovel++;
            if (arguments.IsNovel(successor, problem.InitialState, action)) argumentNovel++;
        }

        Assert.True(
            argumentNovel >= atomNovel,
            $"Argument novelty kept {argumentNovel} of {sandwiches.Length}, atom novelty {atomNovel}.");
    }
}
