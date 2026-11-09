using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Planning;

namespace Mimir.FactoredParity;

/// <summary>
/// Blind breadth-first throughput per generator. Breadth-first with no heuristic isolates the cost of producing
/// successors, which is the only thing the three planners can be compared on directly.
/// </summary>
internal static class Throughput
{
    public static void Run(
        string domainDirectory, string[] problems, int seconds, string generatorCodes = "glf", string spec = "bfs")
    {
        var domain = Domain.FromFile(Path.Combine(domainDirectory, "domain.pddl"));
        Console.WriteLine($"{"instance",-26}{"generator",-10}{"expanded",11}{"generated",13}{"seconds",9}{"exp/s",11}");

        foreach (string name in problems)
        {
            foreach (ApplicableActionGeneratorType type in generatorCodes.Select(code => code switch
                     {
                         'g' => ApplicableActionGeneratorType.Grounded,
                         'l' => ApplicableActionGeneratorType.Lifted,
                         'f' => ApplicableActionGeneratorType.Factored,
                         _ => throw new ArgumentException($"Unknown generator '{code}'.", nameof(generatorCodes)),
                     }))
            {
                Problem problem;
                try
                {
                    problem = Problem.FromFile(domain, Path.Combine(domainDirectory, name + ".pddl"), type);
                }
                catch (Exception e) when (e is NotSupportedException or PddlLoadException
                                              or Mimir.Pddl.Analysis.PddlValidationException)
                {
                    Console.WriteLine($"{name,-26}{type,-10}  unsupported");
                    continue;
                }

                using var cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(seconds));
                var stopwatch = Stopwatch.StartNew();
                PlanResult result = PlannerFactory.CreateFromSpec(spec)
                    .Solve(problem, cancellationToken: cancellation.Token);
                stopwatch.Stop();

                double elapsed = stopwatch.Elapsed.TotalSeconds;
                Console.WriteLine(string.Create(
                    System.Globalization.CultureInfo.InvariantCulture,
                    $"{name,-26}{type,-10}{result.Statistics.NodesExpanded,11}{result.Statistics.NodesGenerated,13}"
                    + $"{elapsed,9:F2}{result.Statistics.NodesExpanded / Math.Max(elapsed, 0.001),11:F0}"));
                Console.Out.Flush();
            }
        }
    }
}
