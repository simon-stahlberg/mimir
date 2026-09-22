using System.Collections.Frozen;
using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

internal readonly record struct NumericFluentIndex(int Value);
internal readonly record struct NumericField(NumericFluentIndex? Index, double InitialValue);

internal sealed class NumericStateLayout
{
    private readonly FrozenDictionary<NumericFunctionKey, NumericField> _fields;
    private readonly double[] _initialValues;

    internal int Count => _initialValues.Length;
    internal ReadOnlySpan<double> InitialValues => _initialValues;

    internal NumericStateLayout(
        IReadOnlyDictionary<NumericFunctionKey, double> initialValues,
        IReadOnlyCollection<NumericFunction> changingFunctions)
    {
        ArgumentNullException.ThrowIfNull(initialValues);
        ArgumentNullException.ThrowIfNull(changingFunctions);
        if (changingFunctions.Any(function => function is null))
            throw new ArgumentException("Changing functions cannot contain null values.", nameof(changingFunctions));
        var changing = new HashSet<NumericFunction>(changingFunctions);

        if (initialValues.Count == 0)
        {
            _fields = FrozenDictionary<NumericFunctionKey, NumericField>.Empty;
            _initialValues = Array.Empty<double>();
            return;
        }

        var fields = new Dictionary<NumericFunctionKey, NumericField>(initialValues.Count);
        var values = new List<double>();
        foreach ((NumericFunctionKey key, double value) in initialValues)
        {
            if (double.IsInfinity(value))
                throw new ArgumentException($"Numeric field '{key}' has a non-finite initial value.", nameof(initialValues));
            NumericFluentIndex? index = null;
            if (changing.Contains(key.Function))
            {
                index = new NumericFluentIndex(values.Count);
                values.Add(value);
            }
            fields.Add(key.Snapshot(), new NumericField(index, value));
        }
        _fields = fields.ToFrozenDictionary();
        _initialValues = values.Count == 0 ? Array.Empty<double>() : values.ToArray();
    }

    // Fields outside the layout were never initialized and no assign effect can define them, so they stay
    // undefined forever.
    internal NumericField Resolve(NumericFunction function, IReadOnlyList<Constant> arguments)
        => _fields.TryGetValue(new NumericFunctionKey(function, arguments), out NumericField field)
            ? field
            : new NumericField(null, NumericEvaluation.Undefined);
}
