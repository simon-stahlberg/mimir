using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search;
using Mimir.Search.Planning;

namespace Mimir.FactoredParity;

/// <summary>Runs IW(k) under each novelty notion over a benchmark tree and reports coverage and cost.</summary>
internal static class IwComparison
{
    private sealed record Outcome(int Solved, int Attempted, long Expanded, double Seconds);

    public static void Run(string benchmarkRoot, int width, int secondsPerInstance, int instancesPerDomain)
    {
        (string Label, NoveltyKind Kind)[] modes =
        [
            ("atoms", NoveltyKind.Atoms),
            ("arguments", NoveltyKind.Arguments),
            ("both", NoveltyKind.Both),
        ];

        var totals = modes.ToDictionary(mode => mode.Label, _ => new Outcome(0, 0, 0, 0));
        Console.WriteLine($"IW({width}), {secondsPerInstance}s per instance\n");
        Console.WriteLine($"{"instance",-46}{"atoms",14}{"arguments",14}{"both",14}");

        foreach ((string domainPath, string problemPath) in Discover(benchmarkRoot, instancesPerDomain))
        {
            Domain domain;
            Problem problem;
            try
            {
                domain = Domain.FromFile(domainPath);
                problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Lifted);
            }
            catch (Exception exception) when (exception is NotSupportedException or Mimir.Pddl.Analysis.PddlValidationException
                                                  or PddlLoadException)
            {
                continue;
            }

            var cells = new List<string>();
            foreach ((string label, NoveltyKind kind) in modes)
            {
                using var cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(secondsPerInstance));
                var stopwatch = Stopwatch.StartNew();
                PlanResult result;
                try
                {
                    result = new IteratedWidthPlanner(width, kind).Solve(problem, cancellationToken: cancellation.Token);
                }
                catch (Exception exception) when (exception is NotSupportedException)
                {
                    cells.Add("n/a");
                    continue;
                }

                stopwatch.Stop();
                bool solved = result.IsSuccess;
                Outcome running = totals[label];
                totals[label] = running with
                {
                    Solved = running.Solved + (solved ? 1 : 0),
                    Attempted = running.Attempted + 1,
                    Expanded = running.Expanded + result.Statistics.NodesExpanded,
                    Seconds = running.Seconds + stopwatch.Elapsed.TotalSeconds,
                };
                cells.Add(solved ? $"{result.Statistics.NodesExpanded}" : "-");
            }

            string name = Path.GetRelativePath(benchmarkRoot, problemPath);
            Console.WriteLine($"{name,-46}{cells[0],14}{cells[1],14}{cells[2],14}");
        }

        Console.WriteLine();
        Console.WriteLine($"{"mode",-14}{"solved",10}{"attempted",12}{"expanded",14}{"seconds",12}");
        foreach ((string label, Outcome outcome) in totals)
        {
            Console.WriteLine(
                $"{label,-14}{outcome.Solved,10}{outcome.Attempted,12}{outcome.Expanded,14}{outcome.Seconds,12:F1}");
        }
    }

    private static IEnumerable<(string DomainPath, string ProblemPath)> Discover(string root, int perDomain)
    {
        foreach (string category in new[] { "strips", "adl" })
        {
            string categoryPath = Path.Combine(root, category);
            if (!Directory.Exists(categoryPath)) continue;

            foreach (string directory in Directory.EnumerateDirectories(categoryPath).OrderBy(p => p, StringComparer.Ordinal))
            {
                string domainPath = Path.Combine(directory, "domain.pddl");
                if (!File.Exists(domainPath)) continue;

                string[] problems = Directory
                    .EnumerateFiles(directory, "*.pddl", SearchOption.TopDirectoryOnly)
                    .Where(p => !string.Equals(p, domainPath, StringComparison.OrdinalIgnoreCase))
                    .Where(p => !IsDomain(p))
                    .OrderBy(p => p, StringComparer.Ordinal)
                    .Take(perDomain)
                    .ToArray();

                foreach (string problemPath in problems)
                {
                    string paired = Path.Combine(directory, $"{Path.GetFileNameWithoutExtension(problemPath)}-domain.pddl");
                    yield return (File.Exists(paired) ? paired : domainPath, problemPath);
                }
            }
        }
    }

    private static bool IsDomain(string path)
        => File.ReadLines(path)
            .Select(line => line.Trim())
            .Where(line => line.Length > 0 && !line.StartsWith(';'))
            .Take(3)
            .Any(line => line.Replace(" ", "").Contains("(define(domain", StringComparison.OrdinalIgnoreCase));
}
