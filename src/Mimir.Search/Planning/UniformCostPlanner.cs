using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Planning;

public class UniformCostPlanner : IPlanner
{
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

        var ucs = new SearchBuilder()
            .WithInitialState(inputs.StartState)
            .WithGoal(inputs.Goal)
            .WithDeadEndDetector(deadEndDetector)
            .BuildUcs();

        setupStopwatch.Stop();
        if (cancellationToken.IsCancellationRequested)
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);

        var searchStopwatch = Stopwatch.StartNew();
        SearchResult result = ucs.Search(cancellationToken);
        searchStopwatch.Stop();

        return PlannerExecution.ToPlanResult(result, setupStopwatch.Elapsed, searchStopwatch.Elapsed);
    }
}
