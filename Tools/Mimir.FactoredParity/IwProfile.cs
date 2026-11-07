using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>
/// Splits an IW(1) expansion into its phases. A factored path can only skip work on successors that turn out to
/// be non-novel, so the share of those bounds whatever it could save.
/// </summary>
internal static class IwProfile
{
    public static void Run(string domainPath, string problemPath, int stateLimit, int width)
    {
        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
        var clique = new CliqueApplicableActionGenerator(problem);
        var factored = new FactoredApplicableActionGenerator(problem);
        GoalCondition goal = GoalCondition.FromProblem(problem);
        var tracker = new NoveltyTracker(width, problem.InitialState.Context);
        tracker.IsNovel(problem.InitialState);

        var generate = new Stopwatch();
        var groups = new Stopwatch();
        var apply = new Stopwatch();
        var closedTime = new Stopwatch();
        var goalTime = new Stopwatch();
        var novelty = new Stopwatch();

        var open = new Queue<State>();
        var closed = new HashSet<State> { problem.InitialState };
        open.Enqueue(problem.InitialState);

        long applicable = 0;
        long duplicates = 0;
        long novel = 0;
        long groupCount = 0;
        int expanded = 0;
        var actions = new List<GroundAction>();

        while (open.Count > 0 && expanded < stateLimit)
        {
            State state = open.Dequeue();
            ExtendedState extended = state.Expand();
            expanded++;

            groups.Start();
            foreach (FactoredAction group in factored.GetFactoredApplicableActions(extended)) groupCount++;
            groups.Stop();

            generate.Start();
            clique.CollectApplicableActions(extended, actions);
            generate.Stop();
            applicable += actions.Count;

            foreach (GroundAction action in actions)
            {
                apply.Start();
                State successor = extended.Apply(action);
                apply.Stop();

                closedTime.Start();
                bool fresh = closed.Add(successor);
                closedTime.Stop();
                if (!fresh) { duplicates++; continue; }

                goalTime.Start();
                bool isGoal = goal.IsSatisfied(successor.Expand());
                goalTime.Stop();

                novelty.Start();
                bool isNovel = tracker.IsNovel(successor, state);
                novelty.Stop();

                if (isGoal || isNovel) { novel++; open.Enqueue(successor); }
            }
        }

        double total = generate.Elapsed.TotalMilliseconds + apply.Elapsed.TotalMilliseconds
            + closedTime.Elapsed.TotalMilliseconds + goalTime.Elapsed.TotalMilliseconds
            + novelty.Elapsed.TotalMilliseconds;

        Console.WriteLine($"{Path.GetFileName(problemPath),-26} k={width} expanded={expanded}  applicable={applicable}  "
            + $"duplicates={duplicates}  kept={novel}  groups={groupCount}");
        Console.WriteLine($"  kept / applicable      {(applicable == 0 ? 0 : 100.0 * novel / applicable),8:F2} %");
        Console.WriteLine($"  generate (clique)      {generate.Elapsed.TotalMilliseconds,8:F1} ms");
        Console.WriteLine($"  generate (factored)    {groups.Elapsed.TotalMilliseconds,8:F1} ms");
        Console.WriteLine($"  apply                  {apply.Elapsed.TotalMilliseconds,8:F1} ms");
        Console.WriteLine($"  closed set             {closedTime.Elapsed.TotalMilliseconds,8:F1} ms");
        Console.WriteLine($"  goal test              {goalTime.Elapsed.TotalMilliseconds,8:F1} ms");
        Console.WriteLine($"  novelty                {novelty.Elapsed.TotalMilliseconds,8:F1} ms");
        Console.WriteLine($"  per-successor total    {total,8:F1} ms");
    }
}
