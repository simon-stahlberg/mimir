using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;

namespace Mimir.Search.Planning;

public static class PlannerFactory
{
    public static IPlanner Create(string algorithmType = "bfs", string heuristicType = "blind")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(algorithmType);
        ArgumentException.ThrowIfNullOrWhiteSpace(heuristicType);

        string algorithm = algorithmType.Trim().ToLowerInvariant();

        return algorithm switch
        {
            "bfs" => new BreadthFirstPlanner(),
            "ucs" => new UniformCostPlanner(),
            "uniform-cost" => new UniformCostPlanner(),
            "uniformcost" => new UniformCostPlanner(),
            "astar" => new AStarPlanner(CreateValidatedHeuristicFactory(heuristicType)),
            "gbfs" => new GreedyBestFirstPlanner(CreateValidatedHeuristicFactory(heuristicType)),
            "iw0" => new IteratedWidthPlanner(0),
            "iw1" => new IteratedWidthPlanner(1),
            "iw2" => new IteratedWidthPlanner(2),
            "iw3" => new IteratedWidthPlanner(3),
            _ => throw new ArgumentException($"Unknown algorithm type '{algorithmType}'.", nameof(algorithmType))
        };
    }

    public static IPlanner CreateFromSpec(string plannerSpec)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(plannerSpec);

        string[] parts = plannerSpec.Split(':', StringSplitOptions.TrimEntries);
        if (parts.Length is < 1 or > 2 || parts.Any(string.IsNullOrWhiteSpace))
            throw new ArgumentException($"Invalid planner specification '{plannerSpec}'. Expected '<algorithm>[:<heuristic>]'.", nameof(plannerSpec));

        string algorithm = parts[0];
        string heuristic = parts.Length > 1 ? parts[1] : "blind";
        return Create(algorithm, heuristic);
    }

    private static Func<State, GoalCondition, IHeuristic> CreateValidatedHeuristicFactory(
        string heuristicType)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(heuristicType);

        _ = heuristicType.Trim().ToLowerInvariant() switch
        {
            "blind" or "goal-count" or "goalcount" or "lifted-ff" or "liftedff" => true,
            _ => throw new ArgumentException($"Unknown heuristic type '{heuristicType}'.", nameof(heuristicType))
        };

        return (start, goal) => CreateHeuristic(heuristicType, start.Context.Problem, goal);
    }

    public static IHeuristic CreateHeuristic(string heuristicType, Problem problem, GoalCondition? goal = null)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(heuristicType);
        ArgumentNullException.ThrowIfNull(problem);

        return heuristicType.Trim().ToLowerInvariant() switch
        {
            "blind" => BlindHeuristic.Instance,
            "goal-count" => new GoalCountHeuristic(problem, goal),
            "goalcount" => new GoalCountHeuristic(problem, goal),
            "lifted-ff" => new LiftedFfHeuristic(problem, goal),
            "liftedff" => new LiftedFfHeuristic(problem, goal),
            _ => throw new ArgumentException($"Unknown heuristic type '{heuristicType}'.", nameof(heuristicType))
        };
    }

}
