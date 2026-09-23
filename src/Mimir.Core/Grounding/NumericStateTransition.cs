using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

// PDDL 2.1 makes an action inapplicable when an update value is undefined, when a relative update targets an
// undefined fluent, or when two effects write the same fluent incompatibly (only increase/decrease combine).
internal static class NumericStateTransition
{
    internal static bool IsDefined(State source, IReadOnlyList<GroundNumericUpdate> unconditional,
        IReadOnlyList<GroundConditionalNumericEffect>? triggered)
        => TryCollectWrites(source, unconditional, triggered, out _);

    internal static double[]? Apply(State source, IReadOnlyList<GroundNumericUpdate> unconditional,
        IReadOnlyList<GroundConditionalNumericEffect>? triggered)
    {
        if (!TryCollectWrites(source, unconditional, triggered, out List<NumericWrite>? writes))
            throw new InvalidOperationException("The action is not applicable: its numeric effects are undefined in this state.");
        if (writes is null) return null;
        double[] values = source.NumericValues.ToArray();
        foreach (NumericWrite write in writes)
        {
            values[write.Index] = write.Additive
                ? NumericEvaluation.Apply(NumericOperator.Add, values[write.Index], write.Value)
                : write.Value;
        }
        return values;
    }

    private static bool TryCollectWrites(State source, IReadOnlyList<GroundNumericUpdate> unconditional,
        IReadOnlyList<GroundConditionalNumericEffect>? triggered, out List<NumericWrite>? writes)
    {
        writes = null;
        for (int i = 0; i < unconditional.Count; i++)
        {
            if (!TryCollect(source, unconditional[i], ref writes)) return false;
        }
        if (triggered is null) return true;
        for (int i = 0; i < triggered.Count; i++)
        {
            if (!TryCollect(source, triggered[i].Effect, ref writes)) return false;
        }
        return true;
    }

    private static bool TryCollect(State source, GroundNumericUpdate update, ref List<NumericWrite>? writes)
    {
        if (!ReferenceEquals(update.Target.Context, source.Context))
            throw new ArgumentException("Numeric update belongs to a different problem.");
        // A target without a state slot was never initialized and is not assignable, so it is undefined.
        if (update.Target.Field.Index is not NumericFluentIndex target) return false;
        int index = target.Value;
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
        writes ??= new List<NumericWrite>(2);
        for (int i = 0; i < writes.Count; i++)
        {
            NumericWrite previous = writes[i];
            if (previous.Index != index) continue;
            if (!additive || !previous.Additive) return false;
            writes[i] = previous with { Value = previous.Value + value };
            return true;
        }
        writes.Add(new NumericWrite(index, additive, value));
        return true;
    }

    private readonly record struct NumericWrite(int Index, bool Additive, double Value);
}
