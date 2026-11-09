using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>Checks the factored relaxed graph against the binding-enumerating one, verdict by verdict.</summary>
internal static class FfCompare
{
    // Warm up, then best of three: a single collection would otherwise swamp runs this short.
    private static double Time(List<ExtendedState> sample, GoalCondition goal, FfDeadEndDetector detector)
    {
        foreach (ExtendedState state in sample) detector.IsDeadEnd(state, goal);

        double best = double.MaxValue;
        for (int attempt = 0; attempt < 3; attempt++)
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
            var stopwatch = Stopwatch.StartNew();
            foreach (ExtendedState state in sample) detector.IsDeadEnd(state, goal);
            stopwatch.Stop();
            best = Math.Min(best, stopwatch.Elapsed.TotalMilliseconds);
        }

        return best;
    }

    public static void Run(string domainDirectory, string[] problems, int states)
    {
        var domain = Domain.FromFile(Path.Combine(domainDirectory, "domain.pddl"));
        Console.WriteLine($"{"instance",-26}{"states",8}{"disagree",10}{"lifted ms",11}{"factored ms",13}{"speedup",9}");

        foreach (string name in problems)
        {
            Problem problem = Problem.FromFile(
                domain, Path.Combine(domainDirectory, name + ".pddl"), ApplicableActionGeneratorType.Factored);
            var goal = GoalCondition.FromProblem(problem);
            var factored = new FfDeadEndDetector(problem, factored: true);
            var lifted = new FfDeadEndDetector(problem, factored: false);
            if (!factored.UsesFactoredGraph) { Console.WriteLine($"{name,-26}  declined by the factored graph"); continue; }

            // A breadth-first sample, so both detectors see the same states.
            var sample = new List<ExtendedState>();
            var queue = new Queue<State>();
            var seen = new HashSet<State> { problem.InitialState };
            queue.Enqueue(problem.InitialState);
            var generator = problem.GetApplicableActionGenerator(problem.InitialState);
            while (queue.Count > 0 && sample.Count < states)
            {
                ExtendedState current = queue.Dequeue().Expand();
                sample.Add(current);
                foreach (GroundAction action in generator.GetApplicableActions(current).Take(6))
                {
                    State next = current.Apply(action);
                    if (seen.Add(next)) queue.Enqueue(next);
                }
            }

            int disagree = 0;
            foreach (ExtendedState state in sample)
                if (factored.IsDeadEnd(state, goal) != lifted.IsDeadEnd(state, goal)) disagree++;

            double liftedMs = Time(sample, goal, lifted);
            double factoredMs = Time(sample, goal, factored);

            Console.WriteLine($"{name,-26}{sample.Count,8}{disagree,10}{liftedMs,11:F1}{factoredMs,13:F1}"
                + $"{liftedMs / Math.Max(factoredMs, 0.001),9:F1}x");
            Console.Out.Flush();
        }
    }
}
