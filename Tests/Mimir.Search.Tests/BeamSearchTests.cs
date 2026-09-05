using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;
using Expansions = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.ExtendedState State, System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)> Successors)>;

namespace Mimir.Search.Tests;

public class BeamSearchTests
{
    private static Problem Problem() => SearchTestHelpers.CreateProblemFromText("""
        (define (domain branching) (:requirements :strips)
          (:predicates (root) (left) (right) (shared) (goal))
          (:action first :parameters () :precondition (root) :effect (and (not (root)) (left)))
          (:action second :parameters () :precondition (root) :effect (and (not (root)) (right)))
          (:action merge-left :parameters () :precondition (left) :effect (and (not (left)) (shared)))
          (:action merge-right :parameters () :precondition (right) :effect (and (not (right)) (shared)))
          (:action finish :parameters () :precondition (shared) :effect (and (not (shared)) (goal))))
        """, """
        (define (problem branching) (:domain branching) (:init (root)) (:goal (goal)))
        """);

    private static SearchBuilder Builder(Problem problem) => new SearchBuilder()
        .WithInitialState(problem.InitialState).WithGoal(problem)
        .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem));

    private sealed class BatchHeuristic : IHeuristic
    {
        public List<int> BatchSizes = [];
        public double Value = 1;
        public bool WrongCount;
        public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
            => throw new InvalidOperationException("Scalar evaluation should not be used.");
        public IReadOnlyList<HeuristicEvaluation> Evaluate(IReadOnlyList<ExtendedState> states, GoalCondition? goal = null)
        {
            Assert.NotNull(goal);
            BatchSizes.Add(states.Count);
            return WrongCount ? [] : states.Select(_ => new HeuristicEvaluation(Value)).ToArray();
        }
    }

    private sealed class BatchQ : IQHeuristic
    {
        public List<int> BatchSizes = [];
        public double Value;
        public bool WrongRows;
        public bool WrongScores;
        public QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null)
            => throw new InvalidOperationException("Scalar evaluation should not be used.");
        public IReadOnlyList<QHeuristicEvaluation> Evaluate(Expansions expansions, GoalCondition? goal = null)
        {
            Assert.NotNull(goal);
            BatchSizes.Add(expansions.Count);
            return WrongRows ? [] : expansions.Select(row => new QHeuristicEvaluation(
                WrongScores ? [] : row.Successors.Select(pair => pair.Action.Schema.Name == "merge-right" ? 10 : Value).ToArray())).ToArray();
        }
    }

    [Fact]
    public void StateHeuristicUsesBatchOverrideAndDeduplicatesSuccessors()
    {
        Problem problem = Problem();
        var heuristic = new BatchHeuristic();
        SearchResult result = Builder(problem).WithHeuristic(heuristic).BuildBeam(beamSize: 2).Search();
        Assert.True(result.IsSuccess);
        Assert.Equal(new[] { 2, 1 }, heuristic.BatchSizes);
        Assert.Equal(new double?[] { 1, 1, null }, result.ActionValues);
        Assert.Equal(3, result.Statistics.EvaluatedCandidates);
        Assert.Equal(result.EndState, SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan));
    }

    [Fact]
    public void QHeuristicBatchesParentsAndKeepsBetterDuplicatePath()
    {
        Problem problem = Problem();
        var heuristic = new BatchQ();
        SearchResult result = Builder(problem).WithQHeuristic(heuristic).BuildQBeam(beamSize: 2).Search();
        Assert.True(result.IsSuccess);
        Assert.Equal(new[] { 1, 2 }, heuristic.BatchSizes);
        Assert.Equal(new[] { "second", "merge-right", "finish" }, result.Plan.Select(action => action.Schema.Name));
        Assert.Equal(new double?[] { 0, 10, null }, result.ActionValues);
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void CancellationAfterGenerationDoesNotStartEvaluation(bool q)
    {
        Problem problem = Problem();
        var heuristic = new BatchHeuristic();
        var qHeuristic = new BatchQ();
        using var cancellation = new CancellationTokenSource();
        SearchBuilder builder = Builder(problem).WithHeuristic(heuristic).WithQHeuristic(qHeuristic);
        ISearchAlgorithm search = q ? builder.BuildQBeam() : builder.BuildBeam();
        search.TransitionGenerated += _ => cancellation.Cancel();
        SearchResult result = search.Search(cancellation.Token);
        Assert.True(result.IsCanceled);
        Assert.Empty(heuristic.BatchSizes);
        Assert.Empty(qHeuristic.BatchSizes);
        Assert.Empty(result.PartialPlan);
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void BuilderAttachesEventsAndOnlyAdmittedNodesAreGenerated(bool q)
    {
        Problem problem = Problem();
        var expanded = new List<SearchNode>();
        var nodes = new List<SearchNode>();
        var generated = new List<SearchTransition>();
        var discovered = new List<SearchTransition>();
        var pruned = new List<SearchTransition>();
        SearchBuilder builder = Builder(problem).WithQHeuristic(new BatchQ())
            .OnNodeExpanded(expanded.Add).OnNodeGenerated(nodes.Add)
            .OnStateGenerated(generated.Add).OnStateGeneratedInSearchTree(discovered.Add)
            .OnStateGeneratedNotInSearchTree(pruned.Add);
        SearchResult result = (q ? builder.BuildQBeam(maxDepth: 1) : builder.BuildBeam(maxDepth: 1)).Search();
        Assert.Equal(SearchStatus.DepthLimitReached, result.Status);
        Assert.Single(expanded);
        Assert.Equal(2, nodes.Count);
        Assert.Equal(2, generated.Count);
        Assert.Single(discovered);
        Assert.Single(pruned);
        Assert.Equal(2, result.Statistics.VisitedStates);
    }

    [Theory]
    [InlineData(double.NaN)]
    [InlineData(double.NegativeInfinity)]
    [InlineData(double.PositiveInfinity)]
    public void RejectsNonfiniteQValues(double value)
    {
        Problem problem = Problem();
        Assert.Throws<InvalidOperationException>(() => Builder(problem)
            .WithQHeuristic(new BatchQ { Value = value }).BuildQBeam().Search());
    }

    [Theory]
    [InlineData(double.NaN)]
    [InlineData(double.NegativeInfinity)]
    [InlineData(-1)]
    public void RejectsInvalidStateValues(double value)
    {
        Problem problem = Problem();
        Assert.Throws<InvalidOperationException>(() => Builder(problem)
            .WithHeuristic(new BatchHeuristic { Value = value }).BuildBeam().Search());
    }

    [Fact]
    public void RejectsIncorrectBatchShapes()
    {
        Problem problem = Problem();
        Assert.Throws<InvalidOperationException>(() => Builder(problem)
            .WithQHeuristic(new BatchQ { WrongRows = true }).BuildQBeam().Search());
        Assert.Throws<InvalidOperationException>(() => Builder(problem)
            .WithQHeuristic(new BatchQ { WrongScores = true }).BuildQBeam().Search());
        Assert.Throws<InvalidOperationException>(() => Builder(problem)
            .WithHeuristic(new BatchHeuristic { WrongCount = true }).BuildBeam().Search());
    }

    [Fact]
    public void ValidatesLimitsAndRequiredQHeuristic()
    {
        SearchBuilder builder = Builder(Problem()).WithQHeuristic(new BatchQ());
        Assert.Throws<ArgumentOutOfRangeException>(() => builder.BuildBeam(beamSize: 0));
        Assert.Throws<ArgumentOutOfRangeException>(() => builder.BuildBeam(maxDepth: -1));
        Assert.Throws<ArgumentOutOfRangeException>(() => builder.BuildQBeam(beamSize: 0));
        Assert.Throws<ArgumentOutOfRangeException>(() => builder.BuildQBeam(maxDepth: -1));
        Assert.Throws<ArgumentOutOfRangeException>(() => builder.BuildBeam().Search(maxExpandedStates: -1));
        Assert.Throws<InvalidOperationException>(() => Builder(Problem()).BuildQBeam());
    }
}
