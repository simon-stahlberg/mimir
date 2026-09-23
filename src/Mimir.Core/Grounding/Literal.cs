namespace Mimir.Core.Grounding;


public enum Polarity
{
    Positive,
    Negative
}

public abstract record Literal(Polarity Polarity)
{
    private Polarity _polarity = RequirePolarity(Polarity);

    public Polarity Polarity
    {
        get => _polarity;
        init => _polarity = RequirePolarity(value);
    }

    public bool IsPositive => Polarity == Polarity.Positive;
    public bool IsNegative => Polarity == Polarity.Negative;

    private static Polarity RequirePolarity(Polarity polarity)
    {
        if (!Enum.IsDefined(polarity))
            throw new ArgumentOutOfRangeException(nameof(Polarity), polarity, null);

        return polarity;
    }
}

public record Literal<T>(T Value, Polarity Polarity) : Literal(Polarity) where T : notnull
{
    private T _value = RequireValue(Value);

    public T Value
    {
        get => _value;
        init => _value = RequireValue(value);
    }

    private static T RequireValue(T value)
    {
        if (value is null) throw new ArgumentNullException(nameof(Value));
        return value;
    }

    public override string ToString() => IsPositive ? Value.ToString()! : $"(not {Value})";
}
