using System.Collections.Immutable;
using Mimir.Core;

namespace Mimir.Core.Grounding;

using Schemas;

/// <summary>
/// A logical expression over the grounded planning model, expressed
/// entirely in Core types. Used for derived predicate bodies and goal
/// conditions without exposing any PDDL AST types in the public API.
/// </summary>
public interface IGroundedExpression
{
}

/// <summary>Always-true sentinel. Replaces EmptyLogic from the PDDL AST.</summary>
public sealed record GroundedTrue : IGroundedExpression;

/// <summary>A single predicate application with schema-level terms (variables or constants).</summary>
public sealed record GroundedAtom(
    Predicate Predicate,
    IReadOnlyList<ITerm> Arguments) : IGroundedExpression
{
    private Predicate _predicate = Predicate ?? throw new ArgumentNullException(nameof(Predicate));
    private IReadOnlyList<ITerm> _arguments = CopyArguments(Arguments, nameof(Arguments));

    public Predicate Predicate
    {
        get => _predicate;
        init => _predicate = value ?? throw new ArgumentNullException(nameof(Predicate));
    }

    public IReadOnlyList<ITerm> Arguments
    {
        get => _arguments;
        init => _arguments = CopyArguments(value, nameof(Arguments));
    }

    private static IReadOnlyList<ITerm> CopyArguments(
        IReadOnlyList<ITerm> arguments,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(arguments, parameterName);

        ITerm[] copy = arguments.ToArray();
        if (copy.Any(argument => argument is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return Array.AsReadOnly(copy);
    }

    public bool Equals(GroundedAtom? other)
        => other is not null
        && ReferenceEquals(Predicate, other.Predicate)
        && ValueSequence.ReferenceEqualsItems(Arguments, other.Arguments);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(Predicate));
        ValueSequence.AddReferencesToHash(ref hash, Arguments);
        return hash.ToHashCode();
    }
}

public sealed record GroundedNot(IGroundedExpression Expression) : IGroundedExpression
{
    private IGroundedExpression _expression =
        Expression ?? throw new ArgumentNullException(nameof(Expression));

    public IGroundedExpression Expression
    {
        get => _expression;
        init => _expression = value ?? throw new ArgumentNullException(nameof(Expression));
    }
}

public sealed record GroundedAnd(
    ImmutableArray<IGroundedExpression> Expressions) : IGroundedExpression
{
    private ImmutableArray<IGroundedExpression> _expressions =
        ValidateExpressions(Expressions, nameof(Expressions));

    public ImmutableArray<IGroundedExpression> Expressions
    {
        get => _expressions;
        init => _expressions = ValidateExpressions(value, nameof(Expressions));
    }

    private static ImmutableArray<IGroundedExpression> ValidateExpressions(
        ImmutableArray<IGroundedExpression> expressions,
        string parameterName)
    {
        if (expressions.IsDefault)
            throw new ArgumentException("The collection must be initialized.", parameterName);
        if (expressions.Any(expression => expression is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return expressions;
    }

    public bool Equals(GroundedAnd? other)
        => other is not null && ValueSequence.Equals(Expressions, other.Expressions);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        ValueSequence.AddToHash(ref hash, Expressions);
        return hash.ToHashCode();
    }
}

public sealed record GroundedOr(
    ImmutableArray<IGroundedExpression> Expressions) : IGroundedExpression
{
    private ImmutableArray<IGroundedExpression> _expressions =
        ValidateExpressions(Expressions, nameof(Expressions));

    public ImmutableArray<IGroundedExpression> Expressions
    {
        get => _expressions;
        init => _expressions = ValidateExpressions(value, nameof(Expressions));
    }

    private static ImmutableArray<IGroundedExpression> ValidateExpressions(
        ImmutableArray<IGroundedExpression> expressions,
        string parameterName)
    {
        if (expressions.IsDefault)
            throw new ArgumentException("The collection must be initialized.", parameterName);
        if (expressions.Any(expression => expression is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return expressions;
    }

    public bool Equals(GroundedOr? other)
        => other is not null && ValueSequence.Equals(Expressions, other.Expressions);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        ValueSequence.AddToHash(ref hash, Expressions);
        return hash.ToHashCode();
    }
}

public sealed record GroundedImply(
    IGroundedExpression Antecedent,
    IGroundedExpression Consequent) : IGroundedExpression
{
    private IGroundedExpression _antecedent =
        Antecedent ?? throw new ArgumentNullException(nameof(Antecedent));
    private IGroundedExpression _consequent =
        Consequent ?? throw new ArgumentNullException(nameof(Consequent));

    public IGroundedExpression Antecedent
    {
        get => _antecedent;
        init => _antecedent = value ?? throw new ArgumentNullException(nameof(Antecedent));
    }

    public IGroundedExpression Consequent
    {
        get => _consequent;
        init => _consequent = value ?? throw new ArgumentNullException(nameof(Consequent));
    }
}

/// <summary>Universal quantifier; variables carry their type for candidate filtering.</summary>
public sealed record GroundedForall(
    IReadOnlyList<Variable> Variables,
    IGroundedExpression Body) : IGroundedExpression
{
    private IReadOnlyList<Variable> _variables = CopyVariables(Variables, nameof(Variables));
    private IGroundedExpression _body = Body ?? throw new ArgumentNullException(nameof(Body));

    public IReadOnlyList<Variable> Variables
    {
        get => _variables;
        init => _variables = CopyVariables(value, nameof(Variables));
    }

    public IGroundedExpression Body
    {
        get => _body;
        init => _body = value ?? throw new ArgumentNullException(nameof(Body));
    }

    private static IReadOnlyList<Variable> CopyVariables(
        IReadOnlyList<Variable> variables,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(variables, parameterName);

        Variable[] copy = variables.ToArray();
        if (copy.Any(variable => variable is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return Array.AsReadOnly(copy);
    }

    public bool Equals(GroundedForall? other)
        => other is not null
        && ValueSequence.ReferenceEqualsItems(Variables, other.Variables)
        && EqualityComparer<IGroundedExpression>.Default.Equals(Body, other.Body);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        ValueSequence.AddReferencesToHash(ref hash, Variables);
        hash.Add(Body);
        return hash.ToHashCode();
    }
}

/// <summary>Existential quantifier; variables carry their type for candidate filtering.</summary>
public sealed record GroundedExists(
    IReadOnlyList<Variable> Variables,
    IGroundedExpression Body) : IGroundedExpression
{
    private IReadOnlyList<Variable> _variables = CopyVariables(Variables, nameof(Variables));
    private IGroundedExpression _body = Body ?? throw new ArgumentNullException(nameof(Body));

    public IReadOnlyList<Variable> Variables
    {
        get => _variables;
        init => _variables = CopyVariables(value, nameof(Variables));
    }

    public IGroundedExpression Body
    {
        get => _body;
        init => _body = value ?? throw new ArgumentNullException(nameof(Body));
    }

    private static IReadOnlyList<Variable> CopyVariables(
        IReadOnlyList<Variable> variables,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(variables, parameterName);

        Variable[] copy = variables.ToArray();
        if (copy.Any(variable => variable is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);

        return Array.AsReadOnly(copy);
    }

    public bool Equals(GroundedExists? other)
        => other is not null
        && ValueSequence.ReferenceEqualsItems(Variables, other.Variables)
        && EqualityComparer<IGroundedExpression>.Default.Equals(Body, other.Body);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        ValueSequence.AddReferencesToHash(ref hash, Variables);
        hash.Add(Body);
        return hash.ToHashCode();
    }
}
