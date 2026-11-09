using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>
/// Times the novelty check alone over a fixed stream of successors, so the cost of moving from pairs to triples
/// can be read off without the rest of the search in the way.
/// </summary>
internal static class NoveltyCost
{
    public static void Run(string domainPath, string problemPath, int states)
    {
        var domain = Domain.FromFile(domainPath);
        Problem problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
        var generator = problem.GetApplicableActionGenerator(problem.InitialState);

        // A fixed stream, so every width sees exactly the same work.
        var stream = new List<(State Parent, State Child)>();
        var queue = new Queue<State>();
        var seen = new HashSet<State> { problem.InitialState };
        queue.Enqueue(problem.InitialState);
        while (queue.Count > 0 && stream.Count < states)
        {
            State parent = queue.Dequeue();
            ExtendedState extended = parent.Expand();
            foreach (GroundAction action in generator.GetApplicableActions(extended))
            {
                State child = extended.Apply(action);
                stream.Add((parent, child));
                if (seen.Add(child)) queue.Enqueue(child);
                if (stream.Count >= states) break;
            }
        }

        int atoms = stream.Count == 0 ? 0 : (int)stream.Average(x => x.Child.GetTrueFacts().Count());
        Console.WriteLine($"{Path.GetFileNameWithoutExtension(problemPath)}: {stream.Count} successors, "
            + $"{atoms} atoms true per state on average");
        Console.WriteLine($"{"width",-12}{"ms",10}{"us/state",11}{"novel",9}{"vs k=2",9}{"cover hit",9}{"MB",10}");

        double baseline = 0;
        foreach ((int k, bool cover) in new[] { (1, false), (2, false), (2, true), (3, false), (3, true) })
        {
            // Best of three: these runs are short enough that a single collection would otherwise dominate.
            double ms = double.MaxValue;
            int novel = 0;
            long hits = 0, misses = 0, bytes = 0;
            for (int attempt = 0; attempt < 3; attempt++)
            {
                var tracker = new NoveltyTracker(k, problem.InitialState.Context) { UseCoverFastPath = cover };
                tracker.IsNovel(problem.InitialState);
                GC.Collect();
                GC.WaitForPendingFinalizers();
                long before = GC.GetTotalMemory(true);
                var stopwatch = Stopwatch.StartNew();
                int count = 0;
                foreach ((State parent, State child) in stream)
                    if (tracker.IsNovel(child, parent)) count++;

                stopwatch.Stop();
                bytes = Math.Max(bytes, GC.GetTotalMemory(false) - before);
                ms = Math.Min(ms, stopwatch.Elapsed.TotalMilliseconds);
                novel = count;
                hits = tracker.CoverHits;
                misses = tracker.CoverMisses;
            }
            if (k == 2 && !cover) baseline = ms;
            string ratio = k == 3 && baseline > 0 ? $"{ms / baseline:F1}x" : "-";
            string hitRate = hits + misses > 0 ? $"{100.0 * hits / (hits + misses):F0}%" : "-";
            Console.WriteLine($"{(cover ? k + " cover" : k.ToString()),-12}{ms,10:F1}"
                + $"{ms * 1000 / stream.Count,11:F1}{novel,9}{ratio,9}{hitRate,9}{bytes / 1048576.0,10:F0}");
        }
    }
}
