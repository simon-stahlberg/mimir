using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Planning;

public class BreadthFirstPlanner : IPlanner
{
    private readonly Func<Problem, State, IApplicableActionGenerator> _generatorFactory;

    public BreadthFirstPlanner(Func<Problem, State, IApplicableActionGenerator> generatorFactory)
    {
        _generatorFactory = generatorFactory ?? throw new ArgumentNullException(nameof(generatorFactory));
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

        var bfs = new SearchBuilder()
            .WithInitialState(inputs.StartState)
            .WithGoal(inputs.Goal)
            .WithActionGenerator(generator)
            .BuildBfs();

        setupStopwatch.Stop();
        if (cancellationToken.IsCancellationRequested)
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);

        var searchStopwatch = Stopwatch.StartNew();
        SearchResult result = bfs.Search(cancellationToken);
        searchStopwatch.Stop();

        return PlannerExecution.ToPlanResult(result, setupStopwatch.Elapsed, searchStopwatch.Elapsed);
    }
}
