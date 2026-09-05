using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;
using Expansions = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.ExtendedState State, System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)> Successors)>;

namespace Mimir.Search.Tests;

public class QGbfsSearchTests
{
    private sealed class ScalarQ : IQHeuristic
    {
        public int Calls;
        public double? Value;
        public QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null)
        {
            Calls++;
            Assert.NotNull(goal);
            return new QHeuristicEvaluation(successors.Select(pair => Value ?? (pair.Action.Schema.Name == "best" ? 10d : -1d)).ToArray());
        }
    }

    private sealed class BatchQ : IQHeuristic
    {
        public int LargestBatch;
        public QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null)
            => throw new InvalidOperationException("Scalar evaluation should not be used for non-goal batches.");

        public IReadOnlyList<QHeuristicEvaluation> Evaluate(Expansions expansions, GoalCondition? goal = null)
        {
            LargestBatch = Math.Max(LargestBatch, expansions.Count);
            return expansions.Select(pair => new QHeuristicEvaluation(new double[pair.Successors.Count])).ToArray();
        }
    }

    private static Problem Problem(string goal = "goal") => SearchTestHelpers.CreateProblemFromText("""
        (define (domain branching) (:requirements :strips)
          (:predicates (root) (left) (right) (shared) (goal))
          (:action first :parameters () :precondition (root) :effect (and (not (root)) (left)))
          (:action best :parameters () :precondition (root) :effect (and (not (root)) (right)))
          (:action merge-left :parameters () :precondition (left) :effect (and (not (left)) (shared)))
          (:action merge-right :parameters () :precondition (right) :effect (and (not (right)) (shared))))
        """, $$"""
        (define (problem branching) (:domain branching) (:init (root)) (:goal ({{goal}})))
        """);

    private static SearchBuilder Builder(Problem problem, IQHeuristic heuristic) => new SearchBuilder()
        .WithInitialState(problem.InitialState).WithGoal(problem)
        .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem)).WithQHeuristic(heuristic);

    [Fact]
    public void DefaultOverloadsPreserveScalarEvaluations()
    {
        Problem problem = Problem();
        var state = problem.InitialState.Expand();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Successors successors = generator.GetApplicableActions(state).Select(action => (action, state.Apply(action).Expand())).ToArray();
        var scalar = new ScalarQ();
        IQHeuristic heuristic = scalar;
        IReadOnlyList<QHeuristicEvaluation> evaluations = heuristic.Evaluate(new[] { (state, successors), (state, successors) }, GoalCondition.FromProblem(problem));
        Assert.Equal(2, scalar.Calls);
        Assert.Equal(evaluations[0].Values, evaluations[1].Values);

        Func<Mimir.Core.Grounding.Action, bool> preferred = _ => true;
        IHeuristic stateHeuristic = new DelegateHeuristic(_ => new HeuristicEvaluation(3, preferred));
        foreach (HeuristicEvaluation evaluation in stateHeuristic.Evaluate(new[] { state, state }))
        {
            Assert.Equal(3, evaluation.Value);
            Assert.Same(preferred, evaluation.IsPreferredAction);
        }
    }

    [Fact]
    public void SearchDispatchesToBatchOverride()
    {
        Problem problem = Problem();
        var heuristic = new BatchQ();
        SearchResult result = Builder(problem, heuristic).BuildQGbfs(batchTarget: 2).Search();
        Assert.Equal(SearchStatus.Failed, result.Status);
        Assert.Equal(2, heuristic.LargestBatch);
        Assert.Equal(4, result.Statistics.VisitedStates);
        Assert.Equal(4, result.Statistics.GeneratedTransitions);
        Assert.Equal(4, result.Statistics.EvaluatedCandidates);
    }

    [Fact]
    public void GoalReturnsWithoutEvaluation()
    {
        Problem problem = Problem("left");
        var heuristic = new BatchQ();
        SearchResult result = Builder(problem, heuristic).BuildQGbfs().Search();
        Assert.True(result.IsSuccess);
        Assert.Equal(0, heuristic.LargestBatch);
        Assert.Null(Assert.Single(result.ActionValues));
        Assert.Equal(1, result.Statistics.GeneratedTransitions);
        Assert.Equal(0, result.Statistics.EvaluatedCandidates);
    }

    [Theory]
    [InlineData(double.NaN)]
    [InlineData(double.PositiveInfinity)]
    [InlineData(double.NegativeInfinity)]
    public void NonfiniteScoresAreRejected(double value)
    {
        Problem problem = Problem();
        var heuristic = new ScalarQ { Value = value };
        InvalidOperationException error = Assert.Throws<InvalidOperationException>(
            () => Builder(problem, heuristic).BuildQGbfs().Search());
        Assert.Contains("row 0, action 0 must be finite", error.Message);
    }

    [Fact]
    public void CancellationAfterGenerationDoesNotStartInference()
    {
        Problem problem = Problem();
        var heuristic = new BatchQ();
        using var cancellation = new CancellationTokenSource();
        ISearchAlgorithm search = Builder(problem, heuristic).BuildQGbfs();
        int generated = 0;
        search.TransitionGenerated += _ =>
        {
            if (++generated == 2) cancellation.Cancel();
        };
        SearchResult result = search.Search(cancellation.Token);
        Assert.True(result.IsCanceled);
        Assert.Equal(2, result.Statistics.GeneratedTransitions);
        Assert.Equal(0, heuristic.LargestBatch);
    }

    [Fact]
    public void StopReturnsBestScoredPartialPath()
    {
        Problem problem = Problem();
        SearchResult result = Builder(problem, new ScalarQ()).BuildQGbfs(shouldStop: _ => true).Search();
        Assert.Equal(SearchStatus.Canceled, result.Status);
        Assert.Empty(result.Plan);
        Assert.Equal("best", Assert.Single(result.PartialPlan).Schema.Name);
        Assert.Equal(10d, Assert.Single(result.ActionValues));
        Assert.Equal(1, result.Statistics.MaxDepth);
        Assert.Equal(result.EndState, SearchTestHelpers.ApplyPlan(problem.InitialState, result.PartialPlan));
    }

    [Fact]
    public void LimitsAndMissingHeuristicAreRejected()
    {
        Problem problem = Problem();
        Assert.Throws<ArgumentOutOfRangeException>(() => Builder(problem, new ScalarQ()).BuildQGbfs(0));
        var builder = new SearchBuilder().WithInitialState(problem.InitialState).WithGoal(problem)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem));
        Assert.Throws<InvalidOperationException>(() => builder.BuildQGbfs());
    }
}
