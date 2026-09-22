namespace Mimir.Core.Schemas;

public sealed class NumericFunction
{
    internal const string TotalCostName = "total-cost";

    internal static bool IsTotalCost(string name) => name.Equals(TotalCostName, StringComparison.OrdinalIgnoreCase);

    public string Name { get; }
    public IReadOnlyList<Variable> Parameters { get; }

    public NumericFunction(string name, IReadOnlyList<Variable> parameters)
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
