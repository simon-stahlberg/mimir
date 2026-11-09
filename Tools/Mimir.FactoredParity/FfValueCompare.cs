using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>Compares factored FF against the binding-enumerating one, value by value and in time.</summary>
internal static class FfValueCompare
{
    public static void Run(string domainDirectory, string[] problems, int states)
    {
        var domain = Domain.FromFile(Path.Combine(domainDirectory, "domain.pddl"));
        Console.WriteLine($"{"instance",-26}{"states",7}{"differ",7}{"lifted ms",11}{"factored ms",13}{"speedup",9}");

        foreach (string name in problems)
        {
            Problem problem = Problem.FromFile(domain, Path.Combine(domainDirectory, name + ".pddl"));
            var goal = GoalCondition.FromProblem(problem);
            FactoredFfHeuristic? factored = FactoredFfHeuristic.TryCreate(problem, goal);
            if (factored is null) { Console.WriteLine($"{name,-26}  declined"); continue; }
            var lifted = new LiftedFfHeuristic(problem, goal);

            var sample = new List<ExtendedState>();
            var queue = new Queue<State>();
            var seen = new HashSet<State> { problem.InitialState };
            queue.Enqueue(problem.InitialState);
            var generator = problem.GetApplicableActionGenerator(problem.InitialState);
            while (queue.Count > 0 && sample.Count < states)
            {
                ExtendedState current = queue.Dequeue().Expand();
                sample.Add(current);
                foreach (GroundAction action in generator.GetApplicableActions(current).Take(5))
                {
                    State next = current.Apply(action);
                    if (seen.Add(next)) queue.Enqueue(next);
                }
            }

            int differ = 0;
            foreach (ExtendedState state in sample)
            {
                double a = lifted.Evaluate(state, goal).Value;
                double b = factored.Evaluate(state, goal).Value;
                if (Math.Abs(a - b) > 1e-9 && !(double.IsPositiveInfinity(a) && double.IsPositiveInfinity(b)))
                {
                    if (differ < 8) Console.WriteLine($"    lifted={a} factored={b}");
                    differ++;
                }
            }

            double liftedMs = Time(sample, goal, s => lifted.Evaluate(s, goal).Value);
            double factoredMs = Time(sample, goal, s => factored.Evaluate(s, goal).Value);
            Console.WriteLine(string.Create(System.Globalization.CultureInfo.InvariantCulture,
                $"{name,-26}{sample.Count,7}{differ,7}{liftedMs,11:F1}{factoredMs,13:F1}"
                + $"{liftedMs / Math.Max(factoredMs, 0.001),9:F1}x"));
            Console.Out.Flush();
        }
    }

    private static double Time(List<ExtendedState> sample, GoalCondition goal, Func<ExtendedState, double> evaluate)
    {
        foreach (ExtendedState state in sample) evaluate(state);
        double best = double.MaxValue;
        for (int attempt = 0; attempt < 3; attempt++)
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
            var stopwatch = Stopwatch.StartNew();
            foreach (ExtendedState state in sample) evaluate(state);
            stopwatch.Stop();
            best = Math.Min(best, stopwatch.Elapsed.TotalMilliseconds);
        }

        return best;
    }
}
