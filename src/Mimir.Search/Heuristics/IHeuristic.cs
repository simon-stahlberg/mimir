using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

public readonly record struct HeuristicEvaluation(double Value, Func<Action, bool>? IsPreferredAction = null);

public interface IHeuristic
{
    HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null);

    IReadOnlyList<HeuristicEvaluation> Evaluate(IReadOnlyList<ExtendedState> states, GoalCondition? goal = null)
    {
        var evaluations = new HeuristicEvaluation[states.Count];
        for (int index = 0; index < states.Count; index++)
            evaluations[index] = Evaluate(states[index], goal);
        return evaluations;
    }
}

internal interface IGroundedHeuristic
{
    GroundedApplicableActionGenerator ActionGenerator { get; }
}

internal static class HeuristicBinding
{
    internal static GroundedApplicableActionGenerator GetInitialGenerator(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        if (problem.GeneratorType != ApplicableActionGeneratorType.Grounded)
            throw new ArgumentException("A grounded heuristic requires a problem configured with grounded action generation.", nameof(problem));

        return (GroundedApplicableActionGenerator)problem.GetApplicableActionGenerator(problem.InitialState);
    }

    internal static IHeuristic Bind(
        IHeuristic heuristic,
        GoalCondition goal,
        IApplicableActionGenerator actionGenerator)
    {
        ArgumentNullException.ThrowIfNull(heuristic);
        ArgumentNullException.ThrowIfNull(goal);
        ArgumentNullException.ThrowIfNull(actionGenerator);

        if (heuristic is PerfectHeuristic perfect && !perfect.IsBoundTo(goal))
            return new PerfectHeuristic(actionGenerator.Problem, goal);

        if (heuristic is not IGroundedHeuristic groundedHeuristic)
            return heuristic;

        if (!ReferenceEquals(groundedHeuristic.ActionGenerator.Problem, actionGenerator.Problem))
        {
            throw new InvalidOperationException(
                "Heuristic and applicable-action generator belong to different problems.");
        }

        if (actionGenerator is not GroundedApplicableActionGenerator groundedGenerator)
        {
            throw new InvalidOperationException(
                "A grounded heuristic requires a grounded applicable-action generator.");
        }

        if (ReferenceEquals(groundedHeuristic.ActionGenerator, groundedGenerator))
            return heuristic;

        return heuristic switch
        {
            AddHeuristic => new AddHeuristic(groundedGenerator, goal),
            MaxHeuristic => new MaxHeuristic(groundedGenerator, goal),
            FFHeuristic => new FFHeuristic(groundedGenerator, goal),
            SetAddHeuristic => new SetAddHeuristic(groundedGenerator, goal),
            H2Heuristic => new H2Heuristic(groundedGenerator, goal),
            _ => throw new InvalidOperationException(
                $"Unsupported grounded heuristic type '{heuristic.GetType().Name}'.")
        };
    }

}
