using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;

namespace Mimir.Search.Heuristics;

public sealed class BlindHeuristic : IHeuristic
{
    public static BlindHeuristic Instance { get; } = new();

    private BlindHeuristic()
    {
    }

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null) => new(0);
}
