using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>Compares iterated width against its serialized form, and records the width each round settled on.</summary>
internal static class SiwCoverage
{
    public static void Run(string benchmarkRoot, int seconds, int maximumWidth)
    {
        int instances = 0;
        int iwSolved = 0;
        int siwSolved = 0;
        int invalid = 0;
        double iwSeconds = 0;
        double siwSeconds = 0;
        var roundWidths = new SortedDictionary<int, int>();

        foreach ((string domainPath, string problemPath) in Discover(benchmarkRoot))
        {
            Domain domain;
            Problem problem;
            try
            {
                domain = Domain.FromFile(domainPath);
                problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
            }
            catch (Exception exception) when (exception is NotSupportedException
                                                  or Mimir.Pddl.Analysis.PddlValidationException or PddlLoadException)
            {
                continue;
            }

            instances++;
            var goal = GoalCondition.FromProblem(problem);

            (bool iwOk, double iwTime) = Solve(new IteratedWidthPlanner(maximumWidth), problem, goal, seconds, null);
            var planner = new SerializedIteratedWidthPlanner(maximumWidth);
            var widths = new List<int>();
            planner.RoundCompleted += widths.Add;
            (bool siwOk, double siwTime) = Solve(planner, problem, goal, seconds, w => invalid += w);

            if (iwOk) iwSolved++;
            if (siwOk)
            {
                siwSolved++;
                foreach (int width in widths)
                    roundWidths[width] = roundWidths.GetValueOrDefault(width) + 1;
            }

            iwSeconds += iwTime;
            siwSeconds += siwTime;
            string name = Path.GetRelativePath(benchmarkRoot, problemPath);
            if (iwOk != siwOk)
                Console.WriteLine($"{name,-50}{(iwOk ? "IW only" : "SIW only")}");
        }

        Console.WriteLine();
        Console.WriteLine($"instances        : {instances}");
        Console.WriteLine($"IW({maximumWidth}) solved     : {iwSolved}");
        Console.WriteLine($"SIW({maximumWidth}) solved    : {siwSolved}");
        Console.WriteLine($"invalid plans    : {invalid}");
        Console.WriteLine($"IW seconds       : {iwSeconds:F1}");
        Console.WriteLine($"SIW seconds      : {siwSeconds:F1}");
        Console.WriteLine("rounds by width  : " + string.Join(
            ", ", roundWidths.Select(entry => $"k={entry.Key}: {entry.Value}")));
    }

    private static (bool Solved, double Seconds) Solve(
        IPlanner planner,
        Problem problem,
        GoalCondition goal,
        int seconds,
        System.Action<int>? reportInvalid)
    {
        using var cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(seconds));
        var stopwatch = Stopwatch.StartNew();
        PlanResult result = planner.Solve(problem, cancellationToken: cancellation.Token);
        stopwatch.Stop();
        if (!result.IsSuccess) return (false, stopwatch.Elapsed.TotalSeconds);

        // A reported plan that does not reach the goal would be worse than no plan at all.
        Mimir.Core.Grounding.State state = problem.InitialState;
        foreach (GroundAction action in result.Plan)
        {
            if (!action.IsApplicable(state.Expand())) { reportInvalid?.Invoke(1); return (false, stopwatch.Elapsed.TotalSeconds); }
            state = state.Expand().Apply(action);
        }

        if (goal.IsSatisfied(state.Expand())) return (true, stopwatch.Elapsed.TotalSeconds);
        reportInvalid?.Invoke(1);
        return (false, stopwatch.Elapsed.TotalSeconds);
    }

    private static IEnumerable<(string DomainPath, string ProblemPath)> Discover(string root)
    {
        foreach (string category in new[] { "strips", "adl" })
        {
            string categoryPath = Path.Combine(root, category);
            if (!Directory.Exists(categoryPath)) continue;

            foreach (string directory in Directory.EnumerateDirectories(categoryPath).OrderBy(p => p, StringComparer.Ordinal))
            {
                string domainPath = Path.Combine(directory, "domain.pddl");
                if (!File.Exists(domainPath)) continue;

                string? problemPath = Directory
                    .EnumerateFiles(directory, "*.pddl", SearchOption.TopDirectoryOnly)
                    .Where(p => !string.Equals(p, domainPath, StringComparison.OrdinalIgnoreCase))
                    .OrderBy(p => p, StringComparer.Ordinal)
                    .FirstOrDefault();
                if (problemPath is null) continue;

                string paired = Path.Combine(directory, $"{Path.GetFileNameWithoutExtension(problemPath)}-domain.pddl");
                yield return (File.Exists(paired) ? paired : domainPath, problemPath);
            }
        }
    }
}
