using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>
/// Times the two novelty formulations over the same state sequence, so the per-successor cost can be compared
/// without the rest of a search run drowning it out.
/// </summary>
internal static class Novelty
{
    public static void Run(string domainPath, string problemPath, int width, int stateLimit)
    {
        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Lifted);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);

        var incremental = new NoveltyTracker(width, problem.InitialState.Context);
        var full = new NoveltyTracker(width, problem.InitialState.Context);
        incremental.IsNovel(problem.InitialState);
        full.IsNovel(problem.InitialState);

        var incrementalTime = new Stopwatch();
        var fullTime = new Stopwatch();
        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        int compared = 0;
        int disagreements = 0;
        int facts = 0;
        while (open.Count > 0 && compared < stateLimit)
        {
            State parent = open.Dequeue();
            ExtendedState extended = parent.Expand();
            foreach (GroundAction action in generator.GetApplicableActions(extended))
            {
                if (compared >= stateLimit) break;
                State successor = extended.Apply(action);
                if (!closed.Add(successor)) continue;

                incrementalTime.Start();
                bool incrementalVerdict = incremental.IsNovel(successor, parent, action);
                incrementalTime.Stop();

                fullTime.Start();
                bool fullVerdict = full.IsNovel(successor);
                fullTime.Stop();

                if (incrementalVerdict != fullVerdict) disagreements++;
                compared++;
                facts = successor.GetTrueFacts().Count();
                if (fullVerdict) open.Enqueue(successor);
            }
        }

        Console.WriteLine($"{Path.GetFileName(problemPath),-34} k={width}  states={compared,6}  atoms/state={facts,6}");
        Console.WriteLine($"  full        {fullTime.Elapsed.TotalMilliseconds,10:F1} ms");
        Console.WriteLine($"  incremental {incrementalTime.Elapsed.TotalMilliseconds,10:F1} ms"
            + $"   ({fullTime.Elapsed.TotalMilliseconds / Math.Max(incrementalTime.Elapsed.TotalMilliseconds, 0.001):F1}x)");
        Console.WriteLine($"  disagreements {disagreements}");
    }
}
