using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class SchemaCondition
{
    public IReadOnlyList<Literal> Literals { get; }
    public IReadOnlyList<NumericComparison> NumericConditions { get; }

    internal SchemaCondition(IEnumerable<Literal> literals, IEnumerable<NumericComparison> numericConditions)
    {
        Literals = Array.AsReadOnly(literals.ToArray());
        NumericConditions = Array.AsReadOnly(numericConditions.ToArray());
    }
}

public sealed class ActionEffect
{
    public IReadOnlyList<Literal<Atom<Fluent>>> Literals { get; }
    public IReadOnlyList<NumericUpdate> NumericEffects { get; }

    internal ActionEffect(IEnumerable<Literal<Atom<Fluent>>> literals, IEnumerable<NumericUpdate> numericEffects)
    {
        Literals = Array.AsReadOnly(literals.ToArray());
        NumericEffects = Array.AsReadOnly(numericEffects.ToArray());
    }
}

public sealed class GroundConjunctiveCondition
{
    public Problem Problem { get; }
    public IReadOnlyList<Literal<Fact>> Literals { get; }
    public IReadOnlyList<GroundNumericComparison> NumericConditions { get; }

    internal GroundConjunctiveCondition(Problem problem, IEnumerable<Literal<Fact>> literals, IEnumerable<GroundNumericComparison> numericConditions)
    {
        Problem = problem;
        NumericConditions = Array.AsReadOnly(numericConditions.ToArray());
        Literals = Array.AsReadOnly(literals.ToArray());
    }
}

public sealed class GroundActionEffect
{
    public IReadOnlyList<Literal<Fact>> Literals { get; }
    public IReadOnlyList<GroundNumericUpdate> NumericEffects { get; }

    internal GroundActionEffect(IEnumerable<Literal<Fact>> literals, IEnumerable<GroundNumericUpdate> numericEffects)
    {
        Literals = Array.AsReadOnly(literals.ToArray());
        NumericEffects = Array.AsReadOnly(numericEffects.ToArray());
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
