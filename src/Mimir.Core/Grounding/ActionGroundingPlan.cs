using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

internal sealed class ActionGroundingPlan
{
    public InstanceContext Context { get; }
    public ActionSchema Schema { get; }
    public int ParameterCount { get; }
    public int MaximumBindingCount { get; }
    public int MaximumArgumentCount { get; }
    public int MaximumConditionCount { get; }
    public int MaximumDerivedConditionCount { get; }
    public bool[] PreconditionRelevantParameters { get; }
    public bool[] TransitionRelevantParameters { get; }
    public CompiledActionCost Cost { get; }
    public CompiledGroundingLiteral<Fluent>[] FluentPreconditions { get; }
    public CompiledGroundingLiteral<Static>[] StaticPreconditions { get; }
    public CompiledGroundingLiteral<Derived>[] DerivedPreconditions { get; }
    public CompiledGroundingEffect[] Effects { get; }

    private ActionGroundingPlan(
        InstanceContext context,
        ActionSchema schema,
        int maximumBindingCount,
        int maximumArgumentCount,
        int maximumConditionCount,
        int maximumDerivedConditionCount,
        bool[] preconditionRelevantParameters,
        bool[] transitionRelevantParameters,
        CompiledActionCost cost,
        CompiledGroundingLiteral<Fluent>[] fluentPreconditions,
        CompiledGroundingLiteral<Static>[] staticPreconditions,
        CompiledGroundingLiteral<Derived>[] derivedPreconditions,
        CompiledGroundingEffect[] effects)
    {
        Context = context;
        Schema = schema;
        ParameterCount = schema.Parameters.Count;
        MaximumBindingCount = maximumBindingCount;
        MaximumArgumentCount = maximumArgumentCount;
        MaximumConditionCount = maximumConditionCount;
        MaximumDerivedConditionCount = maximumDerivedConditionCount;
        PreconditionRelevantParameters = preconditionRelevantParameters;
        TransitionRelevantParameters = transitionRelevantParameters;
        Cost = cost;
        FluentPreconditions = fluentPreconditions;
        StaticPreconditions = staticPreconditions;
        DerivedPreconditions = derivedPreconditions;
        Effects = effects;
    }

    public static ActionGroundingPlan Compile(
        InstanceContext context,
        ActionSchema schema)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(schema);

        var parameterSlots = new Dictionary<Variable, int>(
            schema.Parameters.Count,
            ReferenceEqualityComparer.Instance);
        for (int i = 0; i < schema.Parameters.Count; i++)
            parameterSlots[schema.Parameters[i]] = i;

        int maximumArgumentCount = 0;
        CompiledActionCost cost = CompiledActionCost.Compile(
            schema.CostExpression,
            parameterSlots,
            ref maximumArgumentCount);
        CompiledGroundingLiteral<Fluent>[] fluentPreconditions = CompileLiterals(
            schema.FluentPreconditions,
            parameterSlots,
            ref maximumArgumentCount);
        CompiledGroundingLiteral<Static>[] staticPreconditions = CompileLiterals(
            schema.StaticPreconditions,
            parameterSlots,
            ref maximumArgumentCount);
        CompiledGroundingLiteral<Derived>[] derivedPreconditions = CompileLiterals(
            schema.DerivedPreconditions,
            parameterSlots,
            ref maximumArgumentCount);

        int maximumBindingCount = schema.Parameters.Count;
        int maximumConditionCount = Math.Max(
            schema.FluentPreconditions.Count,
            schema.StaticPreconditions.Count);
        int maximumDerivedConditionCount =
            schema.DerivedPreconditions.Count;
        var effects = new CompiledGroundingEffect[schema.Effects.Count];
        for (int i = 0; i < effects.Length; i++)
        {
            ConditionalEffect effect = schema.Effects[i];
            maximumConditionCount = Math.Max(
                maximumConditionCount,
                Math.Max(
                    effect.FluentConditions.Count,
                    effect.StaticConditions.Count));
            maximumDerivedConditionCount = Math.Max(
                maximumDerivedConditionCount,
                effect.DerivedConditions.Count);
            effects[i] = CompileEffect(
                context,
                effect,
                parameterSlots,
                schema.Parameters.Count,
                ref maximumBindingCount,
                ref maximumArgumentCount);
        }

        return new ActionGroundingPlan(
            context,
            schema,
            maximumBindingCount,
            maximumArgumentCount,
            maximumConditionCount,
            maximumDerivedConditionCount,
            ActionBuilder.FindPreconditionRelevantParameters(schema),
            ActionBuilder.FindTransitionRelevantParameters(schema),
            cost,
            fluentPreconditions,
            staticPreconditions,
            derivedPreconditions,
            effects);
    }

    private static CompiledGroundingEffect CompileEffect(
        InstanceContext context,
        ConditionalEffect effect,
        Dictionary<Variable, int> parameterSlots,
        int parameterCount,
        ref int maximumBindingCount,
        ref int maximumArgumentCount)
    {
        Variable[] referencedVariables =
            ActionBuilder.GetReferencedQuantifiedVariables(effect);
        bool suppressed = false;
        for (int i = 0; i < effect.QuantifiedVariables.Count; i++)
        {
            Variable variable = effect.QuantifiedVariables[i];
            if (referencedVariables.Any(
                    candidate => ReferenceEquals(candidate, variable)))
            {
                continue;
            }

            if (context.GetCompatibleObjects(variable.Type).Length == 0)
                suppressed = true;
        }

        var effectSlots = new Dictionary<Variable, int>(
            parameterSlots,
            ReferenceEqualityComparer.Instance);
        var quantifiedVariables =
            new CompiledGroundingQuantifiedVariable[referencedVariables.Length];
        for (int i = 0; i < referencedVariables.Length; i++)
        {
            Variable variable = referencedVariables[i];
            int slot = parameterCount + i;
            effectSlots[variable] = slot;
            quantifiedVariables[i] = new CompiledGroundingQuantifiedVariable(
                slot,
                context.GetCompatibleObjects(variable.Type));
        }

        maximumBindingCount = Math.Max(
            maximumBindingCount,
            parameterCount + referencedVariables.Length);

        return new CompiledGroundingEffect(
            suppressed,
            quantifiedVariables,
            CompileAtom(effect.Effect.Value, effectSlots, ref maximumArgumentCount),
            effect.Effect.Polarity,
            CompileLiterals(
                effect.FluentConditions,
                effectSlots,
                ref maximumArgumentCount),
            CompileLiterals(
                effect.StaticConditions,
                effectSlots,
                ref maximumArgumentCount),
            CompileLiterals(
                effect.DerivedConditions,
                effectSlots,
                ref maximumArgumentCount));
    }

    private static CompiledGroundingLiteral<T>[] CompileLiterals<T>(
        IReadOnlyList<Literal<Atom<T>>> literals,
        IReadOnlyDictionary<Variable, int> slots,
        ref int maximumArgumentCount)
        where T : IPredicateType
    {
        var compiled = new CompiledGroundingLiteral<T>[literals.Count];
        for (int i = 0; i < compiled.Length; i++)
        {
            Literal<Atom<T>> literal = literals[i];
            compiled[i] = new CompiledGroundingLiteral<T>(
                CompileAtom(literal.Value, slots, ref maximumArgumentCount),
                literal.Polarity);
        }

        return compiled;
    }

    private static CompiledGroundingAtom<T> CompileAtom<T>(
        Atom<T> atom,
        IReadOnlyDictionary<Variable, int> slots,
        ref int maximumArgumentCount)
        where T : IPredicateType
    {
        maximumArgumentCount = Math.Max(
            maximumArgumentCount,
            atom.Arguments.Count);
        var terms = new CompiledGroundingTerm[atom.Arguments.Count];
        for (int i = 0; i < terms.Length; i++)
            terms[i] = CompiledGroundingTerm.Compile(atom.Arguments[i], slots);

        return new CompiledGroundingAtom<T>(atom.Predicate, terms);
    }
}

internal readonly struct CompiledGroundingTerm
{
    private readonly Constant? _constant;
    private readonly int _slot;

    private CompiledGroundingTerm(Constant constant)
    {
        _constant = constant;
        _slot = -1;
    }

    private CompiledGroundingTerm(int slot)
    {
        _constant = null;
        _slot = slot;
    }

    public static CompiledGroundingTerm Compile(
        ITerm term,
        IReadOnlyDictionary<Variable, int> slots)
        => term switch
        {
            Constant constant => new CompiledGroundingTerm(constant),
            Variable variable => new CompiledGroundingTerm(slots[variable]),
            _ => throw new InvalidOperationException(
                $"Unsupported action term '{term.GetType().Name}'.")
        };

    public Constant Resolve(Constant?[] bindings)
        => _constant
            ?? bindings[_slot]
            ?? throw new InvalidOperationException(
                $"Action grounding slot {_slot} is unbound.");
}

internal sealed class CompiledGroundingAtom<T> where T : IPredicateType
{
    public Mimir.Core.Schemas.Predicate<T> Predicate { get; }
    public CompiledGroundingTerm[] Terms { get; }

    public CompiledGroundingAtom(
        Mimir.Core.Schemas.Predicate<T> predicate,
        CompiledGroundingTerm[] terms)
    {
        Predicate = predicate;
        Terms = terms;
    }
}

internal readonly struct CompiledGroundingLiteral<T> where T : IPredicateType
{
    public CompiledGroundingAtom<T> Atom { get; }
    public Polarity Polarity { get; }

    public CompiledGroundingLiteral(
        CompiledGroundingAtom<T> atom,
        Polarity polarity)
    {
        Atom = atom;
        Polarity = polarity;
    }
}

internal readonly struct CompiledGroundingQuantifiedVariable
{
    public int Slot { get; }
    public Constant[] Candidates { get; }

    public CompiledGroundingQuantifiedVariable(int slot, Constant[] candidates)
    {
        Slot = slot;
        Candidates = candidates;
    }
}

internal sealed class CompiledGroundingEffect
{
    public bool IsSuppressed { get; }
    public CompiledGroundingQuantifiedVariable[] QuantifiedVariables { get; }
    public CompiledGroundingAtom<Fluent> Target { get; }
    public Polarity Polarity { get; }
    public CompiledGroundingLiteral<Fluent>[] FluentConditions { get; }
    public CompiledGroundingLiteral<Static>[] StaticConditions { get; }
    public CompiledGroundingLiteral<Derived>[] DerivedConditions { get; }
    public bool IsConditional => FluentConditions.Length != 0
        || StaticConditions.Length != 0
        || DerivedConditions.Length != 0;

    public CompiledGroundingEffect(
        bool isSuppressed,
        CompiledGroundingQuantifiedVariable[] quantifiedVariables,
        CompiledGroundingAtom<Fluent> target,
        Polarity polarity,
        CompiledGroundingLiteral<Fluent>[] fluentConditions,
        CompiledGroundingLiteral<Static>[] staticConditions,
        CompiledGroundingLiteral<Derived>[] derivedConditions)
    {
        IsSuppressed = isSuppressed;
        QuantifiedVariables = quantifiedVariables;
        Target = target;
        Polarity = polarity;
        FluentConditions = fluentConditions;
        StaticConditions = staticConditions;
        DerivedConditions = derivedConditions;
    }
}

internal sealed class CompiledActionCost
{
    private readonly ActionCostExpression _source;
    private readonly CompiledActionCost? _left;
    private readonly CompiledActionCost? _right;
    private readonly NumericFunction? _function;
    private readonly CompiledGroundingTerm[]? _arguments;

    private CompiledActionCost(
        ActionCostExpression source,
        CompiledActionCost? left = null,
        CompiledActionCost? right = null,
        NumericFunction? function = null,
        CompiledGroundingTerm[]? arguments = null)
    {
        _source = source;
        _left = left;
        _right = right;
        _function = function;
        _arguments = arguments;
    }

    public static CompiledActionCost Compile(
        ActionCostExpression expression,
        IReadOnlyDictionary<Variable, int> slots,
        ref int maximumArgumentCount)
    {
        switch (expression)
        {
            case ConstantActionCostExpression:
                return new CompiledActionCost(expression);
            case BinaryActionCostExpression binary:
                return new CompiledActionCost(
                    expression,
                    Compile(binary.Left, slots, ref maximumArgumentCount),
                    Compile(binary.Right, slots, ref maximumArgumentCount));
            case NumericFunctionActionCostExpression function:
            {
                maximumArgumentCount = Math.Max(
                    maximumArgumentCount,
                    function.Arguments.Count);
                var arguments = new CompiledGroundingTerm[function.Arguments.Count];
                for (int i = 0; i < arguments.Length; i++)
                    arguments[i] = CompiledGroundingTerm.Compile(function.Arguments[i], slots);

                return new CompiledActionCost(
                    expression,
                    function: function.Function,
                    arguments: arguments);
            }
            default:
                throw new InvalidOperationException(
                    $"Unsupported action-cost expression '{expression.GetType().Name}'.");
        }
    }

    public double Evaluate(
        Problem problem,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        if (_source is ConstantActionCostExpression constant)
            return constant.Value;

        if (_source is BinaryActionCostExpression binary)
        {
            double left = _left!.Evaluate(problem, bindings, workspace);
            double right = _right!.Evaluate(problem, bindings, workspace);
            return binary.Operator switch
            {
                ActionCostBinaryOperator.Add => left + right,
                ActionCostBinaryOperator.Subtract => left - right,
                ActionCostBinaryOperator.Multiply => left * right,
                ActionCostBinaryOperator.Divide => right == 0d
                    ? throw new InvalidOperationException(
                        "Action cost division by zero is not supported.")
                    : left / right,
                _ => throw new InvalidOperationException(
                    $"Unsupported action cost operator '{binary.Operator}'.")
            };
        }

        Constant[] arguments = workspace.GetArgumentBuffer(_arguments!.Length);
        for (int i = 0; i < arguments.Length; i++)
            arguments[i] = _arguments[i].Resolve(bindings);

        return problem.GetNumericFunctionValue(_function!, arguments);
    }
}

internal sealed class ActionGroundingWorkspace
{
    private Constant[][] _argumentBuffers = Array.Empty<Constant[]>();
    private Constant?[] _bindings = Array.Empty<Constant?>();

    public Constant?[] Bindings => _bindings;
    public List<int> PositiveIndices { get; } = new();
    public List<int> NegativeIndices { get; } = new();
    public List<int> AddEffectIndices { get; } = new();
    public List<int> DeleteEffectIndices { get; } = new();
    public List<Literal<Fact<Derived>>> ActionDerivedConditions { get; } = new();
    public List<Literal<Fact<Derived>>> EffectDerivedConditions { get; } = new();
    public List<GroundConditionalEffect> ConditionalEffects { get; } = new();

    public void Prepare(ActionGroundingPlan plan)
    {
        if (_bindings.Length < plan.MaximumBindingCount)
        {
            int size = Math.Max(plan.MaximumBindingCount, _bindings.Length * 2);
            Array.Resize(ref _bindings, size);
        }

        if (_argumentBuffers.Length <= plan.MaximumArgumentCount)
            Array.Resize(ref _argumentBuffers, plan.MaximumArgumentCount + 1);

        PositiveIndices.EnsureCapacity(plan.MaximumConditionCount);
        NegativeIndices.EnsureCapacity(plan.MaximumConditionCount);
        ActionDerivedConditions.EnsureCapacity(plan.DerivedPreconditions.Length);
        EffectDerivedConditions.EnsureCapacity(
            plan.MaximumDerivedConditionCount);
        AddEffectIndices.EnsureCapacity(plan.Effects.Length);
        DeleteEffectIndices.EnsureCapacity(plan.Effects.Length);
        ConditionalEffects.EnsureCapacity(plan.Effects.Length);
    }

    public Constant[] GetArgumentBuffer(int argumentCount)
    {
        if (argumentCount == 0)
            return Array.Empty<Constant>();

        return _argumentBuffers[argumentCount]
            ??= new Constant[argumentCount];
    }

    public void PrepareForReturn()
    {
        Array.Clear(_bindings);
        for (int i = 1; i < _argumentBuffers.Length; i++)
        {
            Constant[]? buffer = _argumentBuffers[i];
            if (buffer is not null)
                Array.Clear(buffer);
        }

        PositiveIndices.Clear();
        NegativeIndices.Clear();
        AddEffectIndices.Clear();
        DeleteEffectIndices.Clear();
        ActionDerivedConditions.Clear();
        EffectDerivedConditions.Clear();
        ConditionalEffects.Clear();
    }
}
