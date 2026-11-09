using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Evaluation;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>
/// Drives serialized rounds until one fails, then runs that round again against a reference width-2 search whose
/// novelty is recomputed from scratch. A round the reference finishes and the real one does not is over-pruning;
/// one that defeats both is iterated width behaving as defined.
/// </summary>
internal static class IwDiagnostic
{
    public static void Run(string domainPath, string problemPath, int maximumWidth, string generatorCode, int cap)
    {
        var domain = Domain.FromFile(domainPath);
        ApplicableActionGeneratorType type = generatorCode switch
        {
            "g" => ApplicableActionGeneratorType.Grounded,
            "l" => ApplicableActionGeneratorType.Lifted,
            _ => ApplicableActionGeneratorType.Factored,
        };
        Problem problem = Problem.FromFile(domain, problemPath, type);
        var goal = GoalCondition.FromProblem(problem);
        var detector = new FfDeadEndDetector(problem);

        State current = problem.InitialState;
        int round = 0;
        while (!goal.IsSatisfied(current.Expand()))
        {
            int remaining = goal.CountUnsatisfiedGoals(current.Expand());
            bool Target(ExtendedState s) =>
                goal.CountUnsatisfiedGoals(s) < remaining && !detector.IsDeadEnd(s, goal);

            SearchResult? success = null;
            int usedWidth = -1;
            for (int width = 0; width <= maximumWidth && success is null; width++)
            {
                SearchResult result = new IwSearch(width, current, goal, null, Target).Search();
                if (result.IsSuccess) { success = result; usedWidth = width; }
            }

            if (success is null)
            {
                Console.WriteLine($"round {round + 1} failed with {remaining} goals left; comparing against reference");
                Compare(problem, goal, current, Target, maximumWidth, cap);
                return;
            }

            round++;
            Console.WriteLine($"round {round,2}: width {usedWidth}, {remaining} -> {remaining - 1} goals left");
            foreach (GroundAction action in success.Plan) current = current.Expand().Apply(action);
        }

        Console.WriteLine($"solved in {round} rounds");
    }

    private static void Compare(
        Problem problem, GoalCondition goal, State root, Func<ExtendedState, bool> target, int width, int cap)
    {
        SearchResult ours = new IwSearch(width, root, goal, null, target).Search();
        Console.WriteLine($"  ours      IW({width}): success={ours.IsSuccess} expanded={ours.Statistics.NodesExpanded}");

        (bool found, int expanded, int generated) = Reference(problem, root, target, width, cap);
        Console.WriteLine($"  reference IW({width}): success={found} expanded={expanded} generated={generated}");
        Console.WriteLine(found && !ours.IsSuccess
            ? "  VERDICT: the real search prunes states the definition keeps -- over-pruning"
            : "  VERDICT: both agree; iterated width genuinely cannot finish this round");
    }

    // Novelty recomputed from the whole tuple set each time, with no reuse between a state and its parent.
    private static (bool Found, int Expanded, int Generated) Reference(
        Problem problem, State root, Func<ExtendedState, bool> target, int width, int cap)
    {
        var ids = new Dictionary<Fact<Fluent>, int>();
        var singles = new HashSet<int>();
        var pairs = new HashSet<long>();
        var queue = new Queue<State>();
        var generator = problem.GetApplicableActionGenerator(root);
        int expanded = 0, generated = 1;

        int[] Atoms(State state)
        {
            var atoms = new List<int>();
            foreach (Fact<Fluent> fact in state.GetTrueFacts())
            {
                if (!ids.TryGetValue(fact, out int id)) ids[fact] = id = ids.Count;
                atoms.Add(id);
            }

            atoms.Sort();
            return atoms.ToArray();
        }

        bool Admit(State state)
        {
            int[] atoms = Atoms(state);
            bool novel = false;
            foreach (int a in atoms)
                if (singles.Add(a)) novel = true;

            if (width >= 2)
            {
                for (int i = 0; i < atoms.Length; i++)
                    for (int j = i + 1; j < atoms.Length; j++)
                        if (pairs.Add(((long)atoms[i] << 32) | (uint)atoms[j])) novel = true;
            }

            return novel;
        }

        Admit(root);
        queue.Enqueue(root);
        while (queue.Count > 0 && expanded < cap)
        {
            State state = queue.Dequeue();
            ExtendedState extended = state.Expand();
            if (expanded > 0 && target(extended)) return (true, expanded, generated);

            expanded++;
            if (expanded % 2000 == 0)
                Console.Error.WriteLine($"    reference expanded={expanded} atoms={ids.Count} pairs={pairs.Count}");
            foreach (GroundAction action in generator.GetApplicableActions(extended))
            {
                State next = extended.Apply(action);
                generated++;
                // The target is checked ahead of novelty, exactly as iterated width is defined.
                if (target(next.Expand())) return (true, expanded, generated);
                if (Admit(next)) queue.Enqueue(next);
            }
        }

        return (false, expanded, generated);
    }
}
