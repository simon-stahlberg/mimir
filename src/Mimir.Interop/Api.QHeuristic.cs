using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Successors = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)>;
using Expansions = System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.ExtendedState State, System.Collections.Generic.IReadOnlyList<(Mimir.Core.Grounding.Action Action, Mimir.Core.Grounding.ExtendedState State)> Successors)>;

namespace Mimir.Interop;

public static partial class Exports
{
    private sealed class CallbackQHeuristic(Problem problem, IntPtr evaluate) : IQHeuristic
    {
        private readonly IntPtr _evaluate = evaluate != IntPtr.Zero ? evaluate
            : throw new ArgumentNullException(nameof(evaluate));

        public QHeuristicEvaluation Evaluate(ExtendedState state, Successors successors, GoalCondition? goal = null)
            => Evaluate(new (ExtendedState, Successors)[] { (state, successors) }, goal)[0];

        public unsafe IReadOnlyList<QHeuristicEvaluation> Evaluate(Expansions expansions, GoalCondition? goal = null)
        {
            if (goal != null && !ReferenceEquals(goal.Problem, problem))
                throw new ArgumentException("Goal belongs to a different problem.", nameof(goal));
            var offsets = new int[expansions.Count + 1];
            for (int row = 0; row < expansions.Count; row++)
            {
                if (!ReferenceEquals(expansions[row].State.State.Context.Problem, problem))
                    throw new ArgumentException("State belongs to a different problem.", nameof(expansions));
                offsets[row + 1] = checked(offsets[row] + expansions[row].Successors.Count);
            }
            var values = new double[offsets[^1]];
            var handles = new int[checked(expansions.Count + 2 * values.Length)];
            try
            {
                for (int row = 0; row < expansions.Count; row++)
                {
                    handles[row] = StoreExtendedState(expansions[row].State);
                    Successors successors = expansions[row].Successors;
                    for (int index = 0; index < successors.Count; index++)
                    {
                        int position = expansions.Count + 2 * (offsets[row] + index);
                        handles[position] = ObjectRegistry.Store(successors[index].Action);
                        handles[position + 1] = StoreExtendedState(successors[index].State);
                    }
                }
                fixed (int* handlePointer = handles)
                fixed (int* offsetPointer = offsets)
                fixed (double* valuePointer = values)
                {
                    if (((delegate* unmanaged<int*, int*, int, double*, byte>)_evaluate)(
                        handlePointer, offsetPointer, expansions.Count, valuePointer) == 0)
                        throw new CallbackAbortedException();
                }
                var result = new QHeuristicEvaluation[expansions.Count];
                for (int row = 0; row < result.Length; row++)
                    result[row] = new QHeuristicEvaluation(new ArraySegment<double>(values, offsets[row], offsets[row + 1] - offsets[row]));
                return result;
            }
            finally
            {
                // Python zeros each slot as its wrapper adopts the handle.
                foreach (int handle in handles)
                    if (handle != 0) ObjectRegistry.Release(handle);
            }
        }
    }

}
