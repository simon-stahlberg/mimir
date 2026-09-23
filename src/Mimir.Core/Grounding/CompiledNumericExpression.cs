using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

internal abstract class CompiledNumericExpression
{
    public static CompiledNumericExpression Compile(NumericExpression expression, IReadOnlyDictionary<Variable, int> slots)
        => expression switch
        {
            NumericConstant constant => new CompiledNumericConstant(constant),
            GroundFunctionCall call => new CompiledGroundFunctionCall(call),
            FunctionCall call => new CompiledLiftedFunctionCall(call, slots),
            NumericBinaryExpression binary => new CompiledNumericBinary(
                binary.Operator, Compile(binary.Left, slots), Compile(binary.Right, slots)),
            _ => throw new InvalidOperationException($"Unsupported numeric expression '{expression.GetType().Name}'.")
        };

    public abstract NumericExpression Ground(Problem problem, Constant?[] bindings);

    public abstract double Evaluate(InstanceContext context, Constant?[] bindings, State? state);

    private protected static double ReadValue(GroundFunctionCall? call, State? state)
    {
        if (call is null) return NumericEvaluation.Undefined;
        if (call.StateIndex is not int index) return call.InitialValue;
        if (state is null) throw new InvalidOperationException("Changing numeric expressions require a state.");
        return state.NumericValues[index];
    }
}

internal sealed class CompiledNumericConstant(NumericConstant constant) : CompiledNumericExpression
{
    public override NumericExpression Ground(Problem problem, Constant?[] bindings) => constant;

    public override double Evaluate(InstanceContext context, Constant?[] bindings, State? state) => constant.Value;
}

internal sealed class CompiledGroundFunctionCall(GroundFunctionCall call) : CompiledNumericExpression
{
    public override NumericExpression Ground(Problem problem, Constant?[] bindings)
        => ReferenceEquals(call.Context, problem.Context)
            ? call
            : throw new ArgumentException("Numeric expression belongs to a different problem.");

    public override double Evaluate(InstanceContext context, Constant?[] bindings, State? state)
        => ReferenceEquals(call.Context, context)
            ? ReadValue(call, state)
            : throw new ArgumentException("Numeric expression belongs to a different problem.");
}

internal sealed class CompiledLiftedFunctionCall : CompiledNumericExpression
{
    private readonly NumericFunction _function;
    private readonly CompiledGroundingTerm[] _terms;
    // Reused across evaluations to keep lookups allocation-free; evaluators are single-threaded (see README).
    private readonly Constant[] _arguments;

    public CompiledLiftedFunctionCall(FunctionCall call, IReadOnlyDictionary<Variable, int> slots)
    {
        _function = call.Function;
        _terms = call.Arguments.Select(term => CompiledGroundingTerm.Compile(term, slots)).ToArray();
        _arguments = new Constant[_terms.Length];
    }

    public override NumericExpression Ground(Problem problem, Constant?[] bindings)
        => problem.Context.GetFunctionCall(_function, ResolveArguments(bindings));

    // A lookup must not register calls: binding search evaluates many candidates that are never grounded.
    public override double Evaluate(InstanceContext context, Constant?[] bindings, State? state)
        => ReadValue(context.TryGetFunctionCall(_function, ResolveArguments(bindings)), state);

    private Constant[] ResolveArguments(Constant?[] bindings)
    {
        for (int i = 0; i < _terms.Length; i++) _arguments[i] = _terms[i].Resolve(bindings);
        return _arguments;
    }
}

internal sealed class CompiledNumericBinary(
    NumericOperator operation,
    CompiledNumericExpression left,
    CompiledNumericExpression right) : CompiledNumericExpression
{
    public override NumericExpression Ground(Problem problem, Constant?[] bindings)
        => new NumericBinaryExpression(operation, left.Ground(problem, bindings), right.Ground(problem, bindings));

    public override double Evaluate(InstanceContext context, Constant?[] bindings, State? state)
        => NumericEvaluation.Apply(operation, left.Evaluate(context, bindings, state), right.Evaluate(context, bindings, state));
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
