using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>
/// Times the lifted FF heuristic per state against the size of the ground action set, to see whether its cost
/// tracks the number of action instances -- which is what a factored relaxed graph would avoid enumerating.
/// </summary>
internal static class FfCost
{
    public static void Run(string domainDirectory, string[] problems, int calls)
    {
        var domain = Domain.FromFile(Path.Combine(domainDirectory, "domain.pddl"));
        Console.WriteLine($"{"instance",-28}{"groups",8}{"members",14}{"ff us/call",12}{"us per 1k members",19}");

        foreach (string name in problems)
        {
            string problemPath = Path.Combine(domainDirectory, name + ".pddl");
            Problem problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
            var goal = GoalCondition.FromProblem(problem);
            ExtendedState initial = problem.InitialState.Expand();

            var factored = new FactoredApplicableActionGenerator(problem);
            long groups = 0;
            double members = 0;
            foreach (FactoredAction group in factored.GetFactoredApplicableActions(initial))
            {
                groups++;
                members += group.MemberCountUpperBound;
            }

            var heuristic = new LiftedFfHeuristic(problem);
            heuristic.Evaluate(initial, goal);

            var states = new List<ExtendedState> { initial };
            foreach (GroundAction action in factored.GetApplicableActions(initial).Take(calls - 1))
                states.Add(initial.Apply(action).Expand());

            var stopwatch = Stopwatch.StartNew();
            foreach (ExtendedState state in states) heuristic.Evaluate(state, goal);
            stopwatch.Stop();

            double us = stopwatch.Elapsed.TotalMilliseconds * 1000 / states.Count;
            Console.WriteLine($"{name,-28}{groups,8}{members,14:N0}{us,12:F1}{us / Math.Max(members / 1000, 1),19:F2}");
            Console.Out.Flush();
        }
    }
}
