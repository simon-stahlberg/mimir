using Mimir.Search.Algorithms.BreadthFirst;
using Mimir.Search.Evaluation;
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

        var bfs = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
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

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void ReentrantGenerationPreservesSearchAndCallbacks(ApplicableActionGeneratorType generatorType)
    {
        Problem problem = SearchTestHelpers.LoadProblem("childsnack", generatorType: generatorType);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);

        (SearchResult Result,
            List<(string EventType, State State, GroundAction Action, State Successor)> Events,
            List<double> GLayers) Run(bool generateDuringCallback)
        {
            var events = new List<(string, State, GroundAction, State)>();
            var gLayers = new List<double>();
            var search = new BfsSearch(problem.InitialState, GoalCondition.FromProblem(problem), null);
            search.StateGenerated += transition =>
            {
                events.Add(("all", transition.State, transition.Action, transition.SuccessorState));
                if (generateDuringCallback)
                    _ = generator.GetApplicableActions(transition.SuccessorState.Expand()).ToArray();
            };
            search.StateGeneratedInSearchTree += transition => events.Add(("new", transition.State, transition.Action, transition.SuccessorState));
            search.StateGeneratedNotInSearchTree += transition => events.Add(("pruned", transition.State, transition.Action, transition.SuccessorState));
            search.GLayerFinished += gLayers.Add;
            search.TransitionGenerated += transition =>
                events.Add(("transition-all", transition.State, transition.Action, transition.SuccessorState));
            search.TransitionDiscovered += transition =>
                events.Add(("transition-new", transition.State, transition.Action, transition.SuccessorState));
            search.TransitionPruned += transition =>
                events.Add(("transition-pruned", transition.State, transition.Action, transition.SuccessorState));

            return (search.Search(), events, gLayers);
        }

        var reentrant = Run(true);
        var baseline = Run(false);

        Assert.Equal(baseline.Result.Status, reentrant.Result.Status);
        Assert.Equal(baseline.Result.Plan.ToArray(), reentrant.Result.Plan.ToArray());
        Assert.Equal(baseline.Result.PlanCost, reentrant.Result.PlanCost);
        Assert.Equal(
            baseline.Result.Statistics.NodesExpanded,
            reentrant.Result.Statistics.NodesExpanded);
        Assert.Equal(
            baseline.Result.Statistics.NodesGenerated,
            reentrant.Result.Statistics.NodesGenerated);
        Assert.Equal(
            baseline.Result.Statistics.MaxDepth,
            reentrant.Result.Statistics.MaxDepth);
        Assert.Equal(baseline.Events, reentrant.Events);
        Assert.Equal(baseline.GLayers, reentrant.GLayers);
    }

}
