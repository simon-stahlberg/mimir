using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Ast.Effects;

public record AddEffect(PredicateCall Predicate) : IEffect
{
    private PredicateCall _predicate = AstGuard.RequireNotNull(Predicate, nameof(Predicate));

    public PredicateCall Predicate
    {
        get => _predicate;
        init => _predicate = AstGuard.RequireNotNull(value, nameof(Predicate));
    }

    public string ToPddlString() => Predicate.ToPddlString();

    public override string ToString() => ToPddlString();
}

public record DeleteEffect(PredicateCall Predicate) : IEffect
{
    private PredicateCall _predicate = AstGuard.RequireNotNull(Predicate, nameof(Predicate));

    public PredicateCall Predicate
    {
        get => _predicate;
        init => _predicate = AstGuard.RequireNotNull(value, nameof(Predicate));
    }

    public string ToPddlString() => $"(not {Predicate.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Assign(FluentCall Fluent, INumericExpression Value) : IEffect
{
    private FluentCall _fluent = AstGuard.RequireNotNull(Fluent, nameof(Fluent));
    private INumericExpression _value = AstGuard.RequireNotNull(Value, nameof(Value));

    public FluentCall Fluent
    {
        get => _fluent;
        init => _fluent = AstGuard.RequireNotNull(value, nameof(Fluent));
    }

    public INumericExpression Value
    {
        get => _value;
        init => _value = AstGuard.RequireNotNull(value, nameof(Value));
    }

    public string ToPddlString() => $"(assign {Fluent.ToPddlString()} {Value.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Increase(FluentCall Fluent, INumericExpression Value) : IEffect
{
    private FluentCall _fluent = AstGuard.RequireNotNull(Fluent, nameof(Fluent));
    private INumericExpression _value = AstGuard.RequireNotNull(Value, nameof(Value));

    public FluentCall Fluent
    {
        get => _fluent;
        init => _fluent = AstGuard.RequireNotNull(value, nameof(Fluent));
    }

    public INumericExpression Value
    {
        get => _value;
        init => _value = AstGuard.RequireNotNull(value, nameof(Value));
    }

    public string ToPddlString() => $"(increase {Fluent.ToPddlString()} {Value.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Decrease(FluentCall Fluent, INumericExpression Value) : IEffect
{
    private FluentCall _fluent = AstGuard.RequireNotNull(Fluent, nameof(Fluent));
    private INumericExpression _value = AstGuard.RequireNotNull(Value, nameof(Value));

    public FluentCall Fluent
    {
        get => _fluent;
        init => _fluent = AstGuard.RequireNotNull(value, nameof(Fluent));
    }

    public INumericExpression Value
    {
        get => _value;
        init => _value = AstGuard.RequireNotNull(value, nameof(Value));
    }

    public string ToPddlString() => $"(decrease {Fluent.ToPddlString()} {Value.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record ScaleUp(FluentCall Fluent, INumericExpression Value) : IEffect
{
    private FluentCall _fluent = AstGuard.RequireNotNull(Fluent, nameof(Fluent));
    private INumericExpression _value = AstGuard.RequireNotNull(Value, nameof(Value));

    public FluentCall Fluent
    {
        get => _fluent;
        init => _fluent = AstGuard.RequireNotNull(value, nameof(Fluent));
    }

    public INumericExpression Value
    {
        get => _value;
        init => _value = AstGuard.RequireNotNull(value, nameof(Value));
    }

    public string ToPddlString() => $"(scale-up {Fluent.ToPddlString()} {Value.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record ScaleDown(FluentCall Fluent, INumericExpression Value) : IEffect
{
    private FluentCall _fluent = AstGuard.RequireNotNull(Fluent, nameof(Fluent));
    private INumericExpression _value = AstGuard.RequireNotNull(Value, nameof(Value));

    public FluentCall Fluent
    {
        get => _fluent;
        init => _fluent = AstGuard.RequireNotNull(value, nameof(Fluent));
    }

    public INumericExpression Value
    {
        get => _value;
        init => _value = AstGuard.RequireNotNull(value, nameof(Value));
    }

    public string ToPddlString() => $"(scale-down {Fluent.ToPddlString()} {Value.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record ConditionalEffect(ILogicalExpression Condition, IEffect Effect) : IEffect
{
    private ILogicalExpression _condition = AstGuard.RequireNotNull(Condition, nameof(Condition));
    private IEffect _effect = AstGuard.RequireNotNull(Effect, nameof(Effect));

    public ILogicalExpression Condition
    {
        get => _condition;
        init => _condition = AstGuard.RequireNotNull(value, nameof(Condition));
    }

    public IEffect Effect
    {
        get => _effect;
        init => _effect = AstGuard.RequireNotNull(value, nameof(Effect));
    }

    public string ToPddlString() => $"(when {Condition.ToPddlString()} {Effect.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record AndEffect(ImmutableArray<IEffect> Effects) : IEffect
{
    private ImmutableArray<IEffect> _effects = AstGuard.RequireArray(Effects, nameof(Effects));

    public ImmutableArray<IEffect> Effects
    {
        get => _effects;
        init => _effects = AstGuard.RequireArray(value, nameof(Effects));
    }

    public virtual bool Equals(AndEffect? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        AstSequence.Equals(Effects, other.Effects);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        AstSequence.AddHashCode(ref hash, Effects);
        return hash.ToHashCode();
    }

    public string ToPddlString() => Effects.IsDefaultOrEmpty ? "(and)" : $"(and {string.Join(" ", Effects.Select(e => e.ToPddlString()))})";

    public override string ToString() => ToPddlString();
}

public record ForallEffect(ImmutableArray<Parameter> Variables, IEffect Effect) : IEffect
{
    private ImmutableArray<Parameter> _variables = AstGuard.RequireArray(Variables, nameof(Variables));
    private IEffect _effect = AstGuard.RequireNotNull(Effect, nameof(Effect));

    public ImmutableArray<Parameter> Variables
    {
        get => _variables;
        init => _variables = AstGuard.RequireArray(value, nameof(Variables));
    }

    public IEffect Effect
    {
        get => _effect;
        init => _effect = AstGuard.RequireNotNull(value, nameof(Effect));
    }

    public virtual bool Equals(ForallEffect? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        AstSequence.Equals(Variables, other.Variables) &&
        EqualityComparer<IEffect>.Default.Equals(Effect, other.Effect);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        AstSequence.AddHashCode(ref hash, Variables);
        hash.Add(Effect);
        return hash.ToHashCode();
    }

    public string ToPddlString() => $"(forall ({TypedListSerializer.Serialize(Variables)}) {Effect.ToPddlString()})";

    public override string ToString() => ToPddlString();
}
