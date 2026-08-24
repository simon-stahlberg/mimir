using Mimir.Core.Grounding;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

public sealed class DomainBuilder
{
    private const int RequirementStage = 0;
    private const int TypeStage = 1;
    private const int ConstantStage = 2;
    private const int PredicateStage = 3;
    private const int FunctionStage = 4;
    private const int ActionStage = 5;
    private const int DerivedPredicateStage = 6;

    private readonly string _name;
    private IReadOnlyList<string> _requirements = Array.Empty<string>();
    private IReadOnlyList<BuilderTypedNameSpec> _types = Array.Empty<BuilderTypedNameSpec>();
    private IReadOnlyList<BuilderTypedNameSpec> _constants = Array.Empty<BuilderTypedNameSpec>();
    private IReadOnlyList<BuilderPredicateSpec> _predicates = Array.Empty<BuilderPredicateSpec>();
    private IReadOnlyList<BuilderFunctionSpec> _functions = Array.Empty<BuilderFunctionSpec>();
    private IReadOnlyList<BuilderActionSpec> _actions = Array.Empty<BuilderActionSpec>();
    private IReadOnlyList<BuilderDerivedPredicateSpec> _derivedPredicates =
        Array.Empty<BuilderDerivedPredicateSpec>();
    private IReadOnlyDictionary<string, string> _typeHierarchy =
        new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
    private IReadOnlyDictionary<string, BuilderTypedNameSpec> _constantsByName =
        new Dictionary<string, BuilderTypedNameSpec>(StringComparer.OrdinalIgnoreCase);
    private IReadOnlyDictionary<string, BuilderPredicateSpec> _predicatesByName =
        new Dictionary<string, BuilderPredicateSpec>(StringComparer.OrdinalIgnoreCase);
    private IReadOnlyDictionary<string, BuilderFunctionSpec> _functionsByName =
        new Dictionary<string, BuilderFunctionSpec>(StringComparer.OrdinalIgnoreCase);
    private object? _activeChild;
    private int _nextStage;
    private bool _built;

    public DomainBuilder(string name)
    {
        _name = BuilderName.RequireName(name, nameof(name));
    }

    public RequirementListBuilder Requirements()
        => OpenSection(RequirementStage, () => new RequirementListBuilder(this));

    public TypeListBuilder Types()
        => OpenSection(TypeStage, () => new TypeListBuilder(this));

    public ConstantListBuilder Constants()
        => OpenSection(ConstantStage, () => new ConstantListBuilder(this));

    public PredicateListBuilder Predicates()
        => OpenSection(PredicateStage, () => new PredicateListBuilder(this));

    public NumericFunctionListBuilder Functions()
        => OpenSection(FunctionStage, () => new NumericFunctionListBuilder(this));

    public ActionListBuilder Actions()
        => OpenSection(ActionStage, () => new ActionListBuilder(this));

    public DerivedPredicateListBuilder DerivedPredicates()
        => OpenSection(DerivedPredicateStage, () => new DerivedPredicateListBuilder(this));

    public Domain Build()
    {
        EnsureOpen();
        if (_activeChild is not null)
            throw new InvalidOperationException("The open domain section must be closed before building the domain.");

        ValidateCrossSectionRules();
        try
        {
            ProgrammaticDomainCompilation compilation = ProgrammaticDomainCompiler.Compile(this);
            DomainDefinition canonical = Canonicalizer.Compile(compilation.Definition);
            CorePddlSupportValidator.ValidateDomain(canonical);
            DerivedDependencyValidator.Validate(canonical);
            Domain domain = Domain.CreateProgrammatic(canonical, compilation.Inputs);
            _built = true;
            return domain;
        }
        catch (PddlValidationException exception)
        {
            throw new InvalidOperationException(exception.Message, exception);
        }
        catch (NotSupportedException exception)
        {
            throw new InvalidOperationException(exception.Message, exception);
        }
    }

    internal string Name => _name;
    internal IReadOnlyList<string> RequirementSpecs => _requirements;
    internal IReadOnlyList<BuilderTypedNameSpec> TypeSpecs => _types;
    internal IReadOnlyList<BuilderTypedNameSpec> ConstantSpecs => _constants;
    internal IReadOnlyList<BuilderPredicateSpec> PredicateSpecs => _predicates;
    internal IReadOnlyList<BuilderFunctionSpec> FunctionSpecs => _functions;
    internal IReadOnlyList<BuilderActionSpec> ActionSpecs => _actions;
    internal IReadOnlyList<BuilderDerivedPredicateSpec> DerivedPredicateSpecs => _derivedPredicates;

    internal void CommitRequirements(RequirementListBuilder child, IReadOnlyList<string> requirements)
    {
        EnsureActiveChild(child);
        _requirements = Array.AsReadOnly(requirements.ToArray());
        _activeChild = null;
    }

    internal void CommitTypes(TypeListBuilder child, IReadOnlyList<BuilderTypedNameSpec> types)
    {
        EnsureActiveChild(child);
        ValidateTypes(types);
        BuilderTypedNameSpec[] copy = types.ToArray();
        _types = Array.AsReadOnly(copy);
        _typeHierarchy = copy.ToDictionary(type => type.Name, type => type.Type, StringComparer.OrdinalIgnoreCase);
        _activeChild = null;
    }

    internal void CommitConstants(ConstantListBuilder child, IReadOnlyList<BuilderTypedNameSpec> constants)
    {
        EnsureActiveChild(child);
        foreach (BuilderTypedNameSpec constant in constants)
            RequireDeclaredType(constant.Type, $"constant '{constant.Name}'");

        BuilderTypedNameSpec[] copy = constants.ToArray();
        _constants = Array.AsReadOnly(copy);
        _constantsByName = copy.ToDictionary(value => value.Name, StringComparer.OrdinalIgnoreCase);
        _activeChild = null;
    }

    internal void CommitPredicates(PredicateListBuilder child, IReadOnlyList<BuilderPredicateSpec> predicates)
    {
        EnsureActiveChild(child);
        foreach (BuilderPredicateSpec predicate in predicates)
            ValidateDeclarationParameters(predicate.Parameters, $"predicate '{predicate.Name}'");

        BuilderPredicateSpec[] copy = predicates.ToArray();
        _predicates = Array.AsReadOnly(copy);
        _predicatesByName = copy.ToDictionary(value => value.Name, StringComparer.OrdinalIgnoreCase);
        _activeChild = null;
    }

    internal void CommitFunctions(
        NumericFunctionListBuilder child,
        IReadOnlyList<BuilderFunctionSpec> functions)
    {
        EnsureActiveChild(child);
        foreach (BuilderFunctionSpec function in functions)
        {
            ValidateDeclarationParameters(function.Parameters, $"numeric function '{function.Name}'");
            if (_predicatesByName.ContainsKey(function.Name))
                throw new InvalidOperationException($"A predicate named '{function.Name}' is already declared.");
        }

        BuilderFunctionSpec[] copy = functions.ToArray();
        _functions = Array.AsReadOnly(copy);
        _functionsByName = copy.ToDictionary(value => value.Name, StringComparer.OrdinalIgnoreCase);
        _activeChild = null;
    }

    internal void CommitActions(ActionListBuilder child, IReadOnlyList<BuilderActionSpec> actions)
    {
        EnsureActiveChild(child);
        _actions = Array.AsReadOnly(actions.ToArray());
        _activeChild = null;
    }

    internal void CommitDerivedPredicates(
        DerivedPredicateListBuilder child,
        IReadOnlyList<BuilderDerivedPredicateSpec> definitions)
    {
        EnsureActiveChild(child);
        foreach (BuilderDerivedPredicateSpec definition in definitions)
            ValidateDerivedDefinition(definition);

        _derivedPredicates = Array.AsReadOnly(definitions.ToArray());
        _activeChild = null;
    }

    internal void ValidateAction(BuilderActionSpec action)
    {
        ValidateDeclarationParameters(action.Parameters, $"action '{action.Name}'");
        Dictionary<string, string> scope = action.Parameters.ToDictionary(
            parameter => parameter.Name,
            parameter => parameter.Type,
            StringComparer.OrdinalIgnoreCase);

        foreach (BuilderLiteralSpec precondition in action.Preconditions)
            ValidateLiteral(precondition, scope, isEffect: false, $"action '{action.Name}' precondition");
        foreach (BuilderLiteralSpec effect in action.Effects)
            ValidateLiteral(effect, scope, isEffect: true, $"action '{action.Name}' effect");

        foreach (BuilderConditionalEffectSpec conditionalEffect in action.ConditionalEffects)
        {
            ValidateDeclarationParameters(
                conditionalEffect.Parameters,
                $"conditional effect in action '{action.Name}'");
            var conditionalScope = new Dictionary<string, string>(scope, StringComparer.OrdinalIgnoreCase);
            foreach (BuilderParameterSpec parameter in conditionalEffect.Parameters)
            {
                if (!conditionalScope.TryAdd(parameter.Name, parameter.Type))
                {
                    throw new InvalidOperationException(
                        $"Variable '{parameter.Name}' shadows another variable in action '{action.Name}'.");
                }
            }

            foreach (BuilderLiteralSpec condition in conditionalEffect.Conditions)
            {
                ValidateLiteral(
                    condition,
                    conditionalScope,
                    isEffect: false,
                    $"conditional effect condition in action '{action.Name}'");
            }
            ValidateLiteral(
                conditionalEffect.Effect,
                conditionalScope,
                isEffect: true,
                $"conditional effect target in action '{action.Name}'");
        }

        ValidateActionCost(action.Cost.Node, scope, action.Name);
        if (!HasRequirement(":action-costs")
            && action.Cost.Node is not ConstantActionCostNode { Value: 1d })
        {
            throw new InvalidOperationException(
                $"Action '{action.Name}' has a non-default cost but the domain does not require :action-costs.");
        }
    }

    internal void ValidateConditionalEffect(
        string actionName,
        IReadOnlyList<BuilderParameterSpec> actionParameters,
        BuilderConditionalEffectSpec conditionalEffect)
    {
        ValidateDeclarationParameters(
            conditionalEffect.Parameters,
            $"conditional effect in action '{actionName}'");
        Dictionary<string, string> scope = actionParameters.ToDictionary(
            parameter => parameter.Name,
            parameter => parameter.Type,
            StringComparer.OrdinalIgnoreCase);
        foreach (BuilderParameterSpec parameter in conditionalEffect.Parameters)
        {
            if (!scope.TryAdd(parameter.Name, parameter.Type))
            {
                throw new InvalidOperationException(
                    $"Variable '{parameter.Name}' shadows another variable in action '{actionName}'.");
            }
        }

        foreach (BuilderLiteralSpec condition in conditionalEffect.Conditions)
        {
            ValidateLiteral(
                condition,
                scope,
                isEffect: false,
                $"conditional effect condition in action '{actionName}'");
        }
        ValidateLiteral(
            conditionalEffect.Effect,
            scope,
            isEffect: true,
            $"conditional effect target in action '{actionName}'");
    }

    private T OpenSection<T>(int stage, Func<T> create)
        where T : class
    {
        EnsureOpen();
        if (_activeChild is not null)
            throw new InvalidOperationException("Close the current domain section before opening another section.");
        if (stage < _nextStage)
            throw new InvalidOperationException("Domain sections cannot be reopened or added out of order.");

        T child = create();
        _nextStage = stage + 1;
        _activeChild = child;
        return child;
    }

    private void EnsureOpen()
    {
        if (_built)
            throw new InvalidOperationException("The domain builder has already built a domain.");
    }

    private void EnsureActiveChild(object child)
    {
        EnsureOpen();
        if (!ReferenceEquals(_activeChild, child))
            throw new InvalidOperationException("The domain section is not the active section.");
    }

    private void ValidateTypes(IReadOnlyList<BuilderTypedNameSpec> types)
    {
        var hierarchy = types.ToDictionary(type => type.Name, type => type.Type, StringComparer.OrdinalIgnoreCase);
        foreach (BuilderTypedNameSpec type in types)
        {
            if (!type.Type.Equals("object", StringComparison.OrdinalIgnoreCase)
                && !hierarchy.ContainsKey(type.Type))
            {
                throw new InvalidOperationException(
                    $"Parent type '{type.Type}' of type '{type.Name}' is not declared.");
            }

            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase) { type.Name };
            string current = type.Name;
            while (hierarchy.TryGetValue(current, out string? parent)
                   && !parent.Equals("object", StringComparison.OrdinalIgnoreCase))
            {
                if (!visited.Add(parent))
                    throw new InvalidOperationException($"Type hierarchy contains a cycle involving '{parent}'.");
                current = parent;
            }
        }
    }

    private void ValidateDeclarationParameters(
        IReadOnlyList<BuilderParameterSpec> parameters,
        string owner)
    {
        foreach (BuilderParameterSpec parameter in parameters)
            RequireDeclaredType(parameter.Type, $"parameter '{parameter.Name}' of {owner}");
    }

    private void ValidateDerivedDefinition(BuilderDerivedPredicateSpec definition)
    {
        if (!_predicatesByName.TryGetValue(definition.PredicateName, out BuilderPredicateSpec? predicate))
        {
            throw new InvalidOperationException(
                $"Derived predicate '{definition.PredicateName}' is not declared.");
        }

        Dictionary<string, string> scope = predicate.Parameters.ToDictionary(
            parameter => parameter.Name,
            parameter => parameter.Type,
            StringComparer.OrdinalIgnoreCase);
        ValidateLogicalExpression(definition.Body.Node, scope, $"derived predicate '{predicate.Name}'");
    }

    private void ValidateLogicalExpression(
        LogicalExpressionNode expression,
        IReadOnlyDictionary<string, string> scope,
        string owner)
    {
        switch (expression)
        {
            case TrueLogicalExpressionNode or FalseLogicalExpressionNode:
                return;
            case AtomLogicalExpressionNode atom:
                ValidatePredicateBinding(atom.PredicateName, atom.Arguments, scope, owner);
                return;
            case EqualityLogicalExpressionNode equality:
                if (!HasEquality())
                    throw new InvalidOperationException($"{owner} uses equality without :equality or :adl.");
                ResolveTermType(equality.Left, scope, owner);
                ResolveTermType(equality.Right, scope, owner);
                return;
            case NotLogicalExpressionNode not:
                ValidateLogicalExpression(not.Expression, scope, owner);
                return;
            case AndLogicalExpressionNode and:
                foreach (LogicalExpressionNode child in and.Expressions)
                    ValidateLogicalExpression(child, scope, owner);
                return;
            case OrLogicalExpressionNode or:
                foreach (LogicalExpressionNode child in or.Expressions)
                    ValidateLogicalExpression(child, scope, owner);
                return;
            case ImplyLogicalExpressionNode imply:
                ValidateLogicalExpression(imply.Antecedent, scope, owner);
                ValidateLogicalExpression(imply.Consequent, scope, owner);
                return;
            case ExistsLogicalExpressionNode exists:
                ValidateQuantifiedExpression(exists.Parameters, exists.Body, scope, owner);
                return;
            case ForallLogicalExpressionNode forall:
                ValidateQuantifiedExpression(forall.Parameters, forall.Body, scope, owner);
                return;
            default:
                throw new InvalidOperationException(
                    $"Unsupported logical expression specification '{expression.GetType().Name}'.");
        }
    }

    private void ValidateQuantifiedExpression(
        IReadOnlyList<BuilderParameterSpec> parameters,
        LogicalExpressionNode body,
        IReadOnlyDictionary<string, string> parentScope,
        string owner)
    {
        ValidateDeclarationParameters(parameters, owner);
        var scope = new Dictionary<string, string>(parentScope, StringComparer.OrdinalIgnoreCase);
        foreach (BuilderParameterSpec parameter in parameters)
        {
            if (!scope.TryAdd(parameter.Name, parameter.Type))
                throw new InvalidOperationException($"Variable '{parameter.Name}' shadows another variable in {owner}.");
        }

        ValidateLogicalExpression(body, scope, owner);
    }

    private void ValidateLiteral(
        BuilderLiteralSpec literal,
        IReadOnlyDictionary<string, string> scope,
        bool isEffect,
        string owner)
    {
        if (literal.PredicateName == "=")
        {
            if (isEffect)
                throw new InvalidOperationException("Equality cannot be used as an action effect.");
            if (!HasEquality())
                throw new InvalidOperationException($"{owner} uses equality without :equality or :adl.");
            if (literal.Arguments.Count != 2)
                throw new InvalidOperationException("Equality expects exactly two arguments.");
            foreach (string argument in literal.Arguments)
                ResolveTermType(argument, scope, owner);
            return;
        }

        ValidatePredicateBinding(literal.PredicateName, literal.Arguments, scope, owner);
    }

    private void ValidatePredicateBinding(
        string predicateName,
        IReadOnlyList<string> arguments,
        IReadOnlyDictionary<string, string> scope,
        string owner)
    {
        if (!_predicatesByName.TryGetValue(predicateName, out BuilderPredicateSpec? predicate))
            throw new InvalidOperationException($"Predicate '{predicateName}' used by {owner} is not declared.");
        if (predicate.Parameters.Count != arguments.Count)
        {
            throw new InvalidOperationException(
                $"Predicate '{predicate.Name}' expects {predicate.Parameters.Count} arguments, got {arguments.Count}.");
        }

        for (int index = 0; index < arguments.Count; index++)
        {
            string actualType = ResolveTermType(arguments[index], scope, owner);
            string expectedType = predicate.Parameters[index].Type;
            if (!IsCompatible(actualType, expectedType))
            {
                throw new InvalidOperationException(
                    $"Term '{arguments[index]}' has type '{actualType}', expected '{expectedType}' for predicate '{predicate.Name}'.");
            }
        }
    }

    private void ValidateActionCost(
        ActionCostNode cost,
        IReadOnlyDictionary<string, string> scope,
        string actionName)
    {
        switch (cost)
        {
            case ConstantActionCostNode:
                return;
            case BinaryActionCostNode binary:
                ValidateActionCost(binary.Left, scope, actionName);
                ValidateActionCost(binary.Right, scope, actionName);
                return;
            case FunctionActionCostNode functionCall:
                if (!_functionsByName.TryGetValue(functionCall.FunctionName, out BuilderFunctionSpec? function))
                {
                    throw new InvalidOperationException(
                        $"Numeric function '{functionCall.FunctionName}' used by action '{actionName}' is not declared.");
                }
                if (function.Parameters.Count != functionCall.Arguments.Count)
                {
                    throw new InvalidOperationException(
                        $"Numeric function '{function.Name}' expects {function.Parameters.Count} arguments, got {functionCall.Arguments.Count}.");
                }
                for (int index = 0; index < functionCall.Arguments.Count; index++)
                {
                    string actualType = ResolveTermType(functionCall.Arguments[index], scope, $"action '{actionName}' cost");
                    string expectedType = function.Parameters[index].Type;
                    if (!IsCompatible(actualType, expectedType))
                    {
                        throw new InvalidOperationException(
                            $"Term '{functionCall.Arguments[index]}' has type '{actualType}', expected '{expectedType}' for numeric function '{function.Name}'.");
                    }
                }
                return;
            default:
                throw new InvalidOperationException($"Unsupported action cost specification '{cost.GetType().Name}'.");
        }
    }

    private string ResolveTermType(
        string term,
        IReadOnlyDictionary<string, string> scope,
        string owner)
    {
        if (term.StartsWith('?'))
        {
            if (scope.TryGetValue(term, out string? variableType)) return variableType;
            throw new InvalidOperationException($"Variable '{term}' used by {owner} is not in scope.");
        }

        if (_constantsByName.TryGetValue(term, out BuilderTypedNameSpec? constant)) return constant.Type;
        throw new InvalidOperationException($"Constant '{term}' used by {owner} is not declared.");
    }

    private void RequireDeclaredType(string type, string owner)
    {
        if (type.Equals("object", StringComparison.OrdinalIgnoreCase) || _typeHierarchy.ContainsKey(type)) return;
        throw new InvalidOperationException($"Type '{type}' used by {owner} is not declared.");
    }

    private bool IsCompatible(string child, string parent)
    {
        if (child.Equals(parent, StringComparison.OrdinalIgnoreCase)
            || parent.Equals("object", StringComparison.OrdinalIgnoreCase))
        {
            return true;
        }

        string current = child;
        while (_typeHierarchy.TryGetValue(current, out string? next))
        {
            if (next.Equals(parent, StringComparison.OrdinalIgnoreCase)) return true;
            current = next;
        }

        return false;
    }

    private bool HasEquality()
        => HasRequirement(":equality") || HasRequirement(":adl");

    private bool HasRequirement(string requirement)
        => _requirements.Contains(requirement, StringComparer.OrdinalIgnoreCase);

    private void ValidateCrossSectionRules()
    {
        var effectTargets = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (BuilderActionSpec action in _actions)
        {
            effectTargets.UnionWith(action.Effects.Select(effect => effect.PredicateName));
            effectTargets.UnionWith(action.ConditionalEffects.Select(effect => effect.Effect.PredicateName));
        }

        foreach (BuilderDerivedPredicateSpec definition in _derivedPredicates)
        {
            if (effectTargets.Contains(definition.PredicateName))
            {
                throw new InvalidOperationException(
                    $"Predicate '{definition.PredicateName}' cannot be both derived and an action effect target.");
            }
        }
    }
}
