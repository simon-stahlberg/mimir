using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class SchemaCondition
{
    public IReadOnlyList<Literal> Literals { get; }
    public IReadOnlyList<NumericComparison> Comparisons { get; }

    internal SchemaCondition(IEnumerable<Literal> literals, IEnumerable<NumericComparison> comparisons)
    {
        Literals = Array.AsReadOnly(literals.ToArray());
        Comparisons = Array.AsReadOnly(comparisons.ToArray());
    }
}

public sealed class ActionEffect
{
    public IReadOnlyList<Literal<Atom<Fluent>>> Literals { get; }
    public IReadOnlyList<NumericUpdate> NumericUpdates { get; }

    internal ActionEffect(IEnumerable<Literal<Atom<Fluent>>> literals, IEnumerable<NumericUpdate> updates)
    {
        Literals = Array.AsReadOnly(literals.ToArray());
        NumericUpdates = Array.AsReadOnly(updates.ToArray());
    }
}

public sealed class GroundConjunctiveCondition
{
    public Problem Problem { get; }
    public IReadOnlyList<Literal<Fact>> Literals { get; }
    public IReadOnlyList<GroundNumericComparison> Comparisons { get; }

    internal GroundConjunctiveCondition(Problem problem, IEnumerable<Literal<Fact>> literals, IEnumerable<GroundNumericComparison> comparisons)
    {
        Problem = problem;
        Comparisons = Array.AsReadOnly(comparisons.ToArray());
        Literals = Array.AsReadOnly(literals.ToArray());
    }
}

public sealed class GroundActionEffect
{
    public IReadOnlyList<Literal<Fact>> Literals { get; }
    public IReadOnlyList<GroundNumericUpdate> NumericUpdates { get; }

    internal GroundActionEffect(IEnumerable<Literal<Fact>> literals, IEnumerable<GroundNumericUpdate> updates)
    {
        Literals = Array.AsReadOnly(literals.ToArray());
        NumericUpdates = Array.AsReadOnly(updates.ToArray());
    }
}

internal static class GroundConditionLiterals
{
    internal static IEnumerable<Literal<Fact>> Read(InstanceContext context,
        OffsetBitboard positiveFluent, OffsetBitboard negativeFluent,
        OffsetBitboard positiveStatic, OffsetBitboard negativeStatic,
        IReadOnlyList<Literal<Fact<Derived>>> derived)
    {
        foreach (int index in BitboardOps.EnumerateSetBits(positiveFluent))
            yield return new(context.GetFact(new FluentIndex(index)), Polarity.Positive);
        foreach (int index in BitboardOps.EnumerateSetBits(negativeFluent))
            yield return new(context.GetFact(new FluentIndex(index)), Polarity.Negative);
        foreach (int index in BitboardOps.EnumerateSetBits(positiveStatic))
            yield return new(context.GetFact(new StaticIndex(index)), Polarity.Positive);
        foreach (int index in BitboardOps.EnumerateSetBits(negativeStatic))
            yield return new(context.GetFact(new StaticIndex(index)), Polarity.Negative);
        foreach (Literal<Fact<Derived>> literal in derived)
            yield return new(literal.Value, literal.Polarity);
    }
}
