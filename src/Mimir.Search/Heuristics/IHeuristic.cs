using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

public readonly record struct HeuristicEvaluation(double Value, Func<Action, bool>? IsPreferredAction = null);

public interface IHeuristic
{
    HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null);
}

internal interface IGroundedRpgHeuristic
{
    GroundedApplicableActionGenerator ActionGenerator { get; }
}
