using Mimir.Core.Grounding;
using Mimir.Search.Evaluation;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;
using Expansions = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.ExtendedState State, System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)> Successors)>;

namespace Mimir.Search.Heuristics;

public readonly record struct QHeuristicEvaluation(IReadOnlyList<double> Values);

public interface IQHeuristic
{
    QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null);

    IReadOnlyList<QHeuristicEvaluation> Evaluate(Expansions expansions, GoalCondition? goal = null)
    {
        var evaluations = new QHeuristicEvaluation[expansions.Count];
        for (int index = 0; index < expansions.Count; index++)
        {
            var (state, successors) = expansions[index];
            evaluations[index] = Evaluate(state, successors, goal);
        }
        return evaluations;
    }
}
