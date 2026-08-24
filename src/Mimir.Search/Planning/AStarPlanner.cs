using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;

namespace Mimir.Search.Planning;

public class AStarPlanner : IPlanner
{
    private readonly Func<Problem, State, IApplicableActionGenerator> _generatorFactory;
    private readonly Func<Problem, GoalCondition, IApplicableActionGenerator, IHeuristic> _heuristicFactory;

    public AStarPlanner(
        Func<Problem, State, IApplicableActionGenerator> generatorFactory,
        Func<Problem, GoalCondition, IApplicableActionGenerator, IHeuristic>? heuristicFactory = null)
    {
        _generatorFactory = generatorFactory ?? throw new ArgumentNullException(nameof(generatorFactory));
        _heuristicFactory = heuristicFactory ?? ((_, _, _) => BlindHeuristic.Instance);
    }

    public PlanResult Solve(
        Problem problem,
        State? startState = null,
        GoalCondition? goal = null,
        CancellationToken cancellationToken = default)
    {
        var setupStopwatch = Stopwatch.StartNew();
        PlannerInputs inputs = PlannerExecution.ResolveInputs(problem, startState, goal);
        if (cancellationToken.IsCancellationRequested)
        {
            setupStopwatch.Stop();
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);
        }

        IApplicableActionGenerator generator = _generatorFactory(problem, inputs.StartState);
        PlannerExecution.ValidateGenerator(problem, inputs.StartState, generator);

        IHeuristic heuristic = _heuristicFactory(problem, inputs.Goal, generator)
            ?? throw new InvalidOperationException("Heuristic factory returned null.");

        var astar = new SearchBuilder()
            .WithInitialState(inputs.StartState)
            .WithGoal(inputs.Goal)
            .WithActionGenerator(generator)
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
