using System.Collections.Immutable;
using System.Linq;
using System.Globalization;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Ast.Expressions;

public record FluentCall(string Name, ImmutableArray<Term> Arguments) : INumericExpression
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private ImmutableArray<Term> _arguments = AstGuard.RequireArray(Arguments, nameof(Arguments));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public ImmutableArray<Term> Arguments
    {
        get => _arguments;
        init => _arguments = AstGuard.RequireArray(value, nameof(Arguments));
    }

    public virtual bool Equals(FluentCall? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        Name == other.Name &&
        AstSequence.Equals(Arguments, other.Arguments);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        hash.Add(Name, StringComparer.Ordinal);
        AstSequence.AddHashCode(ref hash, Arguments);
        return hash.ToHashCode();
    }

    public string ToPddlString()
    {
        if (Arguments.IsDefaultOrEmpty)
            return $"({Name})";
        return $"({Name} {string.Join(" ", Arguments.Select(a => a.ToPddlString()))})";
    }

    public override string ToString() => ToPddlString();
}

public record NumberLiteral(decimal Value) : INumericExpression
{
    public string ToPddlString() => Value.ToString(CultureInfo.InvariantCulture);

    public override string ToString() => ToPddlString();
}

public record Add(INumericExpression Left, INumericExpression Right) : INumericExpression
{
    private INumericExpression _left = AstGuard.RequireNotNull(Left, nameof(Left));
    private INumericExpression _right = AstGuard.RequireNotNull(Right, nameof(Right));

    public INumericExpression Left
    {
        get => _left;
        init => _left = AstGuard.RequireNotNull(value, nameof(Left));
    }

    public INumericExpression Right
    {
        get => _right;
        init => _right = AstGuard.RequireNotNull(value, nameof(Right));
    }

    public string ToPddlString() => $"(+ {Left.ToPddlString()} {Right.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Negate(INumericExpression Operand) : INumericExpression
{
    private INumericExpression _operand = AstGuard.RequireNotNull(Operand, nameof(Operand));

    public INumericExpression Operand
    {
        get => _operand;
        init => _operand = AstGuard.RequireNotNull(value, nameof(Operand));
    }

    public string ToPddlString() => $"(- {Operand.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Subtract(INumericExpression Left, INumericExpression Right) : INumericExpression
{
    private INumericExpression _left = AstGuard.RequireNotNull(Left, nameof(Left));
    private INumericExpression _right = AstGuard.RequireNotNull(Right, nameof(Right));

    public INumericExpression Left
    {
        get => _left;
        init => _left = AstGuard.RequireNotNull(value, nameof(Left));
    }

    public INumericExpression Right
    {
        get => _right;
        init => _right = AstGuard.RequireNotNull(value, nameof(Right));
    }

    public string ToPddlString() => $"(- {Left.ToPddlString()} {Right.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Multiply(INumericExpression Left, INumericExpression Right) : INumericExpression
{
    private INumericExpression _left = AstGuard.RequireNotNull(Left, nameof(Left));
    private INumericExpression _right = AstGuard.RequireNotNull(Right, nameof(Right));

    public INumericExpression Left
    {
        get => _left;
        init => _left = AstGuard.RequireNotNull(value, nameof(Left));
    }

    public INumericExpression Right
    {
        get => _right;
        init => _right = AstGuard.RequireNotNull(value, nameof(Right));
    }

    public string ToPddlString() => $"(* {Left.ToPddlString()} {Right.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Divide(INumericExpression Left, INumericExpression Right) : INumericExpression
{
    private INumericExpression _left = AstGuard.RequireNotNull(Left, nameof(Left));
    private INumericExpression _right = AstGuard.RequireNotNull(Right, nameof(Right));

    public INumericExpression Left
    {
        get => _left;
        init => _left = AstGuard.RequireNotNull(value, nameof(Left));
    }

    public INumericExpression Right
    {
        get => _right;
        init => _right = AstGuard.RequireNotNull(value, nameof(Right));
    }

    public string ToPddlString() => $"(/ {Left.ToPddlString()} {Right.ToPddlString()})";

    public override string ToString() => ToPddlString();
}
