using System.Diagnostics;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Algorithms.IteratedWidth;
using Mimir.Search.Evaluation;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Planning;

/// <summary>
/// Serialized iterated width: reaches the goal by achieving one more of its atoms at a time.
/// <para>
/// Each round runs iterated width from where the last one stopped, looking not for the goal but for the first
/// state that leaves fewer of its atoms unachieved, widening from 0 until one is found. No ordering over the
/// goal is chosen and the goal itself never changes; progress is measured by the count alone, which is why the
/// loop runs at most once per goal atom.
/// </para>
/// <para>
/// A round may undo an atom achieved earlier, as long as it achieves more than it loses. Refusing that is a
/// known variant, not the published algorithm, and it is not clear it wins often enough to pay for the rounds
/// it rules out.
/// </para>
/// <para>
/// Serializing this way is a gamble, not a refinement: committing to the nearest state that achieves one more
/// goal atom can strand the search where no width reaches the rest. Blocksworld does exactly that, and plain
/// iterated width solves instances of it that this cannot.
/// </para>
/// </summary>
public class SerializedIteratedWidthPlanner : IPlanner
{
    private readonly int _maximumWidth;

    public SerializedIteratedWidthPlanner(int maximumWidth = 2)
    {
        if (maximumWidth is < 0 or > 3)
            throw new ArgumentOutOfRangeException(nameof(maximumWidth), "IW(k) supports k = 0..3.");

        _maximumWidth = maximumWidth;
    }

    /// <summary>Fires with the width each round settled on, once per round.</summary>
    public event System.Action<int>? RoundCompleted;

    public PlanResult Solve(
        Problem problem,
        State? startState = null,
        GoalCondition? goal = null,
        CancellationToken cancellationToken = default,
        IDeadEndDetector? deadEndDetector = null)
    {
        var setupStopwatch = Stopwatch.StartNew();
        PlannerInputs inputs = PlannerExecution.ResolveInputs(problem, startState, goal);
        setupStopwatch.Stop();
        if (cancellationToken.IsCancellationRequested)
            return PlannerExecution.Canceled(setupStopwatch.Elapsed);

        var searchStopwatch = Stopwatch.StartNew();
        var plan = new List<Action>();
        State current = inputs.StartState;
        int expanded = 0;
        int generated = 0;
        int rounds = 0;

        while (!inputs.Goal.IsSatisfied(current.Expand()))
        {
            if (cancellationToken.IsCancellationRequested)
                return Stop(SearchStatus.Canceled, setupStopwatch, searchStopwatch, expanded, generated, rounds);

            int remaining = inputs.Goal.CountUnsatisfiedGoals(current.Expand());
            SearchResult? round = RunRound(inputs, current, remaining, deadEndDetector, cancellationToken,
                ref expanded, ref generated);

            if (round is null)
                return Stop(SearchStatus.Failed, setupStopwatch, searchStopwatch, expanded, generated, rounds);
            if (round.IsCanceled)
                return Stop(SearchStatus.Canceled, setupStopwatch, searchStopwatch, expanded, generated, rounds);

            // A round that moves nowhere would leave the loop unable to make progress.
            if (round.Plan.Count == 0)
                return Stop(SearchStatus.Failed, setupStopwatch, searchStopwatch, expanded, generated, rounds);

            foreach (Action action in round.Plan)
            {
                plan.Add(action);
                current = current.Expand().Apply(action);
            }

            rounds++;
        }

        searchStopwatch.Stop();
        var statistics = new SearchStatistics(expanded, generated, searchStopwatch.Elapsed, rounds);
        return new PlanResult(
            SearchStatus.Succeeded, plan, statistics, setupStopwatch.Elapsed, searchStopwatch.Elapsed);
    }

    /// <summary>The first state with fewer goal atoms left, searched at the narrowest width that finds one.</summary>
    private SearchResult? RunRound(
        PlannerInputs inputs,
        State from,
        int remaining,
        IDeadEndDetector? deadEndDetector,
        CancellationToken cancellationToken,
        ref int expanded,
        ref int generated)
    {
        for (int width = 0; width <= _maximumWidth; width++)
        {
            // The detector gates which state a round may commit to, rather than pruning every state the round
            // generates: reaching one more goal atom is worthless if the atoms left over become unreachable,
            // and a test strong enough to notice that is far too costly to run on every successor.
            var search = new IwSearch(
                width,
                from,
                inputs.Goal,
                deadEndDetector: null,
                state => inputs.Goal.CountUnsatisfiedGoals(state) < remaining
                         && (deadEndDetector is null || !deadEndDetector.IsDeadEnd(state, inputs.Goal)));

            SearchResult result = search.Search(cancellationToken);
            expanded += result.Statistics.NodesExpanded;
            generated += result.Statistics.NodesGenerated;

            if (result.IsCanceled) return result;
            if (!result.IsSuccess) continue;

            RoundCompleted?.Invoke(width);
            return result;
        }

        return null;
    }

    private static PlanResult Stop(
        SearchStatus status,
        Stopwatch setup,
        Stopwatch search,
        int expanded,
        int generated,
        int rounds)
    {
        search.Stop();
        var statistics = new SearchStatistics(expanded, generated, search.Elapsed, rounds);
        return new PlanResult(status, [], statistics, setup.Elapsed, search.Elapsed);
    }
}
