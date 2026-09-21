using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.AStar;
using Mimir.Search.Algorithms.Beam;
using Mimir.Search.Algorithms.BreadthFirst;
using Mimir.Search.Algorithms.GreedyBestFirst;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Algorithms.UniformCost;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Space;
using Xunit;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;

namespace Mimir.Search.Tests;

public sealed class SearchContextTests
{
    [Theory]
    [InlineData("bfs")]
    [InlineData("ucs")]
    [InlineData("astar")]
    [InlineData("gbfs")]
    [InlineData("iw")]
    [InlineData("qgbfs")]
    [InlineData("beam")]
    [InlineData("qbeam")]
    [InlineData("space")]
    public void ConstructorsResolveGeneratorForTheGivenStartState(string algorithm)
    {
        Problem problem = SearchTestHelpers.CreateProblemFromText("""
            (define (domain context) (:requirements :strips)
              (:predicates (enabled) (safe) (done))
              (:action advance :parameters () :precondition (enabled) :effect (and (enabled) (safe)))
              (:action finish :parameters () :precondition (safe) :effect (done)))
            """, "(define (problem p) (:domain context) (:init) (:goal (and (enabled) (done))))");
        GoalCondition goal = GoalCondition.FromProblem(problem);
        var heuristic = new H2Heuristic(problem);
        Assert.Empty(problem.GetApplicableActionGenerator(problem.InitialState)
            .GetApplicableActions(problem.InitialState.Expand()));
        State start = problem.InitialState.WithAdditionalFluentFacts(
            [SearchTestHelpers.GetFluentFact(problem, "enabled")]);

        if (algorithm == "space")
        {
            var space = new SearchSpace(start, goal, null, CancellationToken.None);
            Assert.NotEmpty(space.GoalStates);
            return;
        }

        ISearchAlgorithm search = algorithm switch
        {
            "bfs" => new BfsSearch(start, goal, null),
            "ucs" => new UcsSearch(start, goal, null),
            "astar" => new AStarSearch(start, goal, heuristic, null),
            "gbfs" => new GbfsSearch(start, goal, heuristic, null),
            "iw" => new IwSearch(1, start, goal, null),
            "qgbfs" => new QGbfsSearch(start, goal, new ConstantQ(), 1, true, null, null),
            "beam" => new BeamSearch(start, goal, heuristic, null, 2, 10, false, null, null),
            "qbeam" => new BeamSearch(start, goal, null, new ConstantQ(), 2, 10, true, null, null),
            _ => throw new ArgumentException(algorithm)
        };

        SearchResult result = search.Search();
        Assert.True(result.IsSuccess);
        Assert.Equal(["advance", "finish"], result.Plan.Select(action => action.Schema.Name));
    }

    private sealed class ConstantQ : IQHeuristic
    {
        public QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null)
            => new(new double[successors.Count]);
    }
}
