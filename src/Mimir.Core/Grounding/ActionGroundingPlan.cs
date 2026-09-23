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
    public CompiledNumericExpression Cost { get; }
    public CompiledGroundingLiteral<Fluent>[] FluentPreconditions { get; }
    public CompiledGroundingLiteral<Static>[] StaticPreconditions { get; }
    public CompiledGroundingLiteral<Derived>[] DerivedPreconditions { get; }
    public CompiledGroundingEffect[] Effects { get; }
    public CompiledGroundingNumericEffect[] NumericEffects { get; }
    public CompiledNumericComparison[] NumericPreconditions { get; }

    private ActionGroundingPlan(
        InstanceContext context,
        ActionSchema schema,
        int maximumBindingCount,
        int maximumArgumentCount,
        int maximumConditionCount,
        int maximumDerivedConditionCount,
        bool[] preconditionRelevantParameters,
        bool[] transitionRelevantParameters,
        CompiledNumericExpression cost,
        CompiledGroundingLiteral<Fluent>[] fluentPreconditions,
        CompiledGroundingLiteral<Static>[] staticPreconditions,
        CompiledGroundingLiteral<Derived>[] derivedPreconditions,
        CompiledGroundingEffect[] effects,
        CompiledGroundingNumericEffect[] numericEffects,
        CompiledNumericComparison[] numericPreconditions)
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
        NumericEffects = numericEffects;
        NumericPreconditions = numericPreconditions;
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
        CompiledNumericExpression cost = CompiledNumericExpression.Compile(schema.CostExpression, parameterSlots);
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
        foreach (ConditionalEffectBase effect in schema.Effects.Concat<ConditionalEffectBase>(schema.NumericEffects))
        {
            maximumConditionCount = Math.Max(
                maximumConditionCount,
                Math.Max(
                    effect.FluentConditions.Count,
                    effect.StaticConditions.Count));
            maximumDerivedConditionCount = Math.Max(
                maximumDerivedConditionCount,
                effect.DerivedConditions.Count);
        }

        var effects = new CompiledGroundingEffect[schema.Effects.Count];
        for (int i = 0; i < effects.Length; i++)
        {
            effects[i] = CompileEffect(
                context,
                schema.Effects[i],
                parameterSlots,
                schema.Parameters.Count,
                ref maximumBindingCount,
                ref maximumArgumentCount);
        }

        var numericEffects = new CompiledGroundingNumericEffect[schema.NumericEffects.Count];
        for (int i = 0; i < numericEffects.Length; i++)
        {
            numericEffects[i] = CompileNumericEffect(
                context,
                schema.NumericEffects[i],
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
            effects,
            numericEffects,
            CompiledNumericComparison.Compile(schema.NumericPreconditions, parameterSlots));
    }

    private static CompiledGroundingEffect CompileEffect(
        InstanceContext context,
        ConditionalEffect effect,
        Dictionary<Variable, int> parameterSlots,
        int parameterCount,
        ref int maximumBindingCount,
        ref int maximumArgumentCount)
    {
        (bool suppressed, CompiledGroundingQuantifiedVariable[] quantifiedVariables, Dictionary<Variable, int> effectSlots) =
            CompileQuantification(
                context,
                effect,
                ActionBuilder.GetReferencedQuantifiedVariables(effect),
                parameterSlots,
                parameterCount,
                ref maximumBindingCount);

        return new CompiledGroundingEffect(
            suppressed,
            quantifiedVariables,
            CompileLiterals(effect.FluentConditions, effectSlots, ref maximumArgumentCount),
            CompileLiterals(effect.StaticConditions, effectSlots, ref maximumArgumentCount),
            CompileLiterals(effect.DerivedConditions, effectSlots, ref maximumArgumentCount),
            CompiledNumericComparison.Compile(effect.NumericConditions, effectSlots),
            CompileAtom(effect.Effect.Value, effectSlots, ref maximumArgumentCount),
            effect.Effect.Polarity);
    }

    private static CompiledGroundingNumericEffect CompileNumericEffect(
        InstanceContext context,
        ConditionalNumericEffect effect,
        Dictionary<Variable, int> parameterSlots,
        int parameterCount,
        ref int maximumBindingCount,
        ref int maximumArgumentCount)
    {
        // Each binding of a quantified numeric update contributes its own write (forall + increase sums), so
        // unreferenced quantified variables cannot be collapsed as they can for idempotent literal effects.
        (bool suppressed, CompiledGroundingQuantifiedVariable[] quantifiedVariables, Dictionary<Variable, int> effectSlots) =
            CompileQuantification(
                context,
                effect,
                effect.QuantifiedVariables.ToArray(),
                parameterSlots,
                parameterCount,
                ref maximumBindingCount);

        return new CompiledGroundingNumericEffect(
            suppressed,
            quantifiedVariables,
            CompileLiterals(effect.FluentConditions, effectSlots, ref maximumArgumentCount),
            CompileLiterals(effect.StaticConditions, effectSlots, ref maximumArgumentCount),
            CompileLiterals(effect.DerivedConditions, effectSlots, ref maximumArgumentCount),
            CompiledNumericComparison.Compile(effect.NumericConditions, effectSlots),
            new CompiledNumericUpdate(effect.Effect, effectSlots));
    }

    private static (bool Suppressed, CompiledGroundingQuantifiedVariable[] QuantifiedVariables, Dictionary<Variable, int> Slots)
        CompileQuantification(
            InstanceContext context,
            ConditionalEffectBase effect,
            Variable[] referencedVariables,
            Dictionary<Variable, int> parameterSlots,
            int parameterCount,
            ref int maximumBindingCount)
    {
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
        return (suppressed, quantifiedVariables, effectSlots);
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

internal abstract class CompiledGroundingEffectBase
{
    public bool IsSuppressed { get; }
    public CompiledGroundingQuantifiedVariable[] QuantifiedVariables { get; }
    public CompiledGroundingLiteral<Fluent>[] FluentConditions { get; }
    public CompiledGroundingLiteral<Static>[] StaticConditions { get; }
    public CompiledGroundingLiteral<Derived>[] DerivedConditions { get; }
    public CompiledNumericComparison[] NumericConditions { get; }
    public bool IsConditional => FluentConditions.Length != 0
        || StaticConditions.Length != 0
        || DerivedConditions.Length != 0
        || NumericConditions.Length != 0;

    protected CompiledGroundingEffectBase(
        bool isSuppressed,
        CompiledGroundingQuantifiedVariable[] quantifiedVariables,
        CompiledGroundingLiteral<Fluent>[] fluentConditions,
        CompiledGroundingLiteral<Static>[] staticConditions,
        CompiledGroundingLiteral<Derived>[] derivedConditions,
        CompiledNumericComparison[] numericConditions)
    {
        IsSuppressed = isSuppressed;
        QuantifiedVariables = quantifiedVariables;
        FluentConditions = fluentConditions;
        StaticConditions = staticConditions;
        DerivedConditions = derivedConditions;
        NumericConditions = numericConditions;
    }
}

internal sealed class CompiledGroundingEffect : CompiledGroundingEffectBase
{
    public CompiledGroundingAtom<Fluent> Target { get; }
    public Polarity Polarity { get; }

    public CompiledGroundingEffect(
        bool isSuppressed,
        CompiledGroundingQuantifiedVariable[] quantifiedVariables,
        CompiledGroundingLiteral<Fluent>[] fluentConditions,
        CompiledGroundingLiteral<Static>[] staticConditions,
        CompiledGroundingLiteral<Derived>[] derivedConditions,
        CompiledNumericComparison[] numericConditions,
        CompiledGroundingAtom<Fluent> target,
        Polarity polarity)
        : base(isSuppressed, quantifiedVariables, fluentConditions, staticConditions, derivedConditions, numericConditions)
    {
        Target = target;
        Polarity = polarity;
    }
}

internal sealed class CompiledGroundingNumericEffect : CompiledGroundingEffectBase
{
    public CompiledNumericUpdate Update { get; }

    public CompiledGroundingNumericEffect(
        bool isSuppressed,
        CompiledGroundingQuantifiedVariable[] quantifiedVariables,
        CompiledGroundingLiteral<Fluent>[] fluentConditions,
        CompiledGroundingLiteral<Static>[] staticConditions,
        CompiledGroundingLiteral<Derived>[] derivedConditions,
        CompiledNumericComparison[] numericConditions,
        CompiledNumericUpdate update)
        : base(isSuppressed, quantifiedVariables, fluentConditions, staticConditions, derivedConditions, numericConditions)
    {
        Update = update;
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
    public List<GroundNumericUpdate> NumericEffects { get; } = new();
    public List<GroundConditionalNumericEffect> ConditionalNumericEffects { get; } = new();

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
        NumericEffects.Clear();
        ConditionalNumericEffects.Clear();
    }
}
