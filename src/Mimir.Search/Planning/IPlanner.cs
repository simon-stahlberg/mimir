using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Planning;

public interface IPlanner
{
    /// <summary>
    /// Attempts to find a plan for the given problem.
    /// </summary>
    /// <param name="problem">The problem to solve.</param>
    /// <param name="startState">Optional start state; defaults to the problem's initial state.</param>
    /// <param name="goal">Optional goal condition; defaults to the problem's goal.</param>
    PlanResult Solve(
        Problem problem,
        State? startState = null,
        GoalCondition? goal = null,
        CancellationToken cancellationToken = default);
}

internal readonly record struct PlannerInputs(State StartState, GoalCondition Goal);

internal static class PlannerExecution
{
    internal static PlannerInputs ResolveInputs(Problem problem, State? startState, GoalCondition? goal)
    {
        ArgumentNullException.ThrowIfNull(problem);

        State effectiveStart = startState ?? problem.InitialState;
        if (!ReferenceEquals(effectiveStart.Context, problem.Context))
            throw new ArgumentException("Start state belongs to a different problem instance.", nameof(startState));

        GoalCondition effectiveGoal = goal ?? GoalCondition.FromProblem(problem);
        if (!ReferenceEquals(effectiveGoal.Problem, problem))
            throw new ArgumentException("Goal belongs to a different problem instance.", nameof(goal));

        return new PlannerInputs(effectiveStart, effectiveGoal);
    }

    internal static void ValidateGenerator(
        Problem problem,
        State startState,
        IApplicableActionGenerator? generator)
    {
        if (generator is null)
            throw new InvalidOperationException("Applicable-action generator factory returned null.");

        if (!ReferenceEquals(generator.Problem, problem))
        {
            throw new InvalidOperationException(
                "Applicable-action generator factory returned a generator for a different problem instance.");
        }

        if (generator is GroundedApplicableActionGenerator groundedGenerator
            && !groundedGenerator.GroundingStartState.Equals(startState))
        {
            throw new InvalidOperationException(
                "Applicable-action generator factory returned a grounded generator for a different start state.");
        }
    }

    internal static PlanResult Canceled(TimeSpan setupTime)
    {
        var statistics = new SearchStatistics(0, 0, TimeSpan.Zero, 0);
        return new PlanResult(SearchStatus.Canceled, [], statistics, setupTime, TimeSpan.Zero);
    }

    internal static PlanResult ToPlanResult(
        SearchResult result,
        TimeSpan setupTime,
        TimeSpan searchTime)
    {
        return new PlanResult(result.Status, result.Plan, result.Statistics, setupTime, searchTime);
    }
}
