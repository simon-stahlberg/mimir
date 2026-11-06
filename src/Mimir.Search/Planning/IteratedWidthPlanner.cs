using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Planning;

public class IteratedWidthPlanner : IPlanner
{
    private readonly int _k;
    private readonly Algorithms.IteratedWidth.NoveltyKind _noveltyKind;

    public IteratedWidthPlanner(int k)
        : this(k, Algorithms.IteratedWidth.NoveltyKind.Atoms)
    {
    }

    public IteratedWidthPlanner(int k, Algorithms.IteratedWidth.NoveltyKind noveltyKind)
    {
        if (k is < 0 or > 3)
            throw new ArgumentOutOfRangeException(nameof(k), "IW(k) supports k = 0..3.");

        _k = k;
        _noveltyKind = noveltyKind;
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

        var iw = new SearchBuilder()
            .WithInitialState(inputs.StartState)
            .WithGoal(inputs.Goal)
            .WithDeadEndDetector(deadEndDetector)
            .BuildIw(_k, _noveltyKind);

        setupStopwatch.Stop();
        if (cancellationToken.IsCancellationRequested)
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);

        var searchStopwatch = Stopwatch.StartNew();
        SearchResult result = iw.Search(cancellationToken);
        searchStopwatch.Stop();

        return PlannerExecution.ToPlanResult(result, setupStopwatch.Elapsed, searchStopwatch.Elapsed);
    }
}
