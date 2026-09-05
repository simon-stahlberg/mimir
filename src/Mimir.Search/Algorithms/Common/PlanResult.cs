namespace Mimir.Search;

public sealed class PlanResult
{
    public SearchStatus Status { get; }
    public bool IsSuccess => Status == SearchStatus.Succeeded;
    public bool IsCanceled => Status == SearchStatus.Canceled;
    public bool IsExpansionLimitReached => Status == SearchStatus.ExpansionLimitReached;
    public IReadOnlyList<Mimir.Core.Grounding.Action> Plan { get; }
    public SearchStatistics Statistics { get; }
    public IReadOnlyList<Mimir.Core.Grounding.Action> PartialPlan { get; init; } = [];
    public IReadOnlyList<double?> ActionValues { get; init; } = [];
    public Mimir.Core.Grounding.State? EndState { get; init; }
    public TimeSpan SetupTime { get; }
    public TimeSpan SearchTime { get; }
    public TimeSpan ElapsedTime { get; }
    public int PlanLength => Plan.Count;

    public PlanResult(
        SearchStatus status,
        IReadOnlyList<Mimir.Core.Grounding.Action> plan,
        SearchStatistics statistics,
        TimeSpan setupTime,
        TimeSpan searchTime)
    {
        ArgumentNullException.ThrowIfNull(plan);
        ArgumentNullException.ThrowIfNull(statistics);
        if (!Enum.IsDefined(status))
            throw new ArgumentOutOfRangeException(nameof(status));
        if (status != SearchStatus.Succeeded && plan.Count != 0)
            throw new ArgumentException("Only a successful plan result can contain a plan.", nameof(plan));
        if (setupTime < TimeSpan.Zero)
            throw new ArgumentOutOfRangeException(nameof(setupTime));
        if (searchTime < TimeSpan.Zero)
            throw new ArgumentOutOfRangeException(nameof(searchTime));

        Status = status;
        Plan = Array.AsReadOnly(plan.ToArray());
        Statistics = statistics;
        SetupTime = setupTime;
        SearchTime = searchTime;
        ElapsedTime = setupTime + searchTime;
    }

    public double PlanCost
    {
        get
        {
            if (!IsSuccess)
                return 0d;

            double totalCost = 0d;
            foreach (var action in Plan)
                totalCost += action.Cost;

            return totalCost;
        }
    }
}
