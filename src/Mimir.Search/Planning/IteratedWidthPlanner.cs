using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Planning;

public class IteratedWidthPlanner : IPlanner
{
    private readonly int _k;
    private readonly Func<Problem, State, IApplicableActionGenerator> _generatorFactory;

    public IteratedWidthPlanner(int k, Func<Problem, State, IApplicableActionGenerator> generatorFactory)
    {
        if (k is < 0 or > 3)
            throw new ArgumentOutOfRangeException(nameof(k), "IW(k) supports k = 0..3.");

        _k = k;
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

        var iw = new SearchBuilder()
            .WithInitialState(inputs.StartState)
            .WithGoal(inputs.Goal)
            .WithActionGenerator(generator)
            .BuildIw(_k);

        setupStopwatch.Stop();
        if (cancellationToken.IsCancellationRequested)
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);

        var searchStopwatch = Stopwatch.StartNew();
        SearchResult result = iw.Search(cancellationToken);
        searchStopwatch.Stop();

        return PlannerExecution.ToPlanResult(result, setupStopwatch.Elapsed, searchStopwatch.Elapsed);
    }
}
