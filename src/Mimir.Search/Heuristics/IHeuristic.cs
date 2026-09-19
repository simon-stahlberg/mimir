using Mimir.Core.Engines;
using Mimir.Core.Grounding;
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
