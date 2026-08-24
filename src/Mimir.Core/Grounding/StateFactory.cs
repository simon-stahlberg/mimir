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
        return new State(context);
    }

    public State Create(InstanceContext context, IEnumerable<Fact<Fluent>> trueFacts)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(trueFacts);
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

        var bitboard = new ulong[wordCount];
        foreach (Fact<Fluent> fact in facts)
        {
            int arrayIndex = fact.LocalIndex / 64;
            int bitIndex = fact.LocalIndex % 64;
            bitboard[arrayIndex] |= 1UL << bitIndex;
        }

        return new State(context, bitboard);
    }
}
