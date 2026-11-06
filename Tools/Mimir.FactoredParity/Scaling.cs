using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>
/// Measures both generators on the initial state of each instance in a directory, so that domains whose branching
/// factor explodes can be compared without expanding the products.
/// </summary>
internal static class Scaling
{
    public static void Run(string directory, TimeSpan budget)
    {
        string domainPath = Path.Combine(directory, "domain.pddl");
        var domain = Domain.FromFile(domainPath);

        Console.WriteLine($"{"instance",-24}{"groups",10}{"members",16}{"factored ms",14}{"clique ms",14}{"clique actions",16}");
        foreach (string problemPath in Directory
                     .EnumerateFiles(directory, "*.pddl")
                     .Where(path => !string.Equals(Path.GetFileName(path), "domain.pddl", StringComparison.OrdinalIgnoreCase))
                     .OrderBy(path => path, StringComparer.Ordinal))
        {
            var problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
            ExtendedState state = problem.InitialState.Expand();

            var factored = new FactoredApplicableActionGenerator(problem);
            var stopwatch = Stopwatch.StartNew();
            long groups = 0;
            double members = 0;
            foreach (FactoredAction group in factored.GetFactoredApplicableActions(state))
            {
                groups++;
                members += group.MemberCountUpperBound;
            }

            TimeSpan factoredTime = stopwatch.Elapsed;

            string cliqueActions = "skipped";
            string cliqueTime = "skipped";
            if (members <= 20_000_000)
            {
                var clique = new CliqueApplicableActionGenerator(problem);
                var actions = new List<GroundAction>();
                stopwatch.Restart();
                clique.CollectApplicableActions(state, actions);
                if (stopwatch.Elapsed <= budget)
                {
                    cliqueTime = $"{stopwatch.Elapsed.TotalMilliseconds:F1}";
                    cliqueActions = actions.Count.ToString();
                }
            }

            Console.WriteLine(
                $"{Path.GetFileNameWithoutExtension(problemPath),-24}{groups,10}{members,16:N0}"
                + $"{factoredTime.TotalMilliseconds,14:F1}{cliqueTime,14}{cliqueActions,16}");
        }
    }
}
