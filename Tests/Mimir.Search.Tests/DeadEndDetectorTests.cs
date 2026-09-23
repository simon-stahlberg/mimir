using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;

namespace Mimir.Search.Tests;

public sealed class DeadEndDetectorTests
{
    [Fact]
    public void H2ReusesInitialGroundingAcrossStatesAndGoals()
    {
        Problem problem = CreateProblem();
        var generator = problem.GetApplicableActionGenerator(problem.InitialState);
        var lose = generator.GetApplicableActions(problem.InitialState.Expand()).Single(a => a.Schema.Name == "lose");
        ExtendedState dead = problem.InitialState.Expand().Apply(lose).Expand();
        problem.GetApplicableActionGenerator(dead.State);
        var heuristic = new H2Heuristic(problem);
        var detector = new H2DeadEndDetector(problem);
        GoalCondition goal = GoalCondition.FromProblem(problem);
        GoalCondition hindsight = GoalCondition.FromLiterals(problem,
            [new Literal<Fact>(SearchTestHelpers.GetFluentFact(problem, "dead"), Polarity.Positive)],
            []);

        Assert.Equal(2d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
        Assert.True(detector.IsDeadEnd(dead, goal));
        Assert.False(detector.IsDeadEnd(dead, hindsight));
        Assert.False(detector.IsDeadEnd(problem.InitialState.Expand(), goal));
        Assert.Equal(2d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
        Assert.True(detector.IsDeadEnd(dead, goal));
        Assert.Throws<ArgumentException>(() => detector.IsDeadEnd(CreateProblem().InitialState.Expand(), goal));
        Assert.Throws<NotSupportedException>(() => detector.IsDeadEnd(dead, SearchTestHelpers.ContradictoryGoal(problem)));
    }

    [Fact]
    public void DisjunctionShortCircuitsAndCopiesItsInputs()
    {
        Problem problem = CreateProblem();
        ExtendedState state = problem.InitialState.Expand();
        GoalCondition goal = GoalCondition.FromProblem(problem);
        var calls = new List<int>();
        IDeadEndDetector[] children =
        [
            new DelegateDetector((_, _) => { calls.Add(1); return false; }),
            new DelegateDetector((_, _) => { calls.Add(2); return true; }),
            new DelegateDetector((_, _) => throw new InvalidOperationException("Should short-circuit."))
        ];
        var detector = new DisjunctiveDeadEndDetector(children);
        children[0] = children[2];

        Assert.True(detector.IsDeadEnd(state, goal));
        Assert.Equal([1, 2], calls);
        Assert.False(new DisjunctiveDeadEndDetector().IsDeadEnd(state, goal));
        Assert.False(new DisjunctiveDeadEndDetector(new DelegateDetector((_, _) => false)).IsDeadEnd(state, goal));
        Assert.Throws<ArgumentNullException>(() => new DisjunctiveDeadEndDetector([null!]));
    }

    [Theory]
    [InlineData("bfs")]
    [InlineData("ucs")]
    [InlineData("astar")]
    [InlineData("gbfs")]
    [InlineData("iw")]
    [InlineData("qgbfs")]
    [InlineData("beam")]
    [InlineData("qbeam")]
    public void SearchesPruneDeadEndsAndPreserveSolutions(string algorithm)
    {
        Problem problem = CreateProblem();
        var expanded = new List<State>();
        var pruned = new List<State>();
        var detector = new H2DeadEndDetector(problem);
        SearchBuilder builder = Builder(problem, detector).OnNodeExpanded(node => expanded.Add(node.State));
        ISearchAlgorithm search = Build(builder, algorithm);
        search.TransitionPruned += transition => pruned.Add(transition.SuccessorState);

        SearchResult result = search.Search(maxExpandedStates: 20);

        Assert.True(result.IsSuccess);
        Assert.Equal(["advance", "finish"], result.Plan.Select(action => action.Schema.Name));
        Fact<Fluent> dead = SearchTestHelpers.GetFluentFact(problem, "dead");
        Assert.DoesNotContain(expanded, state => state.IsTrue(dead));
        Assert.Contains(pruned, state => state.IsTrue(dead));

        SearchResult unchanged = Build(Builder(problem, new DelegateDetector((_, _) => false)), algorithm)
            .Search(maxExpandedStates: 20);
        SearchResult baseline = Build(Builder(problem, null), algorithm).Search(maxExpandedStates: 20);
        Assert.Equal(baseline.Status, unchanged.Status);
        Assert.Equal(baseline.Plan, unchanged.Plan);
        Assert.Equal(baseline.Statistics.NodesExpanded, unchanged.Statistics.NodesExpanded);
    }

    [Theory]
    [InlineData("bfs")]
    [InlineData("ucs")]
    [InlineData("astar")]
    [InlineData("gbfs")]
    [InlineData("iw")]
    [InlineData("qgbfs")]
    [InlineData("beam")]
    [InlineData("qbeam")]
    public void SearchesRecognizeGoalBeforeCallingDetector(string algorithm)
    {
        Problem problem = CreateProblem();
        var detector = new DelegateDetector((_, _) => throw new InvalidOperationException("Goal is not a dead end."));
        SearchBuilder builder = Builder(problem, detector).WithGoal(GoalCondition.Always(problem));
        Assert.True(Build(builder, algorithm).Search().IsSuccess);
    }

    [Theory]
    [InlineData("bfs")]
    [InlineData("ucs")]
    [InlineData("astar")]
    [InlineData("gbfs")]
    [InlineData("iw")]
    [InlineData("qgbfs")]
    [InlineData("beam")]
    [InlineData("qbeam")]
    public void SearchesRejectDeadEndRootsWithoutExpanding(string algorithm)
    {
        Problem problem = CreateProblem();
        var detector = new H2DeadEndDetector(problem);
        ExtendedState initial = problem.InitialState.Expand();
        var lose = problem.GetApplicableActionGenerator(problem.InitialState)
            .GetApplicableActions(initial).Single(action => action.Schema.Name == "lose");
        SearchBuilder builder = Builder(problem, detector).WithInitialState(initial.Apply(lose));

        SearchResult result = Build(builder, algorithm).Search();

        Assert.False(result.IsSuccess);
        Assert.Equal(0, result.Statistics.NodesExpanded);
    }

    [Theory]
    [InlineData("qgbfs", true)]
    [InlineData("qgbfs", false)]
    [InlineData("qbeam", true)]
    [InlineData("qbeam", false)]
    public void QModelsReceiveFullRowsBeforeDeadEndsAreMasked(string algorithm, bool maximize)
    {
        Problem problem = CreateProblem();
        var model = new FullRowQ(maximize);
        SearchBuilder builder = Builder(problem, new H2DeadEndDetector(problem)).WithQHeuristic(model);
        ISearchAlgorithm search = algorithm == "qgbfs"
            ? builder.BuildQGbfs(maximize: maximize)
            : builder.BuildQBeam(beamSize: 1, maximize: maximize);

        Assert.True(search.Search().IsSuccess);
        Assert.Equal(1, model.Calls);
    }

    [Fact]
    public void BeamScoresDeadEndCandidatesBeforeMaskingThem()
    {
        Problem problem = CreateProblem();
        var detector = new H2DeadEndDetector(problem);
        Fact<Fluent> dead = SearchTestHelpers.GetFluentFact(problem, "dead");
        var scored = new List<State>();
        var model = new DelegateHeuristic((state, _) =>
        {
            scored.Add(state.State);
            return new HeuristicEvaluation(state.State.IsTrue(dead) ? 0 : 10);
        });
        ISearchAlgorithm search = Builder(problem, detector).WithHeuristic(model).BuildBeam(1);
        Assert.True(search.Search().IsSuccess);
        Assert.Equal(2, scored.Count);
        Assert.Contains(scored, state => state.IsTrue(dead));
    }

    private static SearchBuilder Builder(Problem problem, IDeadEndDetector? detector)
        => new SearchBuilder().WithInitialState(problem.InitialState).WithGoal(problem)
            .WithHeuristic(BlindHeuristic.Instance).WithQHeuristic(new FullRowQ(true))
            .WithDeadEndDetector(detector);

    private static ISearchAlgorithm Build(SearchBuilder builder, string algorithm) => algorithm switch
    {
        "bfs" => builder.BuildBfs(),
        "ucs" => builder.BuildUcs(),
        "astar" => builder.BuildAStar(),
        "gbfs" => builder.BuildGbfs(),
        "iw" => builder.BuildIw(1),
        "qgbfs" => builder.BuildQGbfs(),
        "beam" => builder.BuildBeam(2),
        "qbeam" => builder.BuildQBeam(2),
        _ => throw new ArgumentException(algorithm)
    };

    private sealed class DelegateDetector(Func<ExtendedState, GoalCondition, bool> detect) : IDeadEndDetector
    {
        public bool IsDeadEnd(ExtendedState state, GoalCondition goal) => detect(state, goal);
    }

    private sealed class FullRowQ(bool maximize) : IQHeuristic
    {
        public int Calls { get; private set; }

        public QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null)
        {
            Calls++;
            if (Calls == 1)
                Assert.Equal(["lose", "advance"], successors.Select(pair => pair.Item1.Schema.Name));
            return new QHeuristicEvaluation(successors.Select(pair =>
                pair.Item1.Schema.Name == "lose" ? (maximize ? 100d : -100d) : 0d).ToArray());
        }
    }

    private static Problem CreateProblem() => SearchTestHelpers.CreateProblemFromText("""
        (define (domain detection) (:requirements :strips)
          (:predicates (ready) (safe) (dead) (goal))
          (:action lose :parameters () :precondition (ready) :effect (and (not (ready)) (dead)))
          (:action advance :parameters () :precondition (ready) :effect (and (not (ready)) (safe)))
          (:action loop :parameters () :precondition (dead) :effect (dead))
          (:action finish :parameters () :precondition (safe) :effect (goal)))
        """, "(define (problem p) (:domain detection) (:init (ready)) (:goal (goal)))");
}
