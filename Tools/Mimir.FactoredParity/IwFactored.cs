using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Planning;

namespace Mimir.FactoredParity;

/// <summary>Runs IW(1) over the same instance with the lifted and the factored generator and compares.</summary>
internal static class IwFactored
{
    public static void Sweep(string benchmarkRoot, int seconds)
    {
        int compared = 0;
        int disagreements = 0;
        double liftedSeconds = 0;
        double factoredSeconds = 0;

        foreach ((string domainPath, string problemPath) in Discover(benchmarkRoot))
        {
            Domain domain;
            Problem lifted;
            Problem factored;
            try
            {
                domain = Domain.FromFile(domainPath);
                lifted = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Lifted);
                factored = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
            }
            catch (Exception exception) when (exception is NotSupportedException
                                                  or Mimir.Pddl.Analysis.PddlValidationException or PddlLoadException)
            {
                continue;
            }

            (SearchStatus Status, int Expanded, double Seconds) left = Solve(lifted, seconds);
            (SearchStatus Status, int Expanded, double Seconds) right = Solve(factored, seconds);
            compared++;
            liftedSeconds += left.Seconds;
            factoredSeconds += right.Seconds;

            if (left.Status == right.Status && left.Expanded == right.Expanded) continue;

            disagreements++;
            Console.WriteLine($"DIFFERS {Path.GetRelativePath(benchmarkRoot, problemPath),-48}"
                + $"lifted {left.Status}/{left.Expanded}  factored {right.Status}/{right.Expanded}");
        }

        Console.WriteLine();
        Console.WriteLine($"instances compared : {compared}");
        Console.WriteLine($"differing outcomes : {disagreements}");
        Console.WriteLine($"lifted seconds     : {liftedSeconds:F1}");
        Console.WriteLine($"factored seconds   : {factoredSeconds:F1}");
    }

    private static (SearchStatus Status, int Expanded, double Seconds) Solve(Problem problem, int seconds)
    {
        using var cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(seconds));
        var stopwatch = Stopwatch.StartNew();
        PlanResult result = new IteratedWidthPlanner(1).Solve(problem, cancellationToken: cancellation.Token);
        stopwatch.Stop();
        return (result.Status, result.Statistics.NodesExpanded, stopwatch.Elapsed.TotalSeconds);
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

    public static void Run(string domainPath, string problemPath, int seconds)
    {
        var domain = Domain.FromFile(domainPath);
        Console.WriteLine($"{Path.GetFileName(problemPath),-28}{"status",-12}{"expanded",10}{"generated",12}{"seconds",10}");

        foreach (ApplicableActionGeneratorType generatorType in
                 new[] { ApplicableActionGeneratorType.Lifted, ApplicableActionGeneratorType.Factored })
        {
            Problem problem;
            try
            {
                problem = Problem.FromFile(domain, problemPath, generatorType);
            }
            catch (NotSupportedException exception)
            {
                Console.WriteLine($"  {generatorType,-26}unsupported: {exception.Message}");
                continue;
            }

            using var cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(seconds));
            var stopwatch = Stopwatch.StartNew();
            PlanResult result = new IteratedWidthPlanner(1).Solve(problem, cancellationToken: cancellation.Token);
            stopwatch.Stop();

            string status = result.IsSuccess ? $"plan({result.PlanLength})" : result.Status.ToString();
            Console.WriteLine($"  {generatorType,-26}{status,-12}{result.Statistics.NodesExpanded,10}"
                + $"{result.Statistics.NodesGenerated,12}{stopwatch.Elapsed.TotalSeconds,10:F2}");
        }
    }
}
