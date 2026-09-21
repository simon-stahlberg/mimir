using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class SchemaCondition
{
    public IReadOnlyList<Literal> Literals { get; }
    public IReadOnlyList<NumericComparison> Comparisons => Array.Empty<NumericComparison>();

    internal SchemaCondition(IEnumerable<Literal> literals)
        => Literals = Array.AsReadOnly(literals.ToArray());
}

public sealed class ActionEffect
{
    public IReadOnlyList<Literal<Atom<Fluent>>> Literals { get; }
    public IReadOnlyList<NumericUpdate> NumericUpdates => Array.Empty<NumericUpdate>();

    internal ActionEffect(IEnumerable<Literal<Atom<Fluent>>> literals)
        => Literals = Array.AsReadOnly(literals.ToArray());
}

public sealed class GroundConjunctiveCondition
{
    public Problem Problem { get; }
    public IReadOnlyList<Literal<Fact>> Literals { get; }
    public IReadOnlyList<GroundNumericComparison> Comparisons => Array.Empty<GroundNumericComparison>();

    internal GroundConjunctiveCondition(Problem problem, IEnumerable<Literal<Fact>> literals)
    {
        Problem = problem;
        Literals = Array.AsReadOnly(literals.ToArray());
    }
}

public sealed class GroundActionEffect
{
    public IReadOnlyList<Literal<Fact>> Literals { get; }
    public IReadOnlyList<GroundNumericUpdate> NumericUpdates => Array.Empty<GroundNumericUpdate>();

    internal GroundActionEffect(IEnumerable<Literal<Fact>> literals)
        => Literals = Array.AsReadOnly(literals.ToArray());
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
