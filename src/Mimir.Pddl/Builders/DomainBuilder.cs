using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Builders;

public class ActionBuilder
{
    private readonly string _name;
    private readonly List<Parameter> _parameters = new();
    private ILogicalExpression? _precondition;
    private IEffect? _effect;

    public ActionBuilder(string name)
    {
        _name = PddlName.RequireName(name, nameof(name));
    }

    public ActionBuilder AddParameter(string name, string typeName = "object")
    {
        string validName = PddlName.RequireVariable(name, nameof(name));
        string validTypeName = PddlName.RequireName(typeName, nameof(typeName));
        _parameters.Add(new Parameter(validName, validTypeName));
        return this;
    }

    public ActionBuilder WithPrecondition(ILogicalExpression precondition)
    {
        ArgumentNullException.ThrowIfNull(precondition);
        _precondition = precondition;
        return this;
    }

    public ActionBuilder WithEffect(IEffect effect)
    {
        ArgumentNullException.ThrowIfNull(effect);
        _effect = effect;
        return this;
    }

    public ActionDefinition Build()
    {
        return new ActionDefinition(_name, _parameters.ToImmutableArray(), _precondition, _effect);
    }
}

public class DomainBuilder
{
    private readonly string _name;
    private readonly HashSet<PddlRequirement> _requirements = new();
    private readonly List<TypeDeclaration> _types = new();
    private readonly List<TypeDeclaration> _constants = new();
    private readonly List<PredicateDeclaration> _predicates = new();
    private readonly List<FunctionDeclaration> _functions = new();
    private readonly List<ActionDefinition> _actions = new();
    private readonly List<DerivedPredicate> _derivedPredicates = new();

    public DomainBuilder(string name)
    {
        _name = PddlName.RequireName(name, nameof(name));
    }

    public DomainBuilder AddRequirement(PddlRequirement requirement)
    {
        if (!Enum.IsDefined(requirement))
            throw new ArgumentOutOfRangeException(nameof(requirement), requirement, null);

        _requirements.Add(requirement);
        return this;
    }

    public DomainBuilder AddType(string name, string parentType = "object")
    {
        string validName = PddlName.RequireName(name, nameof(name));
        string validParentType = PddlName.RequireName(parentType, nameof(parentType));
        _types.Add(new TypeDeclaration(validName, validParentType));
        return this;
    }

    public DomainBuilder AddConstant(string name, string typeName = "object")
    {
        string validName = PddlName.RequireName(name, nameof(name));
        string validTypeName = PddlName.RequireName(typeName, nameof(typeName));
        _constants.Add(new TypeDeclaration(validName, validTypeName));
        return this;
    }

    public DomainBuilder AddPredicate(string name, params Parameter[] parameters)
    {
        string validName = PddlName.RequireName(name, nameof(name));
        ArgumentNullException.ThrowIfNull(parameters);
        if (parameters.Any(parameter => parameter is null))
            throw new ArgumentException("Parameters cannot contain null values.", nameof(parameters));

        _predicates.Add(new PredicateDeclaration(validName, parameters.ToImmutableArray()));
        return this;
    }

    public DomainBuilder AddFunction(string name, params Parameter[] parameters)
    {
        string validName = PddlName.RequireName(name, nameof(name));
        ArgumentNullException.ThrowIfNull(parameters);
        if (parameters.Any(parameter => parameter is null))
            throw new ArgumentException("Parameters cannot contain null values.", nameof(parameters));

        _functions.Add(new FunctionDeclaration(validName, parameters.ToImmutableArray()));
        return this;
    }

    public DomainBuilder AddAction(string name, Action<ActionBuilder> configure)
    {
        string validName = PddlName.RequireName(name, nameof(name));
        ArgumentNullException.ThrowIfNull(configure);

        var builder = new ActionBuilder(validName);
        configure(builder);
        _actions.Add(builder.Build());
        return this;
    }

    public DomainBuilder AddAction(ActionDefinition action)
    {
        ArgumentNullException.ThrowIfNull(action);
        PddlName.RequireName(action.Name, nameof(action));
        _actions.Add(action);
        return this;
    }

    public DomainBuilder AddDerivedPredicate(PredicateDeclaration signature, ILogicalExpression body)
    {
        ArgumentNullException.ThrowIfNull(signature);
        ArgumentNullException.ThrowIfNull(body);
        _derivedPredicates.Add(new DerivedPredicate(signature, body));
        return this;
    }

    public DomainDefinition Build()
    {
        return new DomainDefinition(
            _name,
            _requirements.ToImmutableArray(),
            _types.ToImmutableArray(),
            _constants.ToImmutableArray(),
            _predicates.ToImmutableArray(),
            _functions.ToImmutableArray(),
            _actions.ToImmutableArray(),
            _derivedPredicates.ToImmutableArray()
        );
    }
}
