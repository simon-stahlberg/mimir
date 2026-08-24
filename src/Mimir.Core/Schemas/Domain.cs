using System.Collections.ObjectModel;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Core.Schemas;

public sealed record DerivedPredicateDefinition
{
    public Predicate<Derived> Predicate { get; }
    public IGroundedExpression Body { get; }

    public DerivedPredicateDefinition(Predicate<Derived> predicate, IGroundedExpression body)
    {
        ArgumentNullException.ThrowIfNull(predicate);
        ArgumentNullException.ThrowIfNull(body);

        Predicate = predicate;
        Body = body;
    }
}

public partial class Domain
{
    private readonly HashSet<Predicate> _predicateSet = new(ReferenceEqualityComparer.Instance);
    private readonly HashSet<NumericFunction> _functionSet = new(ReferenceEqualityComparer.Instance);
    private readonly HashSet<Constant> _constantSet = new(ReferenceEqualityComparer.Instance);
    private readonly HashSet<ActionSchema> _actionSet = new(ReferenceEqualityComparer.Instance);
    private readonly HashSet<Variable> _variableSet = new(ReferenceEqualityComparer.Instance);
    private readonly Dictionary<string, HashSet<string>> _compatibleParentTypes;

    internal DomainDefinition PddlDefinition { get; }

    public string Name { get; }
    public IReadOnlyList<Predicate<Fluent>> Fluents { get; }
    public IReadOnlyList<Predicate<Static>> Statics { get; }
    public IReadOnlyList<Predicate<Derived>> Derived { get; }
    public IReadOnlyList<NumericFunction> Functions { get; }
    public IReadOnlyList<Constant> Constants { get; }
    public IReadOnlyList<ActionSchema> Actions { get; }
    public IReadOnlyList<string> Requirements { get; }
    public IReadOnlyList<string> ExpandedRequirements { get; }
    public bool UsesTyping { get; }
    public bool UsesEquality { get; }
    public bool UsesConditionalEffects { get; }
    public IReadOnlyDictionary<string, string> TypeHierarchy { get; }
    public IReadOnlyDictionary<string, IGroundedExpression> DerivedDefinitions { get; }
    public Predicate<Static>? EqualityPredicate { get; }
    internal DerivedPredicatePlan DerivedPlan { get; }

    public static Domain FromFile(string filePath)
        => new(LoadFile(filePath));

    public static Domain FromText(string pddlText)
    {
        ArgumentNullException.ThrowIfNull(pddlText);
        return new Domain(LoadText(pddlText, sourcePath: null));
    }

    public bool IsCompatible(string child, string parent)
    {
        ArgumentNullException.ThrowIfNull(child);
        ArgumentNullException.ThrowIfNull(parent);

        return _compatibleParentTypes.TryGetValue(child, out HashSet<string>? compatibleParents)
            && compatibleParents.Contains(parent);
    }

    private static Dictionary<string, HashSet<string>> BuildTypeCompatibility(
        IReadOnlyDictionary<string, string> typeHierarchy)
    {
        var declaredTypes = new HashSet<string>(typeHierarchy.Keys, StringComparer.OrdinalIgnoreCase)
        {
            "object"
        };
        var compatibility = new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);

        foreach (string child in declaredTypes)
        {
            var compatibleParents = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            {
                child,
                "object"
            };
            string current = child;
            while (typeHierarchy.TryGetValue(current, out string? parent))
            {
                if (!declaredTypes.Contains(parent) || !compatibleParents.Add(parent)) break;
                current = parent;
            }

            compatibility.Add(child, compatibleParents);
        }

        return compatibility;
    }

    private static DomainDefinition LoadFile(string filePath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);

        string text = File.ReadAllText(filePath);
        return LoadText(text, filePath);
    }

    private static DomainDefinition LoadText(string text, string? sourcePath)
    {
        return PddlLoading.Execute(
            PddlDocumentType.Domain,
            sourcePath,
            () =>
            {
                DomainDefinition astDomain = PddlParser.ParseDomain(text);
                DomainDefinition canonicalDomain = Canonicalizer.Compile(astDomain);
                CorePddlSupportValidator.ValidateDomain(canonicalDomain);
                DerivedDependencyValidator.Validate(canonicalDomain);
                return canonicalDomain;
            });
    }

    internal Domain(DomainDefinition astDomain)
        : this(astDomain, programmaticInputs: null)
    {
    }

    internal static Domain CreateProgrammatic(
        DomainDefinition astDomain,
        ProgrammaticDomainInputs programmaticInputs)
    {
        ArgumentNullException.ThrowIfNull(programmaticInputs);
        return new Domain(astDomain, programmaticInputs);
    }

    private Domain(
        DomainDefinition astDomain,
        ProgrammaticDomainInputs? programmaticInputs)
    {
        if (!astDomain.IsCanonical())
            throw new ArgumentException("The provided AST DomainDefinition is not in canonical form.");

        CorePddlSupportValidator.ValidateDomain(astDomain);
        DerivedDependencyValidator.Validate(astDomain);

        PddlDefinition = astDomain;
        Name = astDomain.Name;

        var builder = new PddlDomainTranslator(astDomain, programmaticInputs);

        Requirements = Array.AsReadOnly(
            astDomain.Requirements.Select(requirement => requirement.ToPddlString()).ToArray());
        ExpandedRequirements = ExpandRequirements(astDomain.Requirements);
        UsesTyping = astDomain.Requirements.HasRequirement(PddlRequirement.Typing);
        UsesEquality = astDomain.Requirements.HasRequirement(PddlRequirement.Equality);
        UsesConditionalEffects = astDomain.Requirements.HasRequirement(PddlRequirement.ConditionalEffects);
        Fluents = Array.AsReadOnly(builder.Fluents.ToArray());
        Statics = Array.AsReadOnly(builder.Statics.ToArray());
        Derived = Array.AsReadOnly(builder.Derived.ToArray());
        Functions = Array.AsReadOnly(builder.Functions.ToArray());
        Constants = Array.AsReadOnly(builder.Constants.ToArray());
        Actions = Array.AsReadOnly(builder.Actions.ToArray());
        TypeHierarchy = new ReadOnlyDictionary<string, string>(
            builder.TypeHierarchy.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        _compatibleParentTypes = BuildTypeCompatibility(TypeHierarchy);
        DerivedDefinitions = new ReadOnlyDictionary<string, IGroundedExpression>(
            builder.DerivedDefinitions.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        EqualityPredicate = Statics.SingleOrDefault(
            predicate => predicate.Name == "=");
        InitializeMembership();
        DerivedPlan = DerivedPredicatePlan.Create(this);
    }

    public Domain(
        string name,
        IReadOnlyList<Predicate<Fluent>> fluents,
        IReadOnlyList<Predicate<Static>> statics,
        IReadOnlyList<DerivedPredicateDefinition> derivedDefinitions,
        IReadOnlyList<NumericFunction>? functions,
        IReadOnlyList<Constant> constants,
        IReadOnlyList<ActionSchema> actions,
        IReadOnlyDictionary<string, string> typeHierarchy,
        IReadOnlyList<string>? requirements = null)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentNullException.ThrowIfNull(fluents);
        ArgumentNullException.ThrowIfNull(statics);
        ArgumentNullException.ThrowIfNull(derivedDefinitions);
        ArgumentNullException.ThrowIfNull(constants);
        ArgumentNullException.ThrowIfNull(actions);
        ArgumentNullException.ThrowIfNull(typeHierarchy);
        RejectNullElements(fluents, nameof(fluents));
        RejectNullElements(statics, nameof(statics));
        RejectNullElements(derivedDefinitions, nameof(derivedDefinitions));
        RejectNullElements(functions ?? Array.Empty<NumericFunction>(), nameof(functions));
        RejectNullElements(constants, nameof(constants));
        RejectNullElements(actions, nameof(actions));
        RejectNullElements(requirements ?? Array.Empty<string>(), nameof(requirements));
        if (functions?.Any(function =>
                function.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase)) == true)
        {
            throw new ArgumentException(
                "The built-in total-cost function is reserved for planner bookkeeping and cannot be supplied as a programmatic numeric function.",
                nameof(functions));
        }
        if (requirements?.Any(string.IsNullOrWhiteSpace) == true)
            throw new ArgumentException("Requirements cannot contain blank values.", nameof(requirements));
        if (typeHierarchy.Any(pair =>
                string.IsNullOrWhiteSpace(pair.Key) || string.IsNullOrWhiteSpace(pair.Value)))
        {
            throw new ArgumentException(
                "The type hierarchy cannot contain null or blank type names.",
                nameof(typeHierarchy));
        }
        if (typeHierarchy.Keys.Any(type => string.Equals(type, "object", StringComparison.OrdinalIgnoreCase)))
            throw new ArgumentException("The built-in root type 'object' cannot be declared or reparented.", nameof(typeHierarchy));

        Name = name;
        Requirements = Array.AsReadOnly(requirements?.ToArray() ?? Array.Empty<string>());
        Fluents = Array.AsReadOnly(fluents.ToArray());
        if (statics.Any(predicate => predicate.Name.Equals("=", StringComparison.OrdinalIgnoreCase)))
            throw new ArgumentException("The built-in equality predicate cannot be supplied explicitly.", nameof(statics));

        bool equalityEnabled = requirements?.Any(requirement =>
            requirement.Equals(":equality", StringComparison.OrdinalIgnoreCase)
            || requirement.Equals(":adl", StringComparison.OrdinalIgnoreCase)) == true;
        Predicate<Static>? effectiveEqualityPredicate = equalityEnabled
            ? new Predicate<Static>("=", new[]
            {
                new Variable("?left", "object"),
                new Variable("?right", "object"),
            })
            : null;
        Statics = Array.AsReadOnly(statics
            .Concat(effectiveEqualityPredicate is null
                ? Array.Empty<Predicate<Static>>()
                : new[] { effectiveEqualityPredicate })
            .ToArray());
        Derived = Array.AsReadOnly(derivedDefinitions.Select(definition => definition.Predicate).ToArray());
        Functions = Array.AsReadOnly(functions?.ToArray() ?? Array.Empty<NumericFunction>());
        Constants = Array.AsReadOnly(constants.ToArray());
        Actions = Array.AsReadOnly(actions.ToArray());
        TypeHierarchy = new ReadOnlyDictionary<string, string>(
            typeHierarchy.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        _compatibleParentTypes = BuildTypeCompatibility(TypeHierarchy);

        var definitionsByName = new Dictionary<string, IGroundedExpression>(StringComparer.OrdinalIgnoreCase);
        foreach (DerivedPredicateDefinition definition in derivedDefinitions)
        {
            if (!definitionsByName.TryAdd(definition.Predicate.Name, definition.Body))
                throw new ArgumentException(
                    $"Derived predicate '{definition.Predicate.Name}' is defined more than once.",
                    nameof(derivedDefinitions));
        }

        DerivedDefinitions = new ReadOnlyDictionary<string, IGroundedExpression>(definitionsByName);
        EqualityPredicate = effectiveEqualityPredicate;
        InitializeMembership();

        DomainDefinition pddlDefinition = CreatePddlDefinition();
        DerivedDependencyValidator.Validate(DerivedDefinitions);
        PddlDefinition = ValidateProgrammaticDefinition(pddlDefinition);
        ExpandedRequirements = ExpandRequirements(PddlDefinition.Requirements);
        UsesTyping = PddlDefinition.Requirements.HasRequirement(PddlRequirement.Typing);
        UsesEquality = PddlDefinition.Requirements.HasRequirement(PddlRequirement.Equality);
        UsesConditionalEffects = PddlDefinition.Requirements.HasRequirement(PddlRequirement.ConditionalEffects);
        DerivedPlan = DerivedPredicatePlan.Create(this);
    }

    private static IReadOnlyList<string> ExpandRequirements(IEnumerable<PddlRequirement> requirements)
    {
        PddlRequirement[] declaredRequirements = requirements.ToArray();
        return Array.AsReadOnly(Enum.GetValues<PddlRequirement>()
            .Where(declaredRequirements.HasRequirement)
            .Select(requirement => requirement.ToPddlString())
            .ToArray());
    }

    private static void RejectNullElements<T>(IEnumerable<T> values, string parameterName)
        where T : class
    {
        if (values.Any(value => value is null))
            throw new ArgumentException("The collection cannot contain null values.", parameterName);
    }

    private void InitializeMembership()
    {
        foreach (Predicate predicate in Fluents.Cast<Predicate>().Concat(Statics).Concat(Derived))
        {
            _predicateSet.Add(predicate);
            AddVariables(predicate.Parameters);
        }
        foreach (NumericFunction function in Functions)
        {
            _functionSet.Add(function);
            AddVariables(function.Parameters);
        }
        foreach (Constant constant in Constants)
            _constantSet.Add(constant);
        foreach (ActionSchema action in Actions)
        {
            _actionSet.Add(action);
            AddVariables(action.Parameters);
            foreach (ConditionalEffect effect in action.Effects)
                AddVariables(effect.QuantifiedVariables);
        }

        foreach (IGroundedExpression definition in DerivedDefinitions.Values)
            AddExpressionVariables(definition);
    }

    private void AddVariables(IEnumerable<Variable> variables)
    {
        foreach (Variable variable in variables)
            _variableSet.Add(variable);
    }

    private void AddExpressionVariables(IGroundedExpression expression)
    {
        switch (expression)
        {
            case GroundedNot not:
                AddExpressionVariables(not.Expression);
                break;
            case GroundedAnd and:
                foreach (IGroundedExpression child in and.Expressions) AddExpressionVariables(child);
                break;
            case GroundedOr or:
                foreach (IGroundedExpression child in or.Expressions) AddExpressionVariables(child);
                break;
            case GroundedImply imply:
                AddExpressionVariables(imply.Antecedent);
                AddExpressionVariables(imply.Consequent);
                break;
            case GroundedForall forall:
                AddVariables(forall.Variables);
                AddExpressionVariables(forall.Body);
                break;
            case GroundedExists exists:
                AddVariables(exists.Variables);
                AddExpressionVariables(exists.Body);
                break;
        }
    }

    internal bool Contains(Predicate predicate) => _predicateSet.Contains(predicate);
    internal bool IsEqualityPredicate(Predicate predicate)
        => EqualityPredicate is not null && ReferenceEquals(predicate, EqualityPredicate);
    internal bool Contains(NumericFunction function) => _functionSet.Contains(function);
    internal bool Contains(Constant constant) => _constantSet.Contains(constant);
    internal bool Contains(ActionSchema action) => _actionSet.Contains(action);
    internal bool Contains(Variable variable) => _variableSet.Contains(variable);
    internal bool ContainsType(string type)
        => string.Equals(type, "object", StringComparison.OrdinalIgnoreCase)
        || TypeHierarchy.ContainsKey(type);

    private static DomainDefinition ValidateProgrammaticDefinition(DomainDefinition definition)
    {
        try
        {
            DomainDefinition validated = SemanticValidator.ValidateDomain(definition);
            CorePddlSupportValidator.ValidateDomain(validated);
            return validated;
        }
        catch (PddlValidationException exception)
        {
            throw new ArgumentException(exception.Message, exception);
        }
        catch (NotSupportedException exception)
        {
            throw new ArgumentException(exception.Message, exception);
        }
    }

    private DomainDefinition CreatePddlDefinition()
        => ProgrammaticDomainDefinitionBuilder.Build(this);
}
