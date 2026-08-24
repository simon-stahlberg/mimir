using System.Collections.ObjectModel;

namespace Mimir.Search.Space;

/// <summary>
/// Rich statistics about the structure of a fully-expanded search space.
/// </summary>
public sealed class SearchSpaceStatistics
{
    public int TotalStates { get; }
    public int TotalTransitions { get; }
    public int GoalStateCount { get; }
    public int DeadEndStateCount { get; }
    public int MaxDepth { get; }
    public double AverageBranchingFactor { get; }
    public double AverageOutDegree { get; }
    public double AverageInDegree { get; }
    public TimeSpan ExpansionTime { get; }
    public IReadOnlyDictionary<int, int> DepthDistribution { get; }
    public IReadOnlyDictionary<int, int> OutDegreeDistribution { get; }

    public SearchSpaceStatistics(
        int TotalStates,
        int TotalTransitions,
        int GoalStateCount,
        int DeadEndStateCount,
        int MaxDepth,
        double AverageBranchingFactor,
        double AverageOutDegree,
        double AverageInDegree,
        TimeSpan ExpansionTime,
        IReadOnlyDictionary<int, int> DepthDistribution,
        IReadOnlyDictionary<int, int> OutDegreeDistribution)
    {
        ArgumentNullException.ThrowIfNull(DepthDistribution);
        ArgumentNullException.ThrowIfNull(OutDegreeDistribution);

        this.TotalStates = TotalStates;
        this.TotalTransitions = TotalTransitions;
        this.GoalStateCount = GoalStateCount;
        this.DeadEndStateCount = DeadEndStateCount;
        this.MaxDepth = MaxDepth;
        this.AverageBranchingFactor = AverageBranchingFactor;
        this.AverageOutDegree = AverageOutDegree;
        this.AverageInDegree = AverageInDegree;
        this.ExpansionTime = ExpansionTime;
        this.DepthDistribution = new ReadOnlyDictionary<int, int>(new Dictionary<int, int>(DepthDistribution));
        this.OutDegreeDistribution = new ReadOnlyDictionary<int, int>(new Dictionary<int, int>(OutDegreeDistribution));
    }
}
