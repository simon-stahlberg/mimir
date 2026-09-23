using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public partial class Problem
{
    public GroundFunctionCall FunctionCall(string name, params string[] arguments)
    {
        ArgumentNullException.ThrowIfNull(name);
        ArgumentNullException.ThrowIfNull(arguments);
        NumericFunction function = Domain.Functions.FirstOrDefault(function =>
            function.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
            ?? throw new ArgumentException($"Unknown numeric function '{name}'.", nameof(name));
        return FunctionCall(function, arguments.Select(argument =>
            ObjectLookup.TryGetValue(argument, out Constant? value) ? value
                : throw new ArgumentException($"Unknown object '{argument}'.")).ToArray());
    }

    public GroundFunctionCall FunctionCall(NumericFunction function, IReadOnlyList<Constant> arguments)
        => Context.GetFunctionCall(function, arguments);

    public FunctionCall NewFunctionCall(NumericFunction function, IReadOnlyList<ITerm> arguments)
    {
        ArgumentNullException.ThrowIfNull(arguments);
        var dynamicVariables = arguments.OfType<Variable>().Where(ContainsDynamicVariable).ToHashSet();
        Context.ValidateNumericFunctionArguments(function, arguments, dynamicVariables, nameof(arguments));
        return new FunctionCall(function, arguments);
    }

    public Fact Atom(string name, params string[] arguments)
    {
        ArgumentNullException.ThrowIfNull(name);
        ArgumentNullException.ThrowIfNull(arguments);
        if (!AllPredicates.TryGetValue(name, out Predicate? predicate))
            throw new ArgumentException($"Unknown predicate '{name}'.", nameof(name));
        Constant[] objects = arguments.Select(argument =>
            ObjectLookup.TryGetValue(argument, out Constant? value) ? value
                : throw new ArgumentException($"Unknown object '{argument}'.")).ToArray();
        return predicate switch
        {
            Predicate<Fluent> fluent => Context.RegisterFact(fluent, objects),
            Predicate<Static> stat => Context.RegisterFact(stat, objects),
            Predicate<Derived> derived => Context.RegisterFact(derived, objects),
            _ => throw new ArgumentException("Unknown predicate type.")
        };
    }
}
