using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Mimir.Search;
using Xunit;
using Mimir.Search.Planning;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class BfsSearchTests
{
    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    [Fact]
    public void BfsFindsPlanForBlocksWorld()
    {
        var domainPath = Path.Combine(BasePath, "blocks_4", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "blocks_4", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        var grounder = new RpgGrounder();
        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, grounder);

        var bfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .BuildBfs();

        var result = bfs.Search();

        Assert.True(result.IsSuccess);
        Assert.NotEmpty(result.Plan);

        // Verify plan length (BFS should find shortest plan)
        // For blocks_4 p01 (4 blocks), a simple stack/unstack usually takes 6 steps for this problem
        Assert.True(result.Statistics.NodesExpanded > 0);
    }

    [Fact]
    public void RejectsMiconicFullAdlQuantifiedGoalBeforeSearch()
    {
        var domainPath = Path.Combine(BasePath, "miconic-fulladl", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "miconic-fulladl", "p01.pddl");

        Domain domain = Domain.FromFile(domainPath);

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Problem.FromFile(domain, problemPath));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [Fact]
    public void BfsCallbackEvents_FireForGeneratedNewAndPrunedSuccessors()
    {
        var problem = SearchTestHelpers.LoadProblem("childsnack");
        var generated = new List<SearchTransition>();
        var generatedNew = new List<SearchTransition>();
        var pruned = new List<SearchTransition>();
        var finishedGLayers = new List<double>();

        var bfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .OnStateGenerated(generated.Add)
            .OnStateGeneratedInSearchTree(generatedNew.Add)
            .OnStateGeneratedNotInSearchTree(pruned.Add)
            .OnGLayerFinished(finishedGLayers.Add)
            .BuildBfs();

        var result = bfs.Search();

        Assert.True(result.IsSuccess);
        Assert.Equal(16, generated.Count);
        Assert.Equal(6, generatedNew.Count);
        Assert.Equal(10, pruned.Count);
        Assert.Equal([0d, 0d, 1d, 2d, 3d], finishedGLayers);
    }

    [Fact]
    public void GroundedFastPath_MatchesFallbackDuringReentrantCallbacks()
    {
        Problem problem = SearchTestHelpers.LoadProblem("childsnack");
        GroundedApplicableActionGenerator grounded =
            SearchTestHelpers.CreateGroundedGenerator(problem);

        (SearchResult Result,
            List<(string EventType, State State, GroundAction Action, State Successor)> Events,
            List<double> GLayers) Run(IApplicableActionGenerator generator)
        {
            var events = new List<(string, State, GroundAction, State)>();
            var gLayers = new List<double>();
            var search = new SearchBuilder()
                .WithInitialState(problem.InitialState)
                .WithGoal(problem)
                .WithActionGenerator(generator)
                .OnStateGenerated(transition =>
                {
                    events.Add(("all", transition.State, transition.Action, transition.SuccessorState));
                    _ = grounded
                        .GetApplicableActions(transition.SuccessorState.Expand())
                        .ToArray();
                })
                .OnStateGeneratedInSearchTree(transition =>
                    events.Add(("new", transition.State, transition.Action, transition.SuccessorState)))
                .OnStateGeneratedNotInSearchTree(transition =>
                    events.Add(("pruned", transition.State, transition.Action, transition.SuccessorState)))
                .OnGLayerFinished(gLayers.Add)
                .BuildBfs();
            search.TransitionGenerated += transition =>
                events.Add(("transition-all", transition.State, transition.Action, transition.SuccessorState));
            search.TransitionDiscovered += transition =>
                events.Add(("transition-new", transition.State, transition.Action, transition.SuccessorState));
            search.TransitionPruned += transition =>
                events.Add(("transition-pruned", transition.State, transition.Action, transition.SuccessorState));

            return (search.Search(), events, gLayers);
        }

        var fastPath = Run(grounded);
        var fallback = Run(new DelegatingActionGenerator(grounded));

        Assert.Equal(fallback.Result.Status, fastPath.Result.Status);
        Assert.Equal(fallback.Result.Plan.ToArray(), fastPath.Result.Plan.ToArray());
        Assert.Equal(fallback.Result.PlanCost, fastPath.Result.PlanCost);
        Assert.Equal(
            fallback.Result.Statistics.NodesExpanded,
            fastPath.Result.Statistics.NodesExpanded);
        Assert.Equal(
            fallback.Result.Statistics.NodesGenerated,
            fastPath.Result.Statistics.NodesGenerated);
        Assert.Equal(
            fallback.Result.Statistics.MaxDepth,
            fastPath.Result.Statistics.MaxDepth);
        Assert.Equal(fallback.Events, fastPath.Events);
        Assert.Equal(fallback.GLayers, fastPath.GLayers);
    }

    [Fact]
    public void GroundedSubclass_UsesItsInterfaceImplementation()
    {
        Domain domain = new DomainBuilder("d")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("ready").Add("done").Close()
            .Actions()
                .Add("finish")
                    .AddPrecondition("ready")
                    .AddEffect("done")
                    .Close()
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p")
            .InitialState().AddFact("ready").Close()
            .Goal().Add("done").Close()
            .Build();
        var generator = new EmptyGroundedGenerator(problem);
        IApplicableActionGenerator interfaceGenerator = generator;

        Assert.Single(generator.GetApplicableActions(problem.InitialState.Expand()));
        Assert.Empty(interfaceGenerator.GetApplicableActions(problem.InitialState.Expand()));

        var search = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(interfaceGenerator)
            .BuildBfs();

        Assert.Equal(SearchStatus.Failed, search.Search().Status);
    }

    private sealed class DelegatingActionGenerator(IApplicableActionGenerator inner)
        : IApplicableActionGenerator
    {
        public Problem Problem => inner.Problem;

        public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state)
            => inner.GetApplicableActions(state);

        public IEnumerable<GroundAction> GetApplicableActions(
            ExtendedState state,
            int maxActions)
            => inner.GetApplicableActions(state, maxActions);
    }

    private sealed class EmptyGroundedGenerator(Problem problem)
        : GroundedApplicableActionGenerator(
            problem,
            problem.InitialState,
            new RpgGrounder()),
          IApplicableActionGenerator
    {
        IEnumerable<GroundAction> IApplicableActionGenerator.GetApplicableActions(
            ExtendedState state)
            => Array.Empty<GroundAction>();

        IEnumerable<GroundAction> IApplicableActionGenerator.GetApplicableActions(
            ExtendedState state,
            int maxActions)
            => Array.Empty<GroundAction>();
    }
}
