using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Space;

/// <summary>
/// Fluent builder for constructing a fully-expanded SearchSpace.
/// </summary>
public class SearchSpaceBuilder
{
    private State? _initialState;
    private GoalCondition? _goalCondition;
    private int? _maxStates;

    /// <summary>
    /// Sets the initial state for the search space.
    /// </summary>
    public SearchSpaceBuilder WithInitialState(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        _initialState = state;
        return this;
    }

    /// <summary>
    /// Sets the goal condition for the search space.
    /// </summary>
    public SearchSpaceBuilder WithGoal(GoalCondition goalCondition)
    {
        ArgumentNullException.ThrowIfNull(goalCondition);
        _goalCondition = goalCondition;
        return this;
    }

    /// <summary>
    /// Sets the goal condition from a problem's goal definition.
    /// </summary>
    public SearchSpaceBuilder WithGoal(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        _goalCondition = GoalCondition.FromProblem(problem);
        return this;
    }

    /// <summary>
    /// Sets a maximum number of states to expand (safety limit).
    /// Use int.MaxValue or omit for no limit.
    /// </summary>
    public SearchSpaceBuilder WithMaxStates(int maxStates)
    {
        if (maxStates <= 0)
            throw new ArgumentOutOfRangeException(nameof(maxStates), "The maximum number of states must be positive.");

        _maxStates = maxStates;
        return this;
    }

    /// <summary>
    /// Builds the fully-expanded search space.
    /// </summary>
    public SearchSpace Build(CancellationToken cancellationToken = default)
    {
        if (_initialState == null)
            throw new InvalidOperationException("Initial state must be set via WithInitialState().");
        if (_goalCondition == null)
            throw new InvalidOperationException("Goal condition must be set via WithGoal().");

        Problem problem = _initialState.Context.Problem;
        if (!ReferenceEquals(_goalCondition.Problem, problem))
        {
            throw new InvalidOperationException(
                "The initial state and goal condition belong to different problem instances.");
        }

        return new SearchSpace(
            _initialState,
            _goalCondition,
            _maxStates,
            cancellationToken);
    }
}
