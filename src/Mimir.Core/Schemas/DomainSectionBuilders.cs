namespace Mimir.Core.Schemas;

public sealed class RequirementListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<string> _requirements = new();
    private readonly HashSet<Mimir.Pddl.Ast.Models.PddlRequirement> _requirementsAdded = new();
    private bool _closed;

    internal RequirementListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public RequirementListBuilder Add(string requirement)
    {
        EnsureOpen();
        IReadOnlyList<Mimir.Pddl.Ast.Models.PddlRequirement> parsed =
            ProgrammaticDomainCompiler.ParseRequirement(requirement);
        if (parsed.Any(value => _requirementsAdded.Contains(value)))
            throw new ArgumentException($"Requirement '{requirement}' was added more than once.", nameof(requirement));
        _requirementsAdded.UnionWith(parsed);
        _requirements.Add(requirement);
        return this;
    }

    public DomainBuilder Close()
    {
        EnsureOpen();
        _parent.CommitRequirements(this, _requirements);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The requirement list builder is closed.");
    }
}

public sealed class TypeListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<BuilderTypedNameSpec> _types = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal TypeListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public TypeListBuilder Add(string name, string parentType = "object")
    {
        EnsureOpen();
        name = BuilderName.RequireName(name, nameof(name));
        parentType = BuilderName.RequireName(parentType, nameof(parentType));
        if (name.Equals("object", StringComparison.OrdinalIgnoreCase))
            throw new ArgumentException("The built-in root type 'object' cannot be declared or reparented.", nameof(name));
        if (!_names.Add(name))
            throw new ArgumentException($"Type '{name}' was added more than once.", nameof(name));
        _types.Add(new BuilderTypedNameSpec(name, parentType));
        return this;
    }

    public DomainBuilder Close()
    {
        EnsureOpen();
        _parent.CommitTypes(this, _types);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The type list builder is closed.");
    }
}

public sealed class ConstantListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<BuilderTypedNameSpec> _constants = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal ConstantListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public ConstantListBuilder Add(string name, string type = "object")
    {
        EnsureOpen();
        name = BuilderName.RequireName(name, nameof(name));
        type = BuilderName.RequireName(type, nameof(type));
        if (!_names.Add(name))
            throw new ArgumentException($"Constant '{name}' was added more than once.", nameof(name));
        _constants.Add(new BuilderTypedNameSpec(name, type));
        return this;
    }

    public DomainBuilder Close()
    {
        EnsureOpen();
        _parent.CommitConstants(this, _constants);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The constant list builder is closed.");
    }
}

public sealed class PredicateListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<BuilderPredicateSpec> _predicates = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal PredicateListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public PredicateListBuilder Add(
        string name,
        params (string Name, string Type)[] parameters)
    {
        EnsureOpen();
        name = BuilderName.RequireName(name, nameof(name));
        if (name is "=" || name.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
            throw new ArgumentException($"The symbol '{name}' is reserved.", nameof(name));
        if (!_names.Add(name))
            throw new ArgumentException($"Predicate '{name}' was added more than once.", nameof(name));

        BuilderParameterSpec[] copy = BuilderName.CopyParameters(parameters, nameof(parameters));
        _predicates.Add(new BuilderPredicateSpec(name, copy));
        return this;
    }

    public DomainBuilder Close()
    {
        EnsureOpen();
        _parent.CommitPredicates(this, _predicates);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The predicate list builder is closed.");
    }
}

public sealed class NumericFunctionListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<BuilderFunctionSpec> _functions = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal NumericFunctionListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public NumericFunctionListBuilder Add(
        string name,
        params (string Name, string Type)[] parameters)
    {
        EnsureOpen();
        name = BuilderName.RequireName(name, nameof(name));
        if (name.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
            throw new ArgumentException("The built-in total-cost function is reserved for planner bookkeeping.", nameof(name));
        if (!_names.Add(name))
            throw new ArgumentException($"Numeric function '{name}' was added more than once.", nameof(name));

        BuilderParameterSpec[] copy = BuilderName.CopyParameters(parameters, nameof(parameters));
        _functions.Add(new BuilderFunctionSpec(name, copy));
        return this;
    }

    public DomainBuilder Close()
    {
        EnsureOpen();
        _parent.CommitFunctions(this, _functions);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The numeric function list builder is closed.");
    }
}

public sealed class DerivedPredicateListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<BuilderDerivedPredicateSpec> _definitions = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal DerivedPredicateListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public DerivedPredicateListBuilder Define(string predicateName, LogicalExpressionSpec body)
    {
        EnsureOpen();
        predicateName = BuilderName.RequireName(predicateName, nameof(predicateName));
        ArgumentNullException.ThrowIfNull(body);
        if (!_names.Add(predicateName))
        {
            throw new ArgumentException(
                $"Derived predicate '{predicateName}' was defined more than once.",
                nameof(predicateName));
        }

        _definitions.Add(new BuilderDerivedPredicateSpec(predicateName, body));
        return this;
    }

    public DomainBuilder Close()
    {
        EnsureOpen();
        _parent.CommitDerivedPredicates(this, _definitions);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The derived predicate list builder is closed.");
    }
}
