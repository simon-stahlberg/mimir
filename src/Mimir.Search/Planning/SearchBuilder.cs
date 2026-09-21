using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Algorithms.Beam;
using Mimir.Search.Algorithms.BreadthFirst;
using Mimir.Search.Algorithms.AStar;
using Mimir.Search.Algorithms.GreedyBestFirst;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Algorithms.UniformCost;

namespace Mimir.Search.Planning;

public class SearchBuilder
{
    private State? _initialState;
    private GoalCondition? _goalCondition;
    private IHeuristic? _heuristic;
    private IDeadEndDetector? _deadEndDetector;
    private IQHeuristic? _qHeuristic;
    private readonly List<Action<SearchNode>> _onNodeExpanded = new();
    private readonly List<Action<SearchNode>> _onNodeGenerated = new();
    private readonly List<Action<SearchNode>> _onGoalNodeExpanded = new();
    private readonly List<Action<SearchTransition>> _onStateGenerated = new();
    private readonly List<Action<SearchTransition>> _onStateGeneratedInSearchTree = new();
    private readonly List<Action<SearchTransition>> _onStateGeneratedNotInSearchTree = new();
    private readonly List<Action<State>> _onStatePruned = new();
    private readonly List<Action<double>> _onGLayerFinished = new();
    private readonly List<Action<double>> _onFLayerFinished = new();
    private readonly List<Action<double>> _onNewBestHValue = new();
    private readonly List<Action<int>> _onWidthStarted = new();

    public SearchBuilder WithInitialState(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        _initialState = state;
        return this;
    }

    public SearchBuilder WithGoal(GoalCondition goalCondition)
    {
        ArgumentNullException.ThrowIfNull(goalCondition);
        _goalCondition = goalCondition;
        return this;
    }

    public SearchBuilder WithGoal(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        _goalCondition = GoalCondition.FromProblem(problem);
        return this;
    }

    public SearchBuilder WithDeadEndDetector(IDeadEndDetector? detector)
    {
        _deadEndDetector = detector;
        return this;
    }

    public SearchBuilder WithHeuristic(IHeuristic heuristic)
    {
        ArgumentNullException.ThrowIfNull(heuristic);
        _heuristic = heuristic;
        return this;
    }

    public SearchBuilder OnNodeExpanded(Action<SearchNode> callback)
    {
        _onNodeExpanded.Add(callback);
        return this;
    }

    public SearchBuilder OnNodeGenerated(Action<SearchNode> callback)
    {
        _onNodeGenerated.Add(callback);
        return this;
    }

    public SearchBuilder WithQHeuristic(IQHeuristic heuristic)
    {
        ArgumentNullException.ThrowIfNull(heuristic);
        _qHeuristic = heuristic;
        return this;
    }

    public ISearchAlgorithm BuildQGbfs(int batchTarget = 1, bool maximize = true, Func<int, bool>? shouldStop = null)
    {
        Validate();
        var search = new QGbfsSearch(
            _initialState!, _goalCondition!,
            _qHeuristic ?? throw new InvalidOperationException("Q-heuristic not set."),
            batchTarget, maximize, shouldStop, _deadEndDetector);
        foreach (var callback in _onNodeExpanded) search.NodeExpanded += callback;
        foreach (var callback in _onNodeGenerated) search.NodeGenerated += callback;
        foreach (var callback in _onStateGenerated) search.TransitionGenerated += callback;
        return search;
    }

    public ISearchAlgorithm BuildBeam(int beamSize = 1, int maxDepth = int.MaxValue, Func<int, bool>? shouldStop = null)
    {
        Validate();
        var search = new BeamSearch(_initialState!, _goalCondition!,
            _heuristic ?? BlindHeuristic.Instance, null, beamSize, maxDepth, false, shouldStop, _deadEndDetector);
        foreach (var callback in _onNodeExpanded) search.NodeExpanded += callback;
        foreach (var callback in _onNodeGenerated) search.NodeGenerated += callback;
        foreach (var callback in _onStateGenerated) search.TransitionGenerated += callback;
        foreach (var callback in _onStateGeneratedInSearchTree) search.TransitionDiscovered += callback;
        foreach (var callback in _onStateGeneratedNotInSearchTree) search.TransitionPruned += callback;
        return search;
    }

    public ISearchAlgorithm BuildQBeam(int beamSize = 1, int maxDepth = int.MaxValue,
        bool maximize = true, Func<int, bool>? shouldStop = null)
    {
        Validate();
        var search = new BeamSearch(_initialState!, _goalCondition!, null,
            _qHeuristic ?? throw new InvalidOperationException("Q-heuristic not set."),
            beamSize, maxDepth, maximize, shouldStop, _deadEndDetector);
        foreach (var callback in _onNodeExpanded) search.NodeExpanded += callback;
        foreach (var callback in _onNodeGenerated) search.NodeGenerated += callback;
        foreach (var callback in _onStateGenerated) search.TransitionGenerated += callback;
        foreach (var callback in _onStateGeneratedInSearchTree) search.TransitionDiscovered += callback;
        foreach (var callback in _onStateGeneratedNotInSearchTree) search.TransitionPruned += callback;
        return search;
    }

    public SearchBuilder OnGoalNodeExpanded(Action<SearchNode> callback)
    {
        _onGoalNodeExpanded.Add(callback);
        return this;
    }

    public SearchBuilder OnStateGenerated(Action<SearchTransition> callback)
    {
        _onStateGenerated.Add(callback);
        return this;
    }

    public SearchBuilder OnStateGeneratedInSearchTree(Action<SearchTransition> callback)
    {
        _onStateGeneratedInSearchTree.Add(callback);
        return this;
    }

    public SearchBuilder OnStateGeneratedNotInSearchTree(Action<SearchTransition> callback)
    {
        _onStateGeneratedNotInSearchTree.Add(callback);
        return this;
    }

    public SearchBuilder OnStatePruned(Action<State> callback)
    {
        _onStatePruned.Add(callback);
        return this;
    }

    public SearchBuilder OnGLayerFinished(Action<double> callback)
    {
        _onGLayerFinished.Add(callback);
        return this;
    }

    public SearchBuilder OnFLayerFinished(Action<double> callback)
    {
        _onFLayerFinished.Add(callback);
        return this;
    }

    public SearchBuilder OnNewBestHValue(Action<double> callback)
    {
        _onNewBestHValue.Add(callback);
        return this;
    }

    public SearchBuilder OnWidthStarted(Action<int> callback)
    {
        _onWidthStarted.Add(callback);
        return this;
    }

    public ISearchAlgorithm BuildBfs()
    {
        Validate();
        var bfs = new BfsSearch(_initialState!, _goalCondition!, _deadEndDetector);
        foreach (var cb in _onNodeExpanded) bfs.NodeExpanded += cb;
        foreach (var cb in _onNodeGenerated) bfs.NodeGenerated += cb;
        foreach (var cb in _onStateGenerated) bfs.StateGenerated += cb;
        foreach (var cb in _onStateGeneratedInSearchTree) bfs.StateGeneratedInSearchTree += cb;
        foreach (var cb in _onStateGeneratedNotInSearchTree) bfs.StateGeneratedNotInSearchTree += cb;
        foreach (var cb in _onGLayerFinished) bfs.GLayerFinished += cb;
        return bfs;
    }

    public ISearchAlgorithm BuildAStar()
    {
        Validate();
        var astar = new AStarSearch(_initialState!, _goalCondition!, _heuristic ?? BlindHeuristic.Instance, _deadEndDetector);
        foreach (var cb in _onNodeExpanded) astar.NodeExpanded += cb;
        foreach (var cb in _onNodeGenerated) astar.NodeGenerated += cb;
        foreach (var cb in _onStateGenerated) astar.StateGenerated += cb;
        foreach (var cb in _onStatePruned) astar.StatePruned += cb;
        foreach (var cb in _onFLayerFinished) astar.FLayerFinished += cb;
        return astar;
    }

    public ISearchAlgorithm BuildGbfs()
    {
        Validate();
        var gbfs = new GbfsSearch(_initialState!, _goalCondition!, _heuristic ?? BlindHeuristic.Instance, _deadEndDetector);
        foreach (var cb in _onNodeExpanded) gbfs.NodeExpanded += cb;
        foreach (var cb in _onNodeGenerated) gbfs.NodeGenerated += cb;
        foreach (var cb in _onGoalNodeExpanded) gbfs.GoalNodeExpanded += cb;
        foreach (var cb in _onStateGenerated) gbfs.StateGenerated += cb;
        foreach (var cb in _onStatePruned) gbfs.StatePruned += cb;
        foreach (var cb in _onNewBestHValue) gbfs.NewBestHValue += cb;
        return gbfs;
    }

    public ISearchAlgorithm BuildUcs()
    {
        Validate();
        var ucs = new UcsSearch(_initialState!, _goalCondition!, _deadEndDetector);
        foreach (var cb in _onNodeExpanded) ucs.NodeExpanded += cb;
        foreach (var cb in _onNodeGenerated) ucs.NodeGenerated += cb;
        return ucs;
    }

    public ISearchAlgorithm BuildIw(int k)
    {
        if (k is < 0 or > 3)
            throw new ArgumentOutOfRangeException(nameof(k), "IW(k) supports k = 0..3.");
        Validate();
        var iw = new IwSearch(k, _initialState!, _goalCondition!, _deadEndDetector);
        foreach (var cb in _onNodeExpanded) iw.NodeExpanded += cb;
        foreach (var cb in _onNodeGenerated) iw.NodeGenerated += cb;
        foreach (var cb in _onWidthStarted) iw.WidthStarted += cb;
        return iw;
    }

    private void Validate()
    {
        if (_initialState == null) throw new InvalidOperationException("Initial state not set.");
        if (_goalCondition == null) throw new InvalidOperationException("Goal condition not set.");

        Problem problem = _initialState.Context.Problem;
        GoalCondition goalCondition = _goalCondition;
        if (!ReferenceEquals(goalCondition.Problem, problem))
        {
            throw new InvalidOperationException(
                "The initial state and goal condition belong to different problem instances.");
        }
    }

}
