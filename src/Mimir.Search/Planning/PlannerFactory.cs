using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;

namespace Mimir.Search.Planning;

public static class PlannerFactory
{
    public static IPlanner Create(string generatorType, string algorithmType = "bfs", string heuristicType = "blind")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(generatorType);
        ArgumentException.ThrowIfNullOrWhiteSpace(algorithmType);
        ArgumentException.ThrowIfNullOrWhiteSpace(heuristicType);

        var generatorFactory = CreateGeneratorFactory(generatorType);
        string algorithm = algorithmType.Trim().ToLowerInvariant();

        return algorithm switch
        {
            "bfs" => new BreadthFirstPlanner(generatorFactory),
            "ucs" => new UniformCostPlanner(generatorFactory),
            "uniform-cost" => new UniformCostPlanner(generatorFactory),
            "uniformcost" => new UniformCostPlanner(generatorFactory),
            "astar" => new AStarPlanner(generatorFactory, CreateValidatedHeuristicFactory(heuristicType)),
            "gbfs" => new GreedyBestFirstPlanner(generatorFactory, CreateValidatedHeuristicFactory(heuristicType)),
            "iw0" => new IteratedWidthPlanner(0, generatorFactory),
            "iw1" => new IteratedWidthPlanner(1, generatorFactory),
            "iw2" => new IteratedWidthPlanner(2, generatorFactory),
            "iw3" => new IteratedWidthPlanner(3, generatorFactory),
            _ => throw new ArgumentException($"Unknown algorithm type '{algorithmType}'.", nameof(algorithmType))
        };
    }

    public static IPlanner CreateFromSpec(string plannerSpec)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(plannerSpec);

        string[] parts = plannerSpec.Split(':', StringSplitOptions.TrimEntries);
        if (parts.Length is < 1 or > 3 || parts.Any(string.IsNullOrWhiteSpace))
            throw new ArgumentException($"Invalid planner specification '{plannerSpec}'. Expected '<generator>[:<algorithm>[:<heuristic>]]'.", nameof(plannerSpec));

        string generator = parts[0];
        string algorithm = parts.Length > 1 ? parts[1] : "bfs";
        string heuristic = parts.Length > 2 ? parts[2] : "blind";

        return Create(generator, algorithm, heuristic);
    }

    private static Func<Problem, GoalCondition, IApplicableActionGenerator, IHeuristic> CreateValidatedHeuristicFactory(
        string heuristicType)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(heuristicType);

        _ = heuristicType.Trim().ToLowerInvariant() switch
        {
            "blind" or "goal-count" or "goalcount" or "lifted-ff" or "liftedff" => true,
            _ => throw new ArgumentException($"Unknown heuristic type '{heuristicType}'.", nameof(heuristicType))
        };

        return (problem, goal, _) => CreateHeuristic(heuristicType, problem, goal);
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

    private static Func<Problem, State, IApplicableActionGenerator> CreateGeneratorFactory(string generatorType)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(generatorType);

        return generatorType.Trim().ToLowerInvariant() switch
        {
            "grounded" => (problem, startState) =>
                new GroundedApplicableActionGenerator(problem, startState, new RpgGrounder()),
            "lifted" => (problem, _) => new CliqueApplicableActionGenerator(problem),
            _ => throw new ArgumentException($"Unknown generator type '{generatorType}'.", nameof(generatorType))
        };
    }
}
