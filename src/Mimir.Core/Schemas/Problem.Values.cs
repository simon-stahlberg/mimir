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
        return new GroundFunctionCall(this, function, arguments.Select(argument =>
            ObjectLookup.TryGetValue(argument, out Constant? value) ? value
                : throw new ArgumentException($"Unknown object '{argument}'.")).ToArray());
    }

    public FunctionCall NewFunctionCall(NumericFunction function, IReadOnlyList<ITerm> arguments)
    {
        ArgumentNullException.ThrowIfNull(function);
        ArgumentNullException.ThrowIfNull(arguments);
        if (!Domain.Contains(function)) throw new ArgumentException("Function belongs to a different domain.");
        if (arguments.Count != function.Parameters.Count) throw new ArgumentException("Incorrect function arity.");
        for (int index = 0; index < arguments.Count; index++)
        {
            ITerm argument = arguments[index] ?? throw new ArgumentException("Arguments cannot contain null.");
            string type = argument switch
            {
                Constant constant when Context.ContainsObject(constant) => constant.Type,
                Variable variable when ContainsDynamicVariable(variable) => variable.Type,
                _ => throw new ArgumentException("Function argument belongs to a different problem.")
            };
            if (!Domain.IsCompatible(type, function.Parameters[index].Type))
                throw new ArgumentException($"Incompatible argument type for '{function.Name}'.");
        }
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
