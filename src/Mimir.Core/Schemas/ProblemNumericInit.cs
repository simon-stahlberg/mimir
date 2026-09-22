using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using System.Runtime.CompilerServices;

namespace Mimir.Core.Schemas;

internal readonly struct NumericFunctionKey : IEquatable<NumericFunctionKey>
{
    private readonly NumericFunction _function;
    private readonly int _arity;
    private readonly Constant? _arg1;
    private readonly Constant? _arg2;
    private readonly Constant? _arg3;
    private readonly IReadOnlyList<Constant>? _arguments;

    internal NumericFunction Function => _function;
    internal NumericFunctionKey Snapshot() => _arity > 3
        ? new NumericFunctionKey(_function, _arguments!.ToArray()) : this;

    public NumericFunctionKey(NumericFunction function, IReadOnlyList<Constant> arguments)
    {
        _function = function;
        _arity = arguments.Count;
        _arg1 = arguments.Count > 0 ? arguments[0] : null;
        _arg2 = arguments.Count > 1 ? arguments[1] : null;
        _arg3 = arguments.Count > 2 ? arguments[2] : null;
        // Only higher arities retain the collection. Initialization keys receive a
        // private array; lookup keys borrow theirs only during Dictionary.TryGetValue.
        _arguments = arguments.Count > 3 ? arguments : null;
    }

    public override string ToString()
    {
        var arguments = new List<string>(_arity);
        if (_arity > 0) arguments.Add(_arg1!.Name);
        if (_arity > 1) arguments.Add(_arg2!.Name);
        if (_arity > 2) arguments.Add(_arg3!.Name);
        for (int index = 3; index < _arity; index++) arguments.Add(_arguments![index].Name);
        return $"({_function.Name}{string.Concat(arguments.Select(argument => " " + argument))})";
    }

    public bool Equals(NumericFunctionKey other)
    {
        if (!ReferenceEquals(_function, other._function)) return false;
        if (_arity != other._arity) return false;
        if (!ReferenceEquals(_arg1, other._arg1)) return false;
        if (!ReferenceEquals(_arg2, other._arg2)) return false;
        if (!ReferenceEquals(_arg3, other._arg3)) return false;
        for (int i = 3; i < _arity; i++)
        {
            if (!ReferenceEquals(_arguments![i], other._arguments![i])) return false;
        }

        return true;
    }

    public override bool Equals(object? obj) => obj is NumericFunctionKey other && Equals(other);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(_function));
        hash.Add(_arity);
        hash.Add(_arg1 is null ? 0 : RuntimeHelpers.GetHashCode(_arg1));
        hash.Add(_arg2 is null ? 0 : RuntimeHelpers.GetHashCode(_arg2));
        hash.Add(_arg3 is null ? 0 : RuntimeHelpers.GetHashCode(_arg3));
        for (int i = 3; i < _arity; i++)
            hash.Add(RuntimeHelpers.GetHashCode(_arguments![i]));
        return hash.ToHashCode();
    }
}

internal static class ProblemNumericInit
{
    public static void RegisterNumericInitialization(
        Dictionary<NumericFunctionKey, double> numericFunctionValues,
        IReadOnlyDictionary<string, Constant> objectLookup,
        NumericInitialization numericInitialization,
        IReadOnlyDictionary<string, NumericFunction> allFunctions,
        bool actionCostsEnabled,
        ref bool totalCostInitialized)
    {
        NumberLiteral numberLiteral = numericInitialization.Value;

        if (NumericFunction.IsTotalCost(numericInitialization.Fluent.Name))
        {
            if (!actionCostsEnabled)
                throw new NotSupportedException("Initializing total-cost requires the :action-costs requirement.");
            if (!numericInitialization.Fluent.Arguments.IsDefaultOrEmpty)
                throw new NotSupportedException("The built-in total-cost function may not take arguments.");
            if (numberLiteral.Value != 0m)
                throw new NotSupportedException("Only an initial total-cost of 0 is supported.");
            if (totalCostInitialized)
                throw new InvalidOperationException("The built-in total-cost function was initialized more than once.");

            totalCostInitialized = true;
            return;
        }

        Constant[] arguments = numericInitialization.Fluent.Arguments
            .Select(argument =>
            {
                if (argument.IsVariable)
                    throw new NotSupportedException("Problem numeric initialization may not contain variables.");

                if (!objectLookup.TryGetValue(argument.Name, out Constant? constant))
                {
                    throw new InvalidOperationException(
                        $"Numeric initialization references undeclared object or constant '{argument.Name}'.");
                }

                return constant;
            })
            .ToArray();

        if (!allFunctions.TryGetValue(numericInitialization.Fluent.Name, out var function))
            throw new InvalidOperationException($"Numeric function '{numericInitialization.Fluent.Name}' is not declared in the domain.");

        var key = new NumericFunctionKey(function, arguments);

        double value = numberLiteral is ProgrammaticNumberLiteral programmatic
            ? programmatic.RuntimeValue
            : (double)numberLiteral.Value;
        if (!numericFunctionValues.TryAdd(key, value))
            throw new InvalidOperationException($"Numeric function '{function.Name}' was initialized more than once for the same arguments.");
    }
}
