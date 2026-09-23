using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

internal sealed class CompiledNumericExpression
{
    private readonly NumericExpression _source;
    private readonly CompiledNumericExpression? _left;
    private readonly CompiledNumericExpression? _right;
    private readonly CompiledGroundingTerm[] _terms;
    // Reused across evaluations to keep lookups allocation-free; evaluators are single-threaded (see README).
    private readonly Constant[] _arguments;

    private CompiledNumericExpression(NumericExpression source,
        CompiledNumericExpression? left = null, CompiledNumericExpression? right = null,
        CompiledGroundingTerm[]? terms = null)
    {
        _source = source;
        _left = left;
        _right = right;
        _terms = terms ?? Array.Empty<CompiledGroundingTerm>();
        _arguments = new Constant[_terms.Length];
    }

    public static CompiledNumericExpression Compile(NumericExpression expression, IReadOnlyDictionary<Variable, int> slots)
    {
        switch (expression)
        {
            case NumericConstant or GroundFunctionCall:
                return new CompiledNumericExpression(expression);
            case NumericBinaryExpression binary:
                return new CompiledNumericExpression(expression, Compile(binary.Left, slots), Compile(binary.Right, slots));
            case FunctionCall call:
                return new CompiledNumericExpression(expression,
                    terms: call.Arguments.Select(term => CompiledGroundingTerm.Compile(term, slots)).ToArray());
            default:
                throw new InvalidOperationException($"Unsupported numeric expression '{expression.GetType().Name}'.");
        }
    }

    public NumericExpression Ground(Problem problem, Constant?[] bindings)
    {
        return _source switch
        {
            NumericConstant => _source,
            GroundFunctionCall call when ReferenceEquals(call.Context, problem.Context) => call,
            GroundFunctionCall => throw new ArgumentException("Numeric expression belongs to a different problem."),
            FunctionCall call => problem.Context.GetFunctionCall(call.Function, ResolveArguments(bindings)),
            NumericBinaryExpression binary => new NumericBinaryExpression(binary.Operator,
                _left!.Ground(problem, bindings), _right!.Ground(problem, bindings)),
            _ => throw new InvalidOperationException("Unknown numeric expression.")
        };
    }

    public double Evaluate(InstanceContext context, Constant?[] bindings, State? state)
    {
        if (_source is NumericConstant constant) return constant.Value;
        if (_source is NumericBinaryExpression binary)
            return NumericEvaluation.Apply(binary.Operator,
                _left!.Evaluate(context, bindings, state), _right!.Evaluate(context, bindings, state));
        GroundFunctionCall? call = _source switch
        {
            GroundFunctionCall grounded when ReferenceEquals(grounded.Context, context) => grounded,
            GroundFunctionCall => throw new ArgumentException("Numeric expression belongs to a different problem."),
            // A lookup must not register calls: binding search evaluates many candidates that are never grounded.
            FunctionCall lifted => context.TryGetFunctionCall(lifted.Function, ResolveArguments(bindings)),
            _ => throw new InvalidOperationException("Unknown numeric expression.")
        };
        if (call is null) return NumericEvaluation.Undefined;
        if (call.StateIndex is not int index) return call.InitialValue;
        if (state is null) throw new InvalidOperationException("Changing numeric expressions require a state.");
        return state.NumericValues[index];
    }

    private Constant[] ResolveArguments(Constant?[] bindings)
    {
        for (int i = 0; i < _terms.Length; i++) _arguments[i] = _terms[i].Resolve(bindings);
        return _arguments;
    }
}

internal sealed class CompiledNumericComparison
{
    public CompiledNumericExpression Left { get; }
    public CompiledNumericExpression Right { get; }
    public ComparisonOperator Operator { get; }
    public int[] VariableIndices { get; }

    public CompiledNumericComparison(NumericComparison comparison, IReadOnlyDictionary<Variable, int> slots)
    {
        Left = CompiledNumericExpression.Compile(comparison.Left, slots);
        Right = CompiledNumericExpression.Compile(comparison.Right, slots);
        Operator = comparison.Operator;
        VariableIndices = comparison.Variables().Select(variable => slots[variable]).Distinct().Order().ToArray();
    }

    public GroundNumericComparison Ground(Problem problem, Constant?[] bindings)
        => new(Left.Ground(problem, bindings), Operator, Right.Ground(problem, bindings));

    public bool Evaluate(InstanceContext context, Constant?[] bindings, State? state)
        => NumericEvaluation.Compare(Operator,
            Left.Evaluate(context, bindings, state), Right.Evaluate(context, bindings, state));

    public static CompiledNumericComparison[] Compile(IReadOnlyList<NumericComparison> comparisons,
        IReadOnlyDictionary<Variable, int> slots)
        => comparisons.Select(comparison => new CompiledNumericComparison(comparison, slots)).ToArray();
}

internal sealed class CompiledNumericUpdate
{
    private readonly NumericFunction _target;
    private readonly CompiledGroundingTerm[] _targetArguments;
    private readonly NumericUpdateOperator _operator;
    private readonly CompiledNumericExpression _expression;

    public CompiledNumericUpdate(NumericUpdate update, IReadOnlyDictionary<Variable, int> slots)
    {
        _target = update.Target.Function;
        _targetArguments = update.Target.Arguments.Select(term => CompiledGroundingTerm.Compile(term, slots)).ToArray();
        _operator = update.Operator;
        _expression = CompiledNumericExpression.Compile(update.Expression, slots);
    }

    public GroundNumericUpdate Ground(Problem problem, Constant?[] bindings)
    {
        var arguments = new Constant[_targetArguments.Length];
        for (int i = 0; i < arguments.Length; i++)
            arguments[i] = _targetArguments[i].Resolve(bindings);
        return new GroundNumericUpdate(problem.Context.GetFunctionCall(_target, arguments), _operator,
            _expression.Ground(problem, bindings));
    }
}
