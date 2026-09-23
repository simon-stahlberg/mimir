using Mimir.Core.Grounding;
using Action = Mimir.Core.Grounding.Action;


namespace Mimir.Search;

public enum SearchStatus
{
    Failed = 0,
    Succeeded = 1,
    Canceled = 2,
    ExpansionLimitReached = 3,
    DepthLimitReached = 4,
    DeadEnd = 5
}

public record SearchStatistics(
    int NodesExpanded,
    int NodesGenerated,
    TimeSpan TimeTaken,
    int MaxDepth,
    int? VisitedStates = null,
    int? GeneratedTransitions = null,
    int? EvaluatedCandidates = null
);

public class SearchResult
{
    public SearchStatus Status { get; }
    public bool IsSuccess => Status == SearchStatus.Succeeded;
    public bool IsCanceled => Status == SearchStatus.Canceled;
    public bool IsExpansionLimitReached => Status == SearchStatus.ExpansionLimitReached;
    public IReadOnlyList<Action> Plan { get; }
    public SearchStatistics Statistics { get; }
    public IReadOnlyList<Action> PartialPlan { get; init; } = [];
    public IReadOnlyList<double?> ActionValues { get; init; } = [];
    public State? EndState { get; init; }
    public int PlanLength => Plan.Count;
    public double PlanCost { get; }

    public SearchResult(SearchStatus status, IReadOnlyList<Action> plan, SearchStatistics statistics)
    {
        ArgumentNullException.ThrowIfNull(plan);
        ArgumentNullException.ThrowIfNull(statistics);
        if (!Enum.IsDefined(status))
            throw new ArgumentOutOfRangeException(nameof(status));
        if (status != SearchStatus.Succeeded && plan.Count != 0)
            throw new ArgumentException("Only a successful search result can contain a plan.", nameof(plan));

        IReadOnlyList<Action> planSnapshot = Array.AsReadOnly(plan.ToArray());
        Status = status;
        Plan = planSnapshot;
        Statistics = statistics;
        PlanCost = IsSuccess ? ComputePlanCost(planSnapshot) : 0d;
    }

    public static SearchResult Failure(SearchStatistics stats) =>
        new SearchResult(SearchStatus.Failed, Array.Empty<Action>(), stats);

    public static SearchResult Canceled(SearchStatistics stats) =>
        new SearchResult(SearchStatus.Canceled, Array.Empty<Action>(), stats);

    public static SearchResult ExpansionLimitReached(SearchStatistics stats) =>
        new SearchResult(SearchStatus.ExpansionLimitReached, Array.Empty<Action>(), stats);

    public static SearchResult Success(IReadOnlyList<Action> plan, SearchStatistics stats) =>
        new SearchResult(SearchStatus.Succeeded, plan, stats);

    private static double ComputePlanCost(IReadOnlyList<Action> plan)
    {
        double totalCost = 0d;
        foreach (var action in plan)
            totalCost += action.Cost;

        return totalCost;
    }
}
