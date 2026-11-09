using Mimir.Core.Engines;
using Mimir.Core.Schemas;

namespace Mimir.FactoredParity;

/// <summary>
/// Counts ground fluents per instance. LAPKT sizes its novelty table as fluents^arity pointers and caps it at
/// 2048 MB, so above 640 fluents it cannot hold a width-3 table and quietly drops back to width 1.
/// </summary>
internal static class FluentCensus
{
    private const int LapktWidth3Limit = 640;

    public static void Run(string benchmarkRoot, string[] domains)
    {
        Console.WriteLine($"{"instance",-44}{"fluents",9}{"LAPKT IW(3)",14}");
        foreach (string name in domains)
        {
            string directory = Path.Combine(benchmarkRoot, name);
            string domainPath = Path.Combine(directory, "domain.pddl");
            if (!File.Exists(domainPath)) { Console.WriteLine($"{name,-44}  (no domain.pddl)"); continue; }

            Domain domain;
            try { domain = Domain.FromFile(domainPath); }
            catch (Exception e) when (e is PddlLoadException or Mimir.Pddl.Analysis.PddlValidationException)
            {
                Console.WriteLine($"{name,-44}  unreadable: {e.Message}");
                continue;
            }

            foreach (string problemPath in Directory.EnumerateFiles(directory, "*.pddl")
                         .Where(p => !Path.GetFileName(p).Contains("domain", StringComparison.OrdinalIgnoreCase))
                         .OrderBy(p => new FileInfo(p).Length))
            {
                string label = $"{name}/{Path.GetFileNameWithoutExtension(problemPath)}";
                try
                {
                    var problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Grounded);
                    int fluents = problem.InitialState.Context.Fluents.Count;
                    string verdict = fluents <= LapktWidth3Limit ? "fits" : "DOWNGRADES";
                    Console.WriteLine($"{label,-44}{fluents,9}{verdict,14}");
                }
                catch (Exception e) when (e is NotSupportedException or PddlLoadException
                                              or Mimir.Pddl.Analysis.PddlValidationException or OutOfMemoryException)
                {
                    Console.WriteLine($"{label,-44}{"-",9}{"ungroundable",14}");
                }

                Console.Out.Flush();
            }
        }
    }
}
