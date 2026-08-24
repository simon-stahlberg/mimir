namespace Mimir.Core.Schemas;

public interface ITerm
{
    string Name { get; }
}

public sealed class Variable : ITerm
{
    public string Name { get; }
    public string Type { get; }

    public Variable(string name, string type = "object")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentException.ThrowIfNullOrWhiteSpace(type);

        Name = name;
        Type = type;
    }

    public override string ToString() => Name;
}

public sealed class Constant : ITerm
{
    public string Name { get; }
    public string Type { get; }

    public Constant(string name, string type)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentException.ThrowIfNullOrWhiteSpace(type);

        Name = name;
        Type = type;
    }

    public override string ToString() => Name;
}
