using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Heuristics;

namespace Mimir.Search.Evaluation;

public interface IDeadEndDetector
{
    bool IsDeadEnd(ExtendedState state, GoalCondition goal);
}

public sealed class H2DeadEndDetector : IDeadEndDetector
{
    private readonly H2Heuristic _heuristic;

    public H2DeadEndDetector(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        _heuristic = new H2Heuristic(problem, GoalCondition.Always(problem));
    }

    public bool IsDeadEnd(ExtendedState state, GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        return double.IsPositiveInfinity(_heuristic.Evaluate(state, goal).Value);
    }
}

/// <summary>
/// Reports a state as a dead end when the delete relaxation cannot reach the goal from it. Weaker than h2, but
/// it needs no precomputation over atom pairs, so it stays affordable on instances where h2 does not.
/// </summary>
public sealed class FfDeadEndDetector : IDeadEndDetector
{
    private readonly LiftedFfHeuristic? _heuristic;
    private readonly FactoredRelaxedReachability? _factored;

    public FfDeadEndDetector(Problem problem)
        : this(problem, factored: true)
    {
    }

    /// <summary>Reachability over factored groups where the problem allows it, enumerating bindings otherwise.</summary>
    public FfDeadEndDetector(Problem problem, bool factored)
    {
        ArgumentNullException.ThrowIfNull(problem);
        _factored = factored ? FactoredRelaxedReachability.TryCreate(problem) : null;
        if (_factored is null) _heuristic = new LiftedFfHeuristic(problem);
    }

    public bool UsesFactoredGraph => _factored is not null;

    public bool IsDeadEnd(ExtendedState state, GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        return _factored is not null
            ? !_factored.CanReach(state, goal)
            : double.IsPositiveInfinity(_heuristic!.Evaluate(state, goal).Value);
    }
}

public sealed class DisjunctiveDeadEndDetector : IDeadEndDetector
{
    private readonly IDeadEndDetector[] _detectors;

    public DisjunctiveDeadEndDetector(params IDeadEndDetector[] detectors)
    {
        ArgumentNullException.ThrowIfNull(detectors);
        foreach (IDeadEndDetector detector in detectors)
            ArgumentNullException.ThrowIfNull(detector);
        _detectors = detectors.ToArray();
    }

    public bool IsDeadEnd(ExtendedState state, GoalCondition goal)
    {
        ArgumentNullException.ThrowIfNull(state);
        ArgumentNullException.ThrowIfNull(goal);
        if (!ReferenceEquals(state.State.Context.Problem, goal.Problem))
            throw new ArgumentException("State and goal belong to different problems.", nameof(goal));

        foreach (IDeadEndDetector detector in _detectors)
        {
            if (detector.IsDeadEnd(state, goal))
                return true;
        }
        return false;
    }
}
