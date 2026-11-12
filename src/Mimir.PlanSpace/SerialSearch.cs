using System.Diagnostics;
using System.Text;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

// A* over single edits from the empty plan, with the number of edits as the cost so far and the number of
// errors as the estimate of the edits to go.
public static class SerialSearch
{
    public static SerialPlan? Solve(Problem problem, int maxExpansions, TimeSpan timeout)
    {
        ArgumentNullException.ThrowIfNull(problem);
        var watch = Stopwatch.StartNew();
        var open = new PriorityQueue<SerialPlan, (int Cost, int Errors, int Order)>();
        var closed = new HashSet<string>();
        SerialPlan root = SerialPlan.Empty(problem);
        closed.Add(Key(root));
        open.Enqueue(root, (0, 0, 0));
        int order = 0;
        int fewestUnmetGoals = int.MaxValue;
        for (int expansions = 0; expansions < maxExpansions && open.Count > 0 && watch.Elapsed < timeout; expansions++)
        {
            SerialPlan plan = open.Dequeue();
            SerialDiagnosis diagnosis = plan.Diagnose();
            if (diagnosis.IsSolution) return plan;

            // A plan whose only errors are unmet goals is executable. Committing to the first one that leaves
            // fewer goals unmet than any before keeps the search from weighing every way of reaching it
            // against every way of continuing, which grows exponentially with the number of goals.
            if (diagnosis.Errors.Count < fewestUnmetGoals && diagnosis.Errors.All(error => error.Kind == SerialErrorKind.UnmetGoal))
            {
                fewestUnmetGoals = diagnosis.Errors.Count;
                open.Clear();
            }

            int errorIndex = CommittedError(diagnosis);
            IReadOnlyList<SerialRepair> repairs = diagnosis.Errors[errorIndex].Repairs;
            for (int repairIndex = 1; repairIndex < repairs.Count; repairIndex++)
            {
                // A rebind changes the preconditions of a step after steps were inserted for them, which leaves
                // those steps with a purpose the plan no longer has and SerialOracle unable to follow it.
                if (repairs[repairIndex].Kind == SerialRepairKind.Rebind) continue;

                SerialPlan child = plan.Clone();
                var choice = new int[diagnosis.Errors.Count];
                choice[errorIndex] = repairIndex;
                child.Apply(child.Diagnose(), choice, []);
                if (!closed.Add(Key(child))) continue;

                // Among equally promising plans the one with the fewest errors, and then the newest, goes first.
                int errors = child.Diagnose().Errors.Count;
                open.Enqueue(child, (child.Round + errors, errors, --order));
            }
        }

        return null;
    }

    // Repairing one error per plan keeps the branching to the repairs of that error. Unbound parameters come
    // first because they hide the errors of their step, and goals last so that a step is appended only to a
    // plan that is otherwise executable.
    private static int CommittedError(SerialDiagnosis diagnosis)
    {
        foreach (SerialErrorKind kind in new[] { SerialErrorKind.UnboundParameter, SerialErrorKind.UnsatisfiedPrecondition, SerialErrorKind.UnmetGoal })
        {
            for (int i = 0; i < diagnosis.Errors.Count; i++)
            {
                if (diagnosis.Errors[i].Kind == kind) return i;
            }
        }

        throw new InvalidOperationException("A plan without errors has no error to repair.");
    }

    private static string Key(SerialPlan plan)
    {
        var key = new StringBuilder();
        foreach (SerialStep step in plan.Steps)
        {
            key.Append(step.Schema.Name);
            foreach (Constant? constant in step.Binding) key.Append(' ').Append(constant?.Name ?? "?");
            key.Append(';');
        }

        return key.ToString();
    }
}
