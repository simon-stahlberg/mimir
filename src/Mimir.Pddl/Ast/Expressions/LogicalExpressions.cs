using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Ast.Expressions;

public record And(ImmutableArray<ILogicalExpression> Expressions) : ILogicalExpression
{
    private ImmutableArray<ILogicalExpression> _expressions = AstGuard.RequireArray(Expressions, nameof(Expressions));

    public ImmutableArray<ILogicalExpression> Expressions
    {
        get => _expressions;
        init => _expressions = AstGuard.RequireArray(value, nameof(Expressions));
    }

    public virtual bool Equals(And? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        AstSequence.Equals(Expressions, other.Expressions);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        AstSequence.AddHashCode(ref hash, Expressions);
        return hash.ToHashCode();
    }

    public string ToPddlString() => Expressions.IsDefaultOrEmpty ? "(and)" : $"(and {string.Join(" ", Expressions.Select(e => e.ToPddlString()))})";

    public override string ToString() => ToPddlString();
}

public record Or(ImmutableArray<ILogicalExpression> Expressions) : ILogicalExpression
{
    private ImmutableArray<ILogicalExpression> _expressions = AstGuard.RequireArray(Expressions, nameof(Expressions));

    public ImmutableArray<ILogicalExpression> Expressions
    {
        get => _expressions;
        init => _expressions = AstGuard.RequireArray(value, nameof(Expressions));
    }

    public virtual bool Equals(Or? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        AstSequence.Equals(Expressions, other.Expressions);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        AstSequence.AddHashCode(ref hash, Expressions);
        return hash.ToHashCode();
    }

    public string ToPddlString() => Expressions.IsDefaultOrEmpty ? "(or)" : $"(or {string.Join(" ", Expressions.Select(e => e.ToPddlString()))})";

    public override string ToString() => ToPddlString();
}

public record Not(ILogicalExpression Expression) : ILogicalExpression
{
    private ILogicalExpression _expression = AstGuard.RequireNotNull(Expression, nameof(Expression));

    public ILogicalExpression Expression
    {
        get => _expression;
        init => _expression = AstGuard.RequireNotNull(value, nameof(Expression));
    }

    public string ToPddlString() => $"(not {Expression.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Imply(ILogicalExpression Antecedent, ILogicalExpression Consequent) : ILogicalExpression
{
    private ILogicalExpression _antecedent = AstGuard.RequireNotNull(Antecedent, nameof(Antecedent));
    private ILogicalExpression _consequent = AstGuard.RequireNotNull(Consequent, nameof(Consequent));

    public ILogicalExpression Antecedent
    {
        get => _antecedent;
        init => _antecedent = AstGuard.RequireNotNull(value, nameof(Antecedent));
    }

    public ILogicalExpression Consequent
    {
        get => _consequent;
        init => _consequent = AstGuard.RequireNotNull(value, nameof(Consequent));
    }

    public string ToPddlString() => $"(imply {Antecedent.ToPddlString()} {Consequent.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Equality(Term Left, Term Right) : ILogicalExpression
{
    private Term _left = AstGuard.RequireNotNull(Left, nameof(Left));
    private Term _right = AstGuard.RequireNotNull(Right, nameof(Right));

    public Term Left
    {
        get => _left;
        init => _left = AstGuard.RequireNotNull(value, nameof(Left));
    }

    public Term Right
    {
        get => _right;
        init => _right = AstGuard.RequireNotNull(value, nameof(Right));
    }

    public string ToPddlString() => $"(= {Left.ToPddlString()} {Right.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record AmbiguousEquality(string LeftName, string RightName) : ILogicalExpression
{
    private string _leftName = PddlName.RequireName(LeftName, nameof(LeftName));
    private string _rightName = PddlName.RequireName(RightName, nameof(RightName));

    public string LeftName
    {
        get => _leftName;
        init => _leftName = PddlName.RequireName(value, nameof(LeftName));
    }

    public string RightName
    {
        get => _rightName;
        init => _rightName = PddlName.RequireName(value, nameof(RightName));
    }

    public string ToPddlString() => $"(= {LeftName} {RightName})";

    public override string ToString() => ToPddlString();
}

public record Forall(ImmutableArray<Parameter> Variables, ILogicalExpression Body) : ILogicalExpression
{
    private ImmutableArray<Parameter> _variables = AstGuard.RequireArray(Variables, nameof(Variables));
    private ILogicalExpression _body = AstGuard.RequireNotNull(Body, nameof(Body));

    public ImmutableArray<Parameter> Variables
    {
        get => _variables;
        init => _variables = AstGuard.RequireArray(value, nameof(Variables));
    }

    public ILogicalExpression Body
    {
        get => _body;
        init => _body = AstGuard.RequireNotNull(value, nameof(Body));
    }

    public virtual bool Equals(Forall? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        AstSequence.Equals(Variables, other.Variables) &&
        EqualityComparer<ILogicalExpression>.Default.Equals(Body, other.Body);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        AstSequence.AddHashCode(ref hash, Variables);
        hash.Add(Body);
        return hash.ToHashCode();
    }

    public string ToPddlString() => $"(forall ({TypedListSerializer.Serialize(Variables)}) {Body.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record Exists(ImmutableArray<Parameter> Variables, ILogicalExpression Body) : ILogicalExpression
{
    private ImmutableArray<Parameter> _variables = AstGuard.RequireArray(Variables, nameof(Variables));
    private ILogicalExpression _body = AstGuard.RequireNotNull(Body, nameof(Body));

    public ImmutableArray<Parameter> Variables
    {
        get => _variables;
        init => _variables = AstGuard.RequireArray(value, nameof(Variables));
    }

    public ILogicalExpression Body
    {
        get => _body;
        init => _body = AstGuard.RequireNotNull(value, nameof(Body));
    }

    public virtual bool Equals(Exists? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        AstSequence.Equals(Variables, other.Variables) &&
        EqualityComparer<ILogicalExpression>.Default.Equals(Body, other.Body);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        AstSequence.AddHashCode(ref hash, Variables);
        hash.Add(Body);
        return hash.ToHashCode();
    }

    public string ToPddlString() => $"(exists ({TypedListSerializer.Serialize(Variables)}) {Body.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public enum ComparisonOperator
{
    Equal,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual
}

public record Comparison(ComparisonOperator Operator, INumericExpression Left, INumericExpression Right) : ILogicalExpression
{
    private ComparisonOperator _operator = AstGuard.RequireDefined(Operator, nameof(Operator));
    private INumericExpression _left = AstGuard.RequireNotNull(Left, nameof(Left));
    private INumericExpression _right = AstGuard.RequireNotNull(Right, nameof(Right));

    public ComparisonOperator Operator
    {
        get => _operator;
        init => _operator = AstGuard.RequireDefined(value, nameof(Operator));
    }

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

    public string ToPddlString() => $"({ToPddlString(Operator)} {Left.ToPddlString()} {Right.ToPddlString()})";

    public override string ToString() => ToPddlString();

    private static string ToPddlString(ComparisonOperator comparisonOperator) => comparisonOperator switch
    {
        ComparisonOperator.Equal => "=",
        ComparisonOperator.LessThan => "<",
        ComparisonOperator.LessThanOrEqual => "<=",
        ComparisonOperator.GreaterThan => ">",
        ComparisonOperator.GreaterThanOrEqual => ">=",
        _ => throw new ArgumentOutOfRangeException(nameof(comparisonOperator), comparisonOperator, null)
    };
}

public record EmptyLogic() : ILogicalExpression
{
    public string ToPddlString() => "()";

    public override string ToString() => ToPddlString();
}
