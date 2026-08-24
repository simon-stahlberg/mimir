using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Ast.Expressions;

public record PredicateCall(string Name, ImmutableArray<Term> Arguments) : ILogicalExpression, IProblemInitElement
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

    public virtual bool Equals(PredicateCall? other) =>
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
