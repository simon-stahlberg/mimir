namespace Mimir.Core.Grounding;

using Schemas;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;

internal sealed class ActionBinding :
    ReadOnlyCollection<Constant>,
    IEquatable<ActionBinding>
{
    private int _hashCode;
    private bool _hashCodeComputed;

    public InstanceContext Context { get; }
    public ActionSchema Schema { get; }
    internal Constant[] Arguments => (Constant[])Items;

    public ActionBinding(
        InstanceContext context,
        ActionSchema schema,
        IReadOnlyList<Constant> arguments,
        bool takeArgumentOwnership)
        : base(GetArguments(arguments, takeArgumentOwnership))
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(schema);

        Context = context;
        Schema = schema;
    }

    private static Constant[] GetArguments(
        IReadOnlyList<Constant> arguments,
        bool takeArgumentOwnership)
    {
        ArgumentNullException.ThrowIfNull(arguments);
        return takeArgumentOwnership
            ? (Constant[])arguments
            : arguments.ToArray();
    }

    public bool Equals(ActionBinding? other)
    {
        if (ReferenceEquals(this, other))
            return true;
        if (other is null || GetHashCode() != other.GetHashCode())
            return false;
        if (!ReferenceEquals(Context, other.Context)
            || !ReferenceEquals(Schema, other.Schema)
            || Count != other.Count)
        {
            return false;
        }

        for (int index = 0; index < Count; index++)
        {
            if (!ReferenceEquals(this[index], other[index]))
                return false;
        }

        return true;
    }

    public override bool Equals(object? obj) => Equals(obj as ActionBinding);

    public override int GetHashCode()
    {
        if (_hashCodeComputed)
            return _hashCode;

        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(Context));
        hash.Add(RuntimeHelpers.GetHashCode(Schema));
        hash.Add(Count);
        for (int index = 0; index < Count; index++)
            hash.Add(RuntimeHelpers.GetHashCode(this[index]));

        _hashCode = hash.ToHashCode();
        _hashCodeComputed = true;
        return _hashCode;
    }

}

internal sealed class ActionPreconditions :
    ReadOnlyCollection<Literal<Fact<Derived>>>
{
    private readonly bool _staticDerivedPreconditionsSatisfied;
    private readonly Literal<Fact<Derived>>[] _stateDependentDerivedPreconditions;

    internal IReadOnlyList<GroundNumericComparison> Comparisons { get; }
    internal OffsetBitboard PositiveFluent { get; }
    internal OffsetBitboard NegativeFluent { get; }
    internal OffsetBitboard PositiveStatic { get; }
    internal OffsetBitboard NegativeStatic { get; }

    public ActionPreconditions(
        InstanceContext context,
        OffsetBitboard positiveFluent,
        OffsetBitboard negativeFluent,
        OffsetBitboard positiveStatic,
        OffsetBitboard negativeStatic,
        IReadOnlyList<Literal<Fact<Derived>>> derivedPreconditions,
        IReadOnlyList<GroundNumericComparison>? comparisons = null)
        : base(GetDerivedPreconditions(derivedPreconditions))
    {
        ArgumentNullException.ThrowIfNull(context);

        Comparisons = comparisons is null || comparisons.Count == 0
            ? Array.Empty<GroundNumericComparison>() : Array.AsReadOnly(comparisons.ToArray());
        PositiveFluent = positiveFluent;
        NegativeFluent = negativeFluent;
        PositiveStatic = positiveStatic;
        NegativeStatic = negativeStatic;

        bool staticDerivedPreconditionsSatisfied = true;
        List<Literal<Fact<Derived>>>? stateDependentDerivedPreconditions = null;
        for (int index = 0; index < Count; index++)
        {
            Literal<Fact<Derived>> literal = this[index];
            if (!context.IsStaticDerivedPrevalidated(literal.Value.Predicate))
            {
                stateDependentDerivedPreconditions ??=
                    new List<Literal<Fact<Derived>>>();
                stateDependentDerivedPreconditions.Add(literal);
                continue;
            }

            bool holds = context.IsStaticDerivedTruePrevalidated(literal.Value);
            if (holds != literal.IsPositive)
                staticDerivedPreconditionsSatisfied = false;
        }

        _staticDerivedPreconditionsSatisfied =
            staticDerivedPreconditionsSatisfied;
        _stateDependentDerivedPreconditions =
            stateDependentDerivedPreconditions?.ToArray()
            ?? Array.Empty<Literal<Fact<Derived>>>();
    }

    private static Literal<Fact<Derived>>[] GetDerivedPreconditions(
        IReadOnlyList<Literal<Fact<Derived>>> derivedPreconditions)
    {
        ArgumentNullException.ThrowIfNull(derivedPreconditions);
        return derivedPreconditions.ToArray();
    }

    public bool AreStaticDerivedPreconditionsSatisfied()
        => _staticDerivedPreconditionsSatisfied;

    public bool AreStateDependentDerivedPreconditionsSatisfied(
        ExtendedState state)
    {
        for (int index = 0;
             index < _stateDependentDerivedPreconditions.Length;
             index++)
        {
            Literal<Fact<Derived>> literal =
                _stateDependentDerivedPreconditions[index];
            if (state.IsStateDependentDerivedTruePrevalidated(literal.Value)
                != literal.IsPositive)
            {
                return false;
            }
        }

        return true;
    }
}

internal sealed class ActionEffects :
    ReadOnlyCollection<GroundConditionalEffect>
{
    internal OffsetBitboard Add { get; }
    internal OffsetBitboard Delete { get; }
    public double Cost { get; }
    internal IReadOnlyList<GroundNumericUpdate> NumericEffects { get; }
    internal IReadOnlyList<GroundConditionalNumericEffect> ConditionalNumericEffects { get; }
    internal bool HasNumericEffects => NumericEffects.Count > 0 || ConditionalNumericEffects.Count > 0;

    public ActionEffects(
        OffsetBitboard add,
        OffsetBitboard delete,
        IReadOnlyList<GroundConditionalEffect> conditionalEffects,
        IReadOnlyList<GroundNumericUpdate> numericEffects,
        IReadOnlyList<GroundConditionalNumericEffect> conditionalNumericEffects,
        double cost)
        : base(GetConditionalEffects(conditionalEffects))
    {
        ArgumentNullException.ThrowIfNull(numericEffects);
        ArgumentNullException.ThrowIfNull(conditionalNumericEffects);

        Add = add;
        Delete = delete;
        Cost = cost;
        NumericEffects = Array.AsReadOnly(numericEffects.ToArray());
        ConditionalNumericEffects = Array.AsReadOnly(conditionalNumericEffects.ToArray());
    }

    private static GroundConditionalEffect[] GetConditionalEffects(
        IReadOnlyList<GroundConditionalEffect> conditionalEffects)
    {
        ArgumentNullException.ThrowIfNull(conditionalEffects);
        return conditionalEffects.ToArray();
    }
}

public class Action : IEquatable<Action>
{
    public GroundConjunctiveCondition Precondition => new(Context.Problem,
        GroundConditionLiterals.Read(Context, PositiveFluentPreconditions, NegativeFluentPreconditions,
            PositiveStaticPreconditions, NegativeStaticPreconditions, DerivedPreconditions), _preconditions.Comparisons);

    public GroundActionEffect Effect => new(GroundConditionLiterals.Read(Context,
        AddEffects, DeleteEffects, default, default, Array.Empty<Literal<Fact<Derived>>>()), _effects.NumericEffects);

    public NumericExpression CostExpression => Schema.CostExpression.Ground(Context.Problem,
        Schema.Parameters.Zip(Arguments).ToDictionary(pair => pair.First, pair => pair.Second));

    private readonly ActionBinding _binding;
    private readonly ActionPreconditions _preconditions;
    private readonly ActionEffects _effects;

    public InstanceContext Context => _binding.Context;
    public ActionSchema Schema => _binding.Schema;
    public IReadOnlyList<Constant> Arguments => _binding;
    // NaN when the cost is undefined (PDDL 2.1); such an action is never applicable.
    public double Cost => _effects.Cost;

    internal OffsetBitboard PositiveFluentPreconditions
        => _preconditions.PositiveFluent;
    internal OffsetBitboard NegativeFluentPreconditions
        => _preconditions.NegativeFluent;
    internal OffsetBitboard PositiveStaticPreconditions
        => _preconditions.PositiveStatic;
    internal OffsetBitboard NegativeStaticPreconditions
        => _preconditions.NegativeStatic;
    internal IReadOnlyList<GroundNumericComparison> NumericPreconditions => _preconditions.Comparisons;
    internal IReadOnlyList<GroundNumericUpdate> NumericEffects => _effects.NumericEffects;
    internal OffsetBitboard AddEffects => _effects.Add;
    internal OffsetBitboard DeleteEffects => _effects.Delete;
    internal ActionBinding BindingComponent => _binding;
    internal ActionPreconditions PreconditionsComponent => _preconditions;
    internal ActionEffects EffectsComponent => _effects;

    public IReadOnlyList<Literal<Fact<Derived>>> DerivedPreconditions
        => _preconditions;

    public IReadOnlyList<GroundConditionalEffect> ConditionalEffects
        => _effects;

    public IReadOnlyList<GroundConditionalNumericEffect> ConditionalNumericEffects
        => _effects.ConditionalNumericEffects;

    internal Action(
        InstanceContext context,
        ActionSchema schema,
        IReadOnlyList<Constant> arguments,
        OffsetBitboard positiveFluentPreconditions,
        OffsetBitboard negativeFluentPreconditions,
        OffsetBitboard positiveStaticPreconditions,
        OffsetBitboard negativeStaticPreconditions,
        OffsetBitboard addEffects,
        OffsetBitboard deleteEffects,
        IReadOnlyList<Literal<Fact<Derived>>> derivedPreconditions,
        IReadOnlyList<GroundConditionalEffect> conditionalEffects,
        double cost,
        bool takeArgumentOwnership = false)
    {
        _binding = new ActionBinding(
            context,
            schema,
            arguments,
            takeArgumentOwnership);
        _preconditions = new ActionPreconditions(
            context,
            positiveFluentPreconditions,
            negativeFluentPreconditions,
            positiveStaticPreconditions,
            negativeStaticPreconditions,
            derivedPreconditions);
        _effects = new ActionEffects(
            addEffects,
            deleteEffects,
            conditionalEffects,
            Array.Empty<GroundNumericUpdate>(),
            Array.Empty<GroundConditionalNumericEffect>(),
            cost);
    }

    internal Action(
        ActionBinding binding,
        ActionPreconditions preconditions,
        ActionEffects effects)
    {
        _binding = binding ?? throw new ArgumentNullException(nameof(binding));
        _preconditions = preconditions
            ?? throw new ArgumentNullException(nameof(preconditions));
        _effects = effects ?? throw new ArgumentNullException(nameof(effects));
    }

    public override string ToString()
    {
        if (Arguments.Count == 0) return $"({Schema.Name})";
        return $"({Schema.Name} {string.Join(" ", Arguments)})";
    }

    public bool Equals(Action? other)
    {
        if (ReferenceEquals(this, other))
            return true;
        return other is not null && _binding.Equals(other._binding);
    }

    public override bool Equals(object? obj) => Equals(obj as Action);
    public override int GetHashCode() => _binding.GetHashCode();

    public bool IsApplicable(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        State compactState = state.State;
        if (!ReferenceEquals(compactState.Context, Context))
            throw new InvalidOperationException(
                "Cannot check applicability for a state from a different InstanceContext.");

        if (!BitboardOps.StaticPreconditionHolds(
                PositiveStaticPreconditions,
                NegativeStaticPreconditions,
                compactState.Context.StaticBitboardWords))
            return false;

        if (!compactState.ContainsAll(PositiveFluentPreconditions))
            return false;

        if (!compactState.ContainsNone(NegativeFluentPreconditions))
            return false;

        if (!AreNumericPreconditionsSatisfied(compactState)) return false;
        return AreStaticDerivedPreconditionsSatisfied()
            && AreStateDependentDerivedPreconditionsSatisfied(state)
            && AreNumericEffectsDefined(state);
    }

    // Checked after the preconditions because triggered conditional effects are only meaningful in states
    // where the action's preconditions hold.
    internal bool AreNumericEffectsDefined(ExtendedState state)
    {
        if (double.IsNaN(Cost)) return false;
        if (!_effects.HasNumericEffects) return true;
        List<GroundConditionalNumericEffect>? triggered = null;
        foreach (GroundConditionalNumericEffect effect in _effects.ConditionalNumericEffects)
        {
            if (effect.IsSatisfied(state))
                (triggered ??= new List<GroundConditionalNumericEffect>()).Add(effect);
        }
        return NumericStateTransition.IsDefined(state.State, _effects.NumericEffects, triggered);
    }

    internal bool AreNumericPreconditionsSatisfied(State state)
    {
        foreach (GroundNumericComparison comparison in _preconditions.Comparisons)
            if (!state.Holds(comparison)) return false;
        return true;
    }

    internal bool AreStaticDerivedPreconditionsSatisfied()
        => _preconditions.AreStaticDerivedPreconditionsSatisfied();

    internal bool AreStateDependentDerivedPreconditionsSatisfied(
        ExtendedState state)
        => _preconditions.AreStateDependentDerivedPreconditionsSatisfied(state);
}
