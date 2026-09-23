using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Pddl.Ast.Models;

public record NumericInitialization(FluentCall Fluent, NumberLiteral Value) : IProblemInitElement
{
    private FluentCall _fluent = AstGuard.RequireNotNull(Fluent, nameof(Fluent));
    private NumberLiteral _value = AstGuard.RequireNotNull(Value, nameof(Value));

    public FluentCall Fluent
    {
        get => _fluent;
        init => _fluent = AstGuard.RequireNotNull(value, nameof(Fluent));
    }

    public NumberLiteral Value
    {
        get => _value;
        init => _value = AstGuard.RequireNotNull(value, nameof(Value));
    }

    public string ToPddlString() => $"(= {Fluent.ToPddlString()} {Value.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record NegativePredicateInitialization(PredicateCall Predicate) : IProblemInitElement
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
