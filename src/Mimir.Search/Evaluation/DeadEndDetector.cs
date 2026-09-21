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
