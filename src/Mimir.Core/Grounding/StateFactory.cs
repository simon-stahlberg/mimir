using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

public sealed class StateFactory
{
    public static StateFactory Default { get; } = new();

    private StateFactory()
    {
    }

    public State CreateEmpty(InstanceContext context)
    {
        ArgumentNullException.ThrowIfNull(context);
        return Create(context, Array.Empty<Fact<Fluent>>());
    }

    public State Create(InstanceContext context, IEnumerable<Fact<Fluent>> trueFacts,
        IReadOnlyDictionary<GroundFunctionCall, double>? numericValues = null)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(trueFacts);
        int numericCount = context.NumericLayout.Count;
        double[] values = numericCount == 0 ? Array.Empty<double>() : new double[numericCount];
        bool[] assigned = numericCount == 0 ? Array.Empty<bool>() : new bool[numericCount];
        if (numericValues is not null)
        {
            foreach ((GroundFunctionCall call, double value) in numericValues)
            {
                ArgumentNullException.ThrowIfNull(call);
                if (!ReferenceEquals(call.Context, context))
                    throw new ArgumentException("A numeric field belongs to a different problem.", nameof(numericValues));
                if (call.Field.Index is not NumericFluentIndex index)
                    throw new ArgumentException($"Static numeric field '{call.Function.Name}' cannot be overridden in a state.", nameof(numericValues));
                if (assigned[index.Value])
                    throw new ArgumentException($"Numeric field '{call.Function.Name}' was supplied more than once.", nameof(numericValues));
                assigned[index.Value] = true;
                values[index.Value] = value;
            }
        }
        if ((numericValues?.Count ?? 0) != numericCount)
            throw new ArgumentException($"State requires a complete valuation of {numericCount} numeric fields.", nameof(numericValues));

        Fact<Fluent>[] facts = trueFacts.ToArray();
        foreach (Fact<Fluent> fact in facts)
        {
            if (fact is null)
                throw new ArgumentException("True facts cannot contain null values.", nameof(trueFacts));
            if (!ReferenceEquals(fact.Context, context))
                throw new ArgumentException("A fact belongs to a different problem.", nameof(trueFacts));
        }

        int maxFactIndex = facts.Length == 0 ? -1 : facts.Max(fact => fact.LocalIndex);
        int wordCount = maxFactIndex >= 0 ? (maxFactIndex / 64) + 1 : 0;

        ulong[] bitboard = wordCount == 0 ? Array.Empty<ulong>() : new ulong[wordCount];
        foreach (Fact<Fluent> fact in facts)
        {
            int arrayIndex = fact.LocalIndex / 64;
            int bitIndex = fact.LocalIndex % 64;
            bitboard[arrayIndex] |= 1UL << bitIndex;
        }

        return new State(context, bitboard, values, takeOwnership: true);
    }
}
