using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class ActionListBuilder
{
    private readonly DomainBuilder _parent;
    private readonly List<BuilderActionSpec> _actions = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private ActionSchemaBuilder? _activeAction;
    private bool _closed;

    internal ActionListBuilder(DomainBuilder parent)
    {
        _parent = parent;
    }

    public ActionSchemaBuilder Add(string name)
    {
        EnsureAvailable();
        name = BuilderName.RequireName(name, nameof(name));
        if (_names.Contains(name))
            throw new ArgumentException($"Action '{name}' was added more than once.", nameof(name));

        var action = new ActionSchemaBuilder(this, _parent, name);
        _activeAction = action;
        return action;
    }

    public DomainBuilder Close()
    {
        EnsureAvailable();
        _parent.CommitActions(this, _actions);
        _closed = true;
        return _parent;
    }

    internal void CommitAction(ActionSchemaBuilder child, BuilderActionSpec action)
    {
        EnsureOpen();
        if (!ReferenceEquals(_activeAction, child))
            throw new InvalidOperationException("The action builder is not active.");

        _parent.ValidateAction(action);
        if (!_names.Add(action.Name))
            throw new InvalidOperationException($"Action '{action.Name}' was added more than once.");
        _actions.Add(action);
        _activeAction = null;
    }

    private void EnsureAvailable()
    {
        EnsureOpen();
        if (_activeAction is not null)
            throw new InvalidOperationException("Close the current action before modifying the action list.");
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The action list builder is closed.");
    }
}

public sealed class ActionSchemaBuilder
{
    private readonly ActionListBuilder _parent;
    private readonly DomainBuilder _domain;
    private readonly string _name;
    private readonly List<BuilderParameterSpec> _parameters = new();
    private readonly HashSet<string> _parameterNames = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<BuilderLiteralSpec> _preconditions = new();
    private readonly List<LogicalExpressionSpec> _expressions = new();
    private readonly List<BuilderNumericUpdateSpec> _numericUpdates = new();
    private readonly List<BuilderLiteralSpec> _effects = new();
    private readonly List<BuilderConditionalEffectSpec> _conditionalEffects = new();
    private NumericExpressionSpec? _cost;
    private ConditionalEffectBuilder? _activeConditionalEffect;
    private bool _closed;

    internal ActionSchemaBuilder(ActionListBuilder parent, DomainBuilder domain, string name)
    {
        _parent = parent;
        _domain = domain;
        _name = name;
    }

    public ActionSchemaBuilder AddParameter(string name, string type = "object")
    {
        EnsureAvailable();
        name = BuilderName.RequireVariable(name, nameof(name));
        type = BuilderName.RequireName(type, nameof(type));
        if (!_parameterNames.Add(name))
            throw new ArgumentException($"Parameter '{name}' was added more than once.", nameof(name));
        _parameters.Add(new BuilderParameterSpec(name, type));
        return this;
    }

    public ActionSchemaBuilder AddPrecondition(string predicateName, params string[] arguments)
        => AddPrecondition(predicateName, Polarity.Positive, arguments);

    public ActionSchemaBuilder AddPrecondition(
        string predicateName,
        Polarity polarity,
        params string[] arguments)
    {
        EnsureAvailable();
        _preconditions.Add(CreateLiteral(predicateName, polarity, arguments));
        return this;
    }

    public ActionSchemaBuilder AddPrecondition(LogicalExpressionSpec condition)
    {
        EnsureAvailable();
        ArgumentNullException.ThrowIfNull(condition);
        _expressions.Add(condition);
        return this;
    }

    public ActionSchemaBuilder Assign(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.Assign, expression);
    public ActionSchemaBuilder Increase(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.Increase, expression);
    public ActionSchemaBuilder Decrease(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.Decrease, expression);
    public ActionSchemaBuilder ScaleUp(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.ScaleUp, expression);
    public ActionSchemaBuilder ScaleDown(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.ScaleDown, expression);
    internal ActionSchemaBuilder NumericUpdate(NumericFunctionSpec target, NumericUpdateOperator operation, NumericExpressionSpec expression)
    {
        EnsureAvailable();
        ArgumentNullException.ThrowIfNull(target);
        ArgumentNullException.ThrowIfNull(expression);
        if (!Enum.IsDefined(operation)) throw new ArgumentOutOfRangeException(nameof(operation));
        _numericUpdates.Add(new BuilderNumericUpdateSpec(target, operation, expression));
        return this;
    }

    public ActionSchemaBuilder AddEffect(string predicateName, params string[] arguments)
        => AddEffect(predicateName, Polarity.Positive, arguments);

    public ActionSchemaBuilder AddEffect(
        string predicateName,
        Polarity polarity,
        params string[] arguments)
    {
        EnsureAvailable();
        _effects.Add(CreateLiteral(predicateName, polarity, arguments));
        return this;
    }

    public ConditionalEffectBuilder AddConditionalEffect()
    {
        EnsureAvailable();
        var builder = new ConditionalEffectBuilder(this, _name);
        _activeConditionalEffect = builder;
        return builder;
    }

    public ActionSchemaBuilder WithCost(double cost)
        => WithCost(Numeric.Constant(cost));

    public ActionSchemaBuilder WithCost(NumericExpressionSpec cost)
    {
        EnsureAvailable();
        ArgumentNullException.ThrowIfNull(cost);
        if (_cost is not null)
            throw new InvalidOperationException("The action cost has already been set.");

        _cost = cost;
        return this;
    }

    public ActionListBuilder Close()
    {
        EnsureAvailable();
        BuilderActionSpec action = CreateDraft();
        _parent.CommitAction(this, action);
        _closed = true;
        return _parent;
    }

    internal void CommitConditionalEffect(
        ConditionalEffectBuilder child,
        BuilderConditionalEffectSpec effect)
    {
        EnsureOpen();
        if (!ReferenceEquals(_activeConditionalEffect, child))
            throw new InvalidOperationException("The conditional effect builder is not active.");

        _domain.ValidateConditionalEffect(_name, _parameters, effect);
        _conditionalEffects.Add(effect);
        _activeConditionalEffect = null;
    }

    private BuilderActionSpec CreateDraft()
        => new(
            _name,
            _parameters.ToArray(),
            _preconditions.ToArray(),
            _effects.ToArray(),
            _conditionalEffects.ToArray(),
            _cost,
            _expressions.ToArray(),
            _numericUpdates.ToArray());

    private static BuilderLiteralSpec CreateLiteral(
        string predicateName,
        Polarity polarity,
        string[] arguments)
    {
        predicateName = BuilderName.RequirePredicateReference(predicateName, nameof(predicateName));
        if (!Enum.IsDefined(polarity))
            throw new ArgumentOutOfRangeException(nameof(polarity), polarity, null);
        return new BuilderLiteralSpec(
            predicateName,
            polarity,
            BuilderName.CopyTerms(arguments, nameof(arguments)));
    }

    private void EnsureAvailable()
    {
        EnsureOpen();
        if (_activeConditionalEffect is not null)
            throw new InvalidOperationException("Close the conditional effect before modifying the action.");
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The action schema builder is closed.");
    }
}

public sealed class ConditionalEffectBuilder
{
    private readonly ActionSchemaBuilder _parent;
    private readonly string _actionName;
    private readonly List<BuilderParameterSpec> _parameters = new();
    private readonly HashSet<string> _parameterNames = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<BuilderLiteralSpec> _conditions = new();
    private BuilderLiteralSpec? _effect;
    private readonly List<LogicalExpressionSpec> _expressions = new();
    private BuilderNumericUpdateSpec? _numericUpdate;
    private bool _closed;

    internal ConditionalEffectBuilder(
        ActionSchemaBuilder parent,
        string actionName)
    {
        _parent = parent;
        _actionName = actionName;
    }

    public ConditionalEffectBuilder AddParameter(string name, string type = "object")
    {
        EnsureOpen();
        name = BuilderName.RequireVariable(name, nameof(name));
        type = BuilderName.RequireName(type, nameof(type));
        if (!_parameterNames.Add(name))
            throw new ArgumentException($"Parameter '{name}' was added more than once.", nameof(name));
        _parameters.Add(new BuilderParameterSpec(name, type));
        return this;
    }

    public ConditionalEffectBuilder AddCondition(string predicateName, params string[] arguments)
        => AddCondition(predicateName, Polarity.Positive, arguments);

    public ConditionalEffectBuilder AddCondition(
        string predicateName,
        Polarity polarity,
        params string[] arguments)
    {
        EnsureOpen();
        _conditions.Add(CreateLiteral(predicateName, polarity, arguments));
        return this;
    }

    public ConditionalEffectBuilder AddCondition(LogicalExpressionSpec condition)
    {
        EnsureOpen();
        ArgumentNullException.ThrowIfNull(condition);
        _expressions.Add(condition);
        return this;
    }
    public ConditionalEffectBuilder Assign(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.Assign, expression);
    public ConditionalEffectBuilder Increase(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.Increase, expression);
    public ConditionalEffectBuilder Decrease(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.Decrease, expression);
    public ConditionalEffectBuilder ScaleUp(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.ScaleUp, expression);
    public ConditionalEffectBuilder ScaleDown(NumericFunctionSpec target, NumericExpressionSpec expression) => NumericUpdate(target, NumericUpdateOperator.ScaleDown, expression);
    internal ConditionalEffectBuilder NumericUpdate(NumericFunctionSpec target, NumericUpdateOperator operation, NumericExpressionSpec expression)
    {
        EnsureOpen();
        ArgumentNullException.ThrowIfNull(target);
        ArgumentNullException.ThrowIfNull(expression);
        if (!Enum.IsDefined(operation)) throw new ArgumentOutOfRangeException(nameof(operation));
        if (_effect is not null || _numericUpdate is not null)
            throw new InvalidOperationException("A conditional effect must contain exactly one effect.");
        _numericUpdate = new BuilderNumericUpdateSpec(target, operation, expression);
        return this;
    }

    public ConditionalEffectBuilder AddEffect(string predicateName, params string[] arguments)
        => AddEffect(predicateName, Polarity.Positive, arguments);

    public ConditionalEffectBuilder AddEffect(
        string predicateName,
        Polarity polarity,
        params string[] arguments)
    {
        EnsureOpen();
        if (_effect is not null || _numericUpdate is not null)
            throw new InvalidOperationException("A conditional effect must contain exactly one effect literal.");
        _effect = CreateLiteral(predicateName, polarity, arguments);
        return this;
    }

    public ActionSchemaBuilder Close()
    {
        EnsureOpen();
        if (_effect is null && _numericUpdate is null)
            throw new InvalidOperationException("A conditional effect must contain exactly one effect literal.");

        var effect = new BuilderConditionalEffectSpec(
            _parameters.ToArray(),
            _conditions.ToArray(),
            _effect, _expressions.ToArray(), _numericUpdate);
        _parent.CommitConditionalEffect(this, effect);
        _closed = true;
        return _parent;
    }

    private static BuilderLiteralSpec CreateLiteral(
        string predicateName,
        Polarity polarity,
        string[] arguments)
    {
        predicateName = BuilderName.RequirePredicateReference(predicateName, nameof(predicateName));
        if (!Enum.IsDefined(polarity))
            throw new ArgumentOutOfRangeException(nameof(polarity), polarity, null);
        return new BuilderLiteralSpec(
            predicateName,
            polarity,
            BuilderName.CopyTerms(arguments, nameof(arguments)));
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException($"The conditional effect builder for action '{_actionName}' is closed.");
    }
}
