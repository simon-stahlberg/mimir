namespace Mimir.Core.Schemas;

public abstract class Predicate
{
    public string Name { get; }
    public IReadOnlyList<Variable> Parameters { get; }

    protected Predicate(string name, IReadOnlyList<Variable> parameters)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentNullException.ThrowIfNull(parameters);

        Variable[] parameterCopy = parameters.ToArray();
        if (parameterCopy.Any(parameter => parameter is null))
            throw new ArgumentException("The collection cannot contain null values.", nameof(parameters));

        Name = name;
        Parameters = Array.AsReadOnly(parameterCopy);
    }

    public override string ToString()
    {
        if (Parameters.Count == 0) return $"({Name})";
        return $"({Name} {string.Join(" ", Parameters)})";
    }
}

public sealed class Predicate<T> : Predicate where T : notnull, IPredicateType
{
    public Predicate(string name, IReadOnlyList<Variable> parameters)
        : base(name, parameters)
    {
    }
}
