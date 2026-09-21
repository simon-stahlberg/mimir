using System.Diagnostics;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;

namespace Mimir.Search.Planning;

public class AStarPlanner : IPlanner
{
    private readonly Func<State, GoalCondition, IHeuristic> _heuristicFactory;

    public AStarPlanner(
        Func<State, GoalCondition, IHeuristic>? heuristicFactory = null)
    {
        _heuristicFactory = heuristicFactory ?? ((_, _) => BlindHeuristic.Instance);
    }

    public PlanResult Solve(
        Problem problem,
        State? startState = null,
        GoalCondition? goal = null,
        CancellationToken cancellationToken = default,
        IDeadEndDetector? deadEndDetector = null)
    {
        var setupStopwatch = Stopwatch.StartNew();
        PlannerInputs inputs = PlannerExecution.ResolveInputs(problem, startState, goal);
        if (cancellationToken.IsCancellationRequested)
        {
            setupStopwatch.Stop();
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);
        }

        IHeuristic heuristic = _heuristicFactory(inputs.StartState, inputs.Goal)
            ?? throw new InvalidOperationException("Heuristic factory returned null.");
        var astar = new SearchBuilder()
            .WithInitialState(inputs.StartState)
            .WithGoal(inputs.Goal)
            .WithDeadEndDetector(deadEndDetector)
            .WithHeuristic(heuristic)
            .BuildAStar();

        setupStopwatch.Stop();
        if (cancellationToken.IsCancellationRequested)
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);

        var searchStopwatch = Stopwatch.StartNew();
        SearchResult result = astar.Search(cancellationToken);
        searchStopwatch.Stop();

        return PlannerExecution.ToPlanResult(result, setupStopwatch.Elapsed, searchStopwatch.Elapsed);
    }
}
