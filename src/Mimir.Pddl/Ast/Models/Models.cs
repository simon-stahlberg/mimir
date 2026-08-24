using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Pddl.Ast.Models;

public enum PddlRequirement
{
    Strips,
    Typing,
    Equality,
    NegativePreconditions,
    DisjunctivePreconditions,
    NumericFluents,
    ConditionalEffects,
    ExistentialPreconditions,
    UniversalPreconditions,
    Adl,
    DerivedPredicates,
    ActionCosts
}

public static class PddlRequirementExtensions
{
    public static bool HasRequirement(
        this IEnumerable<PddlRequirement> requirements,
        PddlRequirement requirement)
    {
        ArgumentNullException.ThrowIfNull(requirements);

        bool hasAnyRequirement = false;
        bool hasAdl = false;
        foreach (PddlRequirement declaredRequirement in requirements)
        {
            hasAnyRequirement = true;
            if (declaredRequirement == requirement)
                return true;

            hasAdl |= declaredRequirement == PddlRequirement.Adl;
        }

        if (!hasAnyRequirement && requirement == PddlRequirement.Strips)
            return true;

        return hasAdl && requirement is
            PddlRequirement.Strips or
            PddlRequirement.Typing or
            PddlRequirement.Equality or
            PddlRequirement.NegativePreconditions or
            PddlRequirement.DisjunctivePreconditions or
            PddlRequirement.ConditionalEffects or
            PddlRequirement.ExistentialPreconditions or
            PddlRequirement.UniversalPreconditions;
    }

    public static string ToPddlString(this PddlRequirement requirement) => requirement switch
    {
        PddlRequirement.Strips => ":strips",
        PddlRequirement.Typing => ":typing",
        PddlRequirement.Equality => ":equality",
        PddlRequirement.NegativePreconditions => ":negative-preconditions",
        PddlRequirement.DisjunctivePreconditions => ":disjunctive-preconditions",
        PddlRequirement.NumericFluents => ":numeric-fluents",
        PddlRequirement.ConditionalEffects => ":conditional-effects",
        PddlRequirement.ExistentialPreconditions => ":existential-preconditions",
        PddlRequirement.UniversalPreconditions => ":universal-preconditions",
        PddlRequirement.Adl => ":adl",
        PddlRequirement.DerivedPredicates => ":derived-predicates",
        PddlRequirement.ActionCosts => ":action-costs",
        _ => throw new ArgumentOutOfRangeException(nameof(requirement), requirement, null)
    };
}

public record PredicateDeclaration(string Name, ImmutableArray<Parameter> Parameters) : INode
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private ImmutableArray<Parameter> _parameters = AstGuard.RequireArray(Parameters, nameof(Parameters));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public ImmutableArray<Parameter> Parameters
    {
        get => _parameters;
        init => _parameters = AstGuard.RequireArray(value, nameof(Parameters));
    }

    public virtual bool Equals(PredicateDeclaration? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        Name == other.Name &&
        AstSequence.Equals(Parameters, other.Parameters);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        hash.Add(Name, StringComparer.Ordinal);
        AstSequence.AddHashCode(ref hash, Parameters);
        return hash.ToHashCode();
    }

    public string ToPddlString() => Parameters.IsDefaultOrEmpty
        ? $"({Name})"
        : $"({Name} {TypedListSerializer.Serialize(Parameters)})";

    public override string ToString() => ToPddlString();
}

public record FunctionDeclaration(string Name, ImmutableArray<Parameter> Parameters) : INode
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private ImmutableArray<Parameter> _parameters = AstGuard.RequireArray(Parameters, nameof(Parameters));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public ImmutableArray<Parameter> Parameters
    {
        get => _parameters;
        init => _parameters = AstGuard.RequireArray(value, nameof(Parameters));
    }

    public virtual bool Equals(FunctionDeclaration? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        Name == other.Name &&
        AstSequence.Equals(Parameters, other.Parameters);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        hash.Add(Name, StringComparer.Ordinal);
        AstSequence.AddHashCode(ref hash, Parameters);
        return hash.ToHashCode();
    }

    public string ToPddlString() => Parameters.IsDefaultOrEmpty
        ? $"({Name})"
        : $"({Name} {TypedListSerializer.Serialize(Parameters)})";

    public override string ToString() => ToPddlString();
}

public record TypeDeclaration(string Name, string ParentType = "object") : INode
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private string _parentType = PddlName.RequireName(ParentType, nameof(ParentType));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public string ParentType
    {
        get => _parentType;
        init => _parentType = PddlName.RequireName(value, nameof(ParentType));
    }

    public string ToPddlString() => ParentType == "object" ? Name : $"{Name} - {ParentType}";

    public override string ToString() => ToPddlString();
}

public record ActionDefinition(
    string Name,
    ImmutableArray<Parameter> Parameters,
    ILogicalExpression? Precondition,
    IEffect? Effect) : INode
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private ImmutableArray<Parameter> _parameters = AstGuard.RequireArray(Parameters, nameof(Parameters));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public ImmutableArray<Parameter> Parameters
    {
        get => _parameters;
        init => _parameters = AstGuard.RequireArray(value, nameof(Parameters));
    }

    public virtual bool Equals(ActionDefinition? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        Name == other.Name &&
        AstSequence.Equals(Parameters, other.Parameters) &&
        EqualityComparer<ILogicalExpression?>.Default.Equals(Precondition, other.Precondition) &&
        EqualityComparer<IEffect?>.Default.Equals(Effect, other.Effect);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        hash.Add(Name, StringComparer.Ordinal);
        AstSequence.AddHashCode(ref hash, Parameters);
        hash.Add(Precondition);
        hash.Add(Effect);
        return hash.ToHashCode();
    }

    public string ToPddlString()
    {
        var builder = new System.Text.StringBuilder();
        builder.Append($"(:action {Name}");
        if (!Parameters.IsDefaultOrEmpty)
            builder.Append($" :parameters ({TypedListSerializer.Serialize(Parameters)})");
        else
            builder.Append(" :parameters ()");

        if (Precondition != null)
            builder.Append($" :precondition {Precondition.ToPddlString()}");

        if (Effect != null)
            builder.Append($" :effect {Effect.ToPddlString()}");

        builder.Append(")");
        return builder.ToString();
    }

    public override string ToString() => ToPddlString();
}

public record DerivedPredicate(
    PredicateDeclaration Signature,
    ILogicalExpression Body) : INode
{
    private PredicateDeclaration _signature = AstGuard.RequireNotNull(Signature, nameof(Signature));
    private ILogicalExpression _body = AstGuard.RequireNotNull(Body, nameof(Body));

    public PredicateDeclaration Signature
    {
        get => _signature;
        init => _signature = AstGuard.RequireNotNull(value, nameof(Signature));
    }

    public ILogicalExpression Body
    {
        get => _body;
        init => _body = AstGuard.RequireNotNull(value, nameof(Body));
    }

    public string ToPddlString() => $"(:derived {Signature.ToPddlString()}\n    {Body.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record DomainDefinition(
    string Name,
    ImmutableArray<PddlRequirement> Requirements,
    ImmutableArray<TypeDeclaration> Types,
    ImmutableArray<TypeDeclaration> Constants,
    ImmutableArray<PredicateDeclaration> Predicates,
    ImmutableArray<FunctionDeclaration> Functions,
    ImmutableArray<ActionDefinition> Actions,
    ImmutableArray<DerivedPredicate> DerivedPredicates) : INode
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private ImmutableArray<PddlRequirement> _requirements =
        AstGuard.RequireDefinedArray(Requirements, nameof(Requirements));
    private ImmutableArray<TypeDeclaration> _types = AstGuard.RequireArray(Types, nameof(Types));
    private ImmutableArray<TypeDeclaration> _constants = AstGuard.RequireArray(Constants, nameof(Constants));
    private ImmutableArray<PredicateDeclaration> _predicates = AstGuard.RequireArray(Predicates, nameof(Predicates));
    private ImmutableArray<FunctionDeclaration> _functions = AstGuard.RequireArray(Functions, nameof(Functions));
    private ImmutableArray<ActionDefinition> _actions = AstGuard.RequireArray(Actions, nameof(Actions));
    private ImmutableArray<DerivedPredicate> _derivedPredicates =
        AstGuard.RequireArray(DerivedPredicates, nameof(DerivedPredicates));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public ImmutableArray<PddlRequirement> Requirements
    {
        get => _requirements;
        init => _requirements = AstGuard.RequireDefinedArray(value, nameof(Requirements));
    }

    public ImmutableArray<TypeDeclaration> Types
    {
        get => _types;
        init => _types = AstGuard.RequireArray(value, nameof(Types));
    }

    public ImmutableArray<TypeDeclaration> Constants
    {
        get => _constants;
        init => _constants = AstGuard.RequireArray(value, nameof(Constants));
    }

    public ImmutableArray<PredicateDeclaration> Predicates
    {
        get => _predicates;
        init => _predicates = AstGuard.RequireArray(value, nameof(Predicates));
    }

    public ImmutableArray<FunctionDeclaration> Functions
    {
        get => _functions;
        init => _functions = AstGuard.RequireArray(value, nameof(Functions));
    }

    public ImmutableArray<ActionDefinition> Actions
    {
        get => _actions;
        init => _actions = AstGuard.RequireArray(value, nameof(Actions));
    }

    public ImmutableArray<DerivedPredicate> DerivedPredicates
    {
        get => _derivedPredicates;
        init => _derivedPredicates = AstGuard.RequireArray(value, nameof(DerivedPredicates));
    }

    public virtual bool Equals(DomainDefinition? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        Name == other.Name &&
        AstSequence.Equals(Requirements, other.Requirements) &&
        AstSequence.Equals(Types, other.Types) &&
        AstSequence.Equals(Constants, other.Constants) &&
        AstSequence.Equals(Predicates, other.Predicates) &&
        AstSequence.Equals(Functions, other.Functions) &&
        AstSequence.Equals(Actions, other.Actions) &&
        AstSequence.Equals(DerivedPredicates, other.DerivedPredicates);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        hash.Add(Name, StringComparer.Ordinal);
        AstSequence.AddHashCode(ref hash, Requirements);
        AstSequence.AddHashCode(ref hash, Types);
        AstSequence.AddHashCode(ref hash, Constants);
        AstSequence.AddHashCode(ref hash, Predicates);
        AstSequence.AddHashCode(ref hash, Functions);
        AstSequence.AddHashCode(ref hash, Actions);
        AstSequence.AddHashCode(ref hash, DerivedPredicates);
        return hash.ToHashCode();
    }

    public string ToPddlString()
    {
        var builder = new System.Text.StringBuilder();
        builder.AppendLine($"(define (domain {Name})");

        if (!Requirements.IsDefaultOrEmpty)
            builder.AppendLine($"  (:requirements {string.Join(" ", Requirements.Select(r => r.ToPddlString()))})");

        if (!Types.IsDefaultOrEmpty)
            builder.AppendLine($"  (:types {TypedListSerializer.Serialize(Types)})");

        if (!Constants.IsDefaultOrEmpty)
            builder.AppendLine($"  (:constants {TypedListSerializer.Serialize(Constants)})");

        if (!Predicates.IsDefaultOrEmpty)
        {
            builder.AppendLine("  (:predicates");
            foreach(var p in Predicates) builder.AppendLine($"    {p.ToPddlString()}");
            builder.AppendLine("  )");
        }

        if (!Functions.IsDefaultOrEmpty)
        {
            builder.AppendLine("  (:functions");
            foreach(var f in Functions) builder.AppendLine($"    {f.ToPddlString()}");
            builder.AppendLine("  )");
        }

        foreach(var a in Actions)
            builder.AppendLine($"  {a.ToPddlString()}");

        foreach(var d in DerivedPredicates)
            builder.AppendLine($"  {d.ToPddlString()}");

        builder.AppendLine(")");
        return builder.ToString();
    }

    public override string ToString() => ToPddlString();
}

public enum MetricDirection { Minimize, Maximize }

public record Metric(MetricDirection Direction, INumericExpression Expression) : INode
{
    private MetricDirection _direction = AstGuard.RequireDefined(Direction, nameof(Direction));
    private INumericExpression _expression = AstGuard.RequireNotNull(Expression, nameof(Expression));

    public MetricDirection Direction
    {
        get => _direction;
        init => _direction = AstGuard.RequireDefined(value, nameof(Direction));
    }

    public INumericExpression Expression
    {
        get => _expression;
        init => _expression = AstGuard.RequireNotNull(value, nameof(Expression));
    }

    public string ToPddlString() => $"(:metric {Direction.ToString().ToLowerInvariant()} {Expression.ToPddlString()})";

    public override string ToString() => ToPddlString();
}

public record ProblemDefinition(
    string Name,
    string DomainName,
    ImmutableArray<PddlRequirement> Requirements,
    ImmutableArray<TypeDeclaration> Objects,
    ImmutableArray<IProblemInitElement> Init,
    ILogicalExpression Goal,
    Metric? Metric) : INode
{
    private string _name = PddlName.RequireName(Name, nameof(Name));
    private string _domainName = PddlName.RequireName(DomainName, nameof(DomainName));
    private ImmutableArray<PddlRequirement> _requirements =
        AstGuard.RequireDefinedArray(Requirements, nameof(Requirements));
    private ImmutableArray<TypeDeclaration> _objects = AstGuard.RequireArray(Objects, nameof(Objects));
    private ImmutableArray<IProblemInitElement> _init = AstGuard.RequireArray(Init, nameof(Init));
    private ILogicalExpression _goal = AstGuard.RequireNotNull(Goal, nameof(Goal));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireName(value, nameof(Name));
    }

    public string DomainName
    {
        get => _domainName;
        init => _domainName = PddlName.RequireName(value, nameof(DomainName));
    }

    public ImmutableArray<PddlRequirement> Requirements
    {
        get => _requirements;
        init => _requirements = AstGuard.RequireDefinedArray(value, nameof(Requirements));
    }

    public ImmutableArray<TypeDeclaration> Objects
    {
        get => _objects;
        init => _objects = AstGuard.RequireArray(value, nameof(Objects));
    }

    public ImmutableArray<IProblemInitElement> Init
    {
        get => _init;
        init => _init = AstGuard.RequireArray(value, nameof(Init));
    }

    public ILogicalExpression Goal
    {
        get => _goal;
        init => _goal = AstGuard.RequireNotNull(value, nameof(Goal));
    }

    public virtual bool Equals(ProblemDefinition? other) =>
        other is not null &&
        EqualityContract == other.EqualityContract &&
        Name == other.Name &&
        DomainName == other.DomainName &&
        AstSequence.Equals(Requirements, other.Requirements) &&
        AstSequence.Equals(Objects, other.Objects) &&
        AstSequence.Equals(Init, other.Init) &&
        EqualityComparer<ILogicalExpression>.Default.Equals(Goal, other.Goal) &&
        EqualityComparer<Metric?>.Default.Equals(Metric, other.Metric);

    public override int GetHashCode()
    {
        HashCode hash = new();
        hash.Add(EqualityContract);
        hash.Add(Name, StringComparer.Ordinal);
        hash.Add(DomainName, StringComparer.Ordinal);
        AstSequence.AddHashCode(ref hash, Requirements);
        AstSequence.AddHashCode(ref hash, Objects);
        AstSequence.AddHashCode(ref hash, Init);
        hash.Add(Goal);
        hash.Add(Metric);
        return hash.ToHashCode();
    }

    public string ToPddlString()
    {
        var builder = new System.Text.StringBuilder();
        builder.AppendLine($"(define (problem {Name})");
        builder.AppendLine($"  (:domain {DomainName})");

        if (!Requirements.IsDefaultOrEmpty)
            builder.AppendLine($"  (:requirements {string.Join(" ", Requirements.Select(r => r.ToPddlString()))})");

        if (!Objects.IsDefaultOrEmpty)
            builder.AppendLine($"  (:objects {TypedListSerializer.Serialize(Objects)})");

        builder.AppendLine("  (:init");
        foreach(var expr in Init) builder.AppendLine($"    {expr.ToPddlString()}");
        builder.AppendLine("  )");

        builder.AppendLine($"  (:goal {Goal.ToPddlString()})");

        if (Metric != null)
            builder.AppendLine($"  {Metric.ToPddlString()}");

        builder.AppendLine(")");
        return builder.ToString();
    }

    public override string ToString() => ToPddlString();
}
