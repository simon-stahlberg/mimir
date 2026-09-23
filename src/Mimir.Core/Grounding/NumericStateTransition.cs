using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

// PDDL 2.1 makes an action inapplicable when an update value is undefined, when a relative update targets an
// undefined fluent, or when two effects write the same fluent incompatibly (only increase/decrease combine).
internal static class NumericStateTransition
{
    // Actions rarely have more numeric effects than this, so their writes fit on the stack.
    private const int StackWriteLimit = 16;

    internal static bool IsDefined(ExtendedState state, Action action)
    {
        int capacity = action.NumericEffects.Count + action.ConditionalNumericEffects.Count;
        if (capacity == 0) return true;
        Span<NumericWrite> writes = capacity <= StackWriteLimit ? stackalloc NumericWrite[capacity] : new NumericWrite[capacity];
        return TryCollectWrites(state, action, writes, out _);
    }

    // values is null when the action writes no numeric fluent in this state.
    internal static bool TryApply(ExtendedState state, Action action, out double[]? values)
    {
        values = null;
        int capacity = action.NumericEffects.Count + action.ConditionalNumericEffects.Count;
        if (capacity == 0) return true;
        Span<NumericWrite> writes = capacity <= StackWriteLimit ? stackalloc NumericWrite[capacity] : new NumericWrite[capacity];
        if (!TryCollectWrites(state, action, writes, out int count)) return false;
        if (count == 0) return true;

        values = state.State.NumericValues.ToArray();
        foreach (NumericWrite write in writes[..count])
        {
            values[write.Index] = write.Additive
                ? NumericEvaluation.Apply(NumericOperator.Add, values[write.Index], write.Value)
                : write.Value;
        }
        return true;
    }

    private static bool TryCollectWrites(ExtendedState state, Action action, Span<NumericWrite> writes, out int count)
    {
        count = 0;
        State source = state.State;
        IReadOnlyList<GroundNumericUpdate> unconditional = action.NumericEffects;
        for (int i = 0; i < unconditional.Count; i++)
        {
            if (!TryCollect(source, unconditional[i], writes, ref count)) return false;
        }

        IReadOnlyList<GroundConditionalNumericEffect> conditional = action.ConditionalNumericEffects;
        for (int i = 0; i < conditional.Count; i++)
        {
            if (conditional[i].IsSatisfied(state) && !TryCollect(source, conditional[i].Effect, writes, ref count)) return false;
        }
        return true;
    }

    private static bool TryCollect(State source, GroundNumericUpdate update, Span<NumericWrite> writes, ref int count)
    {
        if (!ReferenceEquals(update.Target.Context, source.Context))
            throw new ArgumentException("Numeric update belongs to a different problem.");
        // A target without a state slot was never initialized and is not assignable, so it is undefined.
        if (update.Target.StateIndex is not int index) return false;
        double before = source.NumericValues[index];
        double right = source.Value(update.Expression);
        if (double.IsNaN(right)) return false;
        if (update.Operator != NumericUpdateOperator.Assign && double.IsNaN(before)) return false;
        double value = update.Operator switch
        {
            NumericUpdateOperator.Assign => right,
            NumericUpdateOperator.Increase => right,
            NumericUpdateOperator.Decrease => -right,
            NumericUpdateOperator.ScaleUp => NumericEvaluation.Apply(NumericOperator.Multiply, before, right),
            NumericUpdateOperator.ScaleDown => NumericEvaluation.Apply(NumericOperator.Divide, before, right),
            _ => throw new InvalidOperationException("Unknown numeric update operator.")
        };
        if (double.IsNaN(value)) return false;

        bool additive = update.Operator is NumericUpdateOperator.Increase or NumericUpdateOperator.Decrease;
        for (int i = 0; i < count; i++)
        {
            NumericWrite previous = writes[i];
            if (previous.Index != index) continue;
            if (!additive || !previous.Additive) return false;
            writes[i] = previous with { Value = previous.Value + value };
            return true;
        }
        writes[count++] = new NumericWrite(index, additive, value);
        return true;
    }

    private readonly record struct NumericWrite(int Index, bool Additive, double Value);
}
