using System.Collections.Frozen;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Core.Schemas;

public partial class Problem
{
    private static readonly object DynamicVariableRegistration = new();
    internal static readonly IReadOnlySet<Variable> NoVariables = FrozenSet<Variable>.Empty;

    private readonly ConditionalWeakTable<Variable, object> _dynamicVariables = new();
    private readonly Lazy<IApplicableActionGenerator> _initialActionGenerator;
    internal const int StateGeneratorCacheCapacity = 16;
    private readonly ConditionalWeakTable<State, IApplicableActionGenerator> _stateGenerators = new();
    private readonly Queue<WeakReference<State>> _stateGeneratorOrder = new();

    public string Name { get; }
    public Domain Domain { get; }
    public InstanceContext Context { get; internal set; } = null!;
    public IReadOnlyList<string> Requirements { get; }
    public IReadOnlyList<Constant> DeclaredObjects { get; }
    public IReadOnlyList<Constant> AllObjects { get; }
    public IReadOnlyDictionary<string, Constant> ObjectLookup { get; }
    public ApplicableActionGeneratorType GeneratorType { get; }

    internal readonly List<Fact<Fluent>> _initialFluentFacts = new();
    internal readonly List<Fact<Static>> _initialStaticFacts = new();
    private State? _initialState;

    public State InitialState
    {
        get
        {
            if (_initialState == null)
                _initialState = BuildInitialState();

            return _initialState;
        }
    }

    public IReadOnlyList<Literal<Fact>> Goal { get; }
    public IReadOnlyList<GroundNumericComparison> NumericGoals { get; }
    public bool HasNumericPlanning => NumericGoals.Count > 0 || Domain.DerivedPlan.HasNumericConditions || Domain.Actions.Any(action => action.HasNumericConditionsOrEffects);
    internal void RequirePropositionalPlanning(string component)
    {
        if (HasNumericPlanning) throw new NotSupportedException($"Numeric planning is not supported by {component}.");
    }
    public IReadOnlyDictionary<string, Predicate> AllPredicates { get; }

    public static Problem FromFile(
        Domain domain,
        string problemFilePath,
        ApplicableActionGeneratorType generatorType = ApplicableActionGeneratorType.Grounded)
    {
        return new Problem(domain, ParseProblemFile(domain, problemFilePath), problemFilePath, generatorType);
    }

    public static Problem FromText(
        Domain domain,
        string pddlText,
        ApplicableActionGeneratorType generatorType = ApplicableActionGeneratorType.Grounded)
    {
        ArgumentNullException.ThrowIfNull(domain);
        ArgumentNullException.ThrowIfNull(pddlText);

        ProblemDefinition definition = ParseProblemText(pddlText, sourcePath: null);
        return new Problem(domain, definition, sourcePath: null, generatorType);
    }

    private static ProblemDefinition ParseProblemFile(Domain domain, string filePath)
    {
        ArgumentNullException.ThrowIfNull(domain);
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);

        string text = File.ReadAllText(filePath);
        return ParseProblemText(text, filePath);
    }

    private static ProblemDefinition ParseProblemText(string text, string? sourcePath)
    {
        return PddlLoading.Execute(
            PddlDocumentType.Problem,
            sourcePath,
            () => PddlParser.ParseProblem(text));
    }

    internal Problem(Domain domain, ProblemDefinition astProblem)
        : this(domain, astProblem, sourcePath: null, ApplicableActionGeneratorType.Grounded)
    {
    }

    internal static Problem CreateProgrammatic(
        Domain domain,
        ProblemDefinition definition,
        ApplicableActionGeneratorType generatorType)
        => new(domain, definition, sourcePath: null, generatorType, isProgrammatic: true);

    private Problem(
        Domain domain,
        ProblemDefinition astProblem,
        string? sourcePath,
        ApplicableActionGeneratorType generatorType,
        bool isProgrammatic = false)
    {
        ArgumentNullException.ThrowIfNull(domain);
        ArgumentNullException.ThrowIfNull(astProblem);
        if (!Enum.IsDefined(generatorType))
            throw new ArgumentOutOfRangeException(nameof(generatorType), generatorType, null);

        ProblemDefinition validatedProblem = isProgrammatic
            ? ValidateProgrammaticDefinition(domain, astProblem)
            : PddlLoading.Execute(
                PddlDocumentType.Problem,
                sourcePath,
                () => ValidateDefinition(domain, astProblem));

        Domain = domain;
        Name = validatedProblem.Name;
        Requirements = Array.AsReadOnly(
            validatedProblem.Requirements.Select(requirement => requirement.ToPddlString()).ToArray());
        GeneratorType = generatorType;

        PddlProblemTranslator builder = isProgrammatic
            ? new PddlProblemTranslator(domain, this, validatedProblem)
            : PddlLoading.Execute(
                PddlDocumentType.Problem,
                sourcePath,
                () => new PddlProblemTranslator(domain, this, validatedProblem));

        Context = builder.Context;
        DeclaredObjects = Array.AsReadOnly(builder.DeclaredObjects.ToArray());
        AllObjects = Array.AsReadOnly(builder.AllObjects.ToArray());
        ObjectLookup = new ReadOnlyDictionary<string, Constant>(
            builder.ObjectLookup.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        AllPredicates = new ReadOnlyDictionary<string, Predicate>(
            builder.AllPredicates.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        Goal = Array.AsReadOnly(builder.Goal.ToArray());
        var bindings = new Dictionary<Variable, Constant>();
        NumericGoals = Array.AsReadOnly(builder.NumericGoals.Select(comparison => new GroundNumericComparison(
            comparison.Left.Ground(this, bindings), comparison.Operator, comparison.Right.Ground(this, bindings))).ToArray());
        _initialActionGenerator = new Lazy<IApplicableActionGenerator>(
            CreateInitialActionGenerator,
            LazyThreadSafetyMode.ExecutionAndPublication);
    }

    private static ProblemDefinition ValidateDefinition(Domain domain, ProblemDefinition definition)
    {
        ProblemDefinition validated = SemanticValidator.ValidateProblem(domain.PddlDefinition, definition);
        CorePddlSupportValidator.ValidateProblem(domain.PddlDefinition, validated);
        return validated;
    }

    private static ProblemDefinition ValidateProgrammaticDefinition(
        Domain domain,
        ProblemDefinition definition)
    {
        try
        {
            return ValidateDefinition(domain, definition);
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

    public IApplicableActionGenerator GetApplicableActionGenerator(State startState)
    {
        ArgumentNullException.ThrowIfNull(startState);
        if (!ReferenceEquals(startState.Context, Context))
            throw new ArgumentException("State belongs to a different problem.", nameof(startState));

        if (GeneratorType == ApplicableActionGeneratorType.Lifted || startState.Equals(InitialState))
            return _initialActionGenerator.Value;

        if (_stateGenerators.TryGetValue(startState, out IApplicableActionGenerator? cached))
            return cached;

        var generator = new GroundedApplicableActionGenerator(this, startState, new RpgGrounder());
        while (_stateGeneratorOrder.Count >= StateGeneratorCacheCapacity)
        {
            if (_stateGeneratorOrder.Dequeue().TryGetTarget(out State? oldest))
                _stateGenerators.Remove(oldest);
        }

        // Both the keys and FIFO bookkeeping must allow unused states to be collected.
        _stateGenerators.Add(startState, generator);
        _stateGeneratorOrder.Enqueue(new WeakReference<State>(startState));
        return generator;
    }

    private IApplicableActionGenerator CreateInitialActionGenerator()
        => GeneratorType switch
        {
            ApplicableActionGeneratorType.Grounded =>
                new GroundedApplicableActionGenerator(this, InitialState, new RpgGrounder()),
            ApplicableActionGeneratorType.Lifted => new CliqueApplicableActionGenerator(this),
            _ => throw new InvalidOperationException($"Unknown generator type '{GeneratorType}'."),
        };

    // -------- Factory helpers for runtime construction --------

    /// <summary>
    /// Create a new lifted variable. The returned variable is not tracked by
    /// any predicate or action — it's intended for ad-hoc condition building
    /// (e.g. <see cref="ConjunctiveCondition"/>).
    /// </summary>
    public Variable NewVariable(string name, string type = "object")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(type);
        if (!Domain.ContainsType(type))
            throw new ArgumentException($"Type '{type}' is not declared by the domain.", nameof(type));

        var variable = new Variable(name, type);
        _dynamicVariables.Add(variable, DynamicVariableRegistration);
        return variable;
    }

    /// <summary>
    /// Construct an atom for the given predicate and terms.
    /// </summary>
    public Atom<T> NewAtom<T>(Predicate<T> predicate, IReadOnlyList<ITerm> terms) where T : IPredicateType
    {
        ValidateAtom(predicate, terms, nameof(predicate), nameof(terms));
        return new Atom<T>(predicate, terms);
    }

    /// <summary>
    /// Construct a lifted literal.
    /// </summary>
    public Literal<Atom<T>> NewLiteral<T>(Atom<T> atom, bool isPositive) where T : IPredicateType
    {
        ArgumentNullException.ThrowIfNull(atom);
        ValidateAtom(atom.Predicate, atom.Arguments, nameof(atom), nameof(atom));
        return new Literal<Atom<T>>(atom, isPositive ? Polarity.Positive : Polarity.Negative);
    }

    /// <summary>
    /// Construct a ground literal from a fact and polarity.
    /// </summary>
    public Literal<Fact<T>> NewGroundLiteral<T>(Fact<T> fact, bool isPositive) where T : IPredicateType
    {
        ArgumentNullException.ThrowIfNull(fact);
        if (!ReferenceEquals(fact.Context, Context))
            throw new ArgumentException("Fact belongs to a different problem.", nameof(fact));
        return new Literal<Fact<T>>(fact, isPositive ? Polarity.Positive : Polarity.Negative);
    }

    public Mimir.Core.Grounding.Action GroundAction(ActionSchema schema, params Constant[] arguments)
        => ActionBuilder.BuildAction(schema, arguments, this);

    public GroundConjunctiveCondition GroundCondition(IEnumerable<Literal<Fact>> literals,
        IReadOnlyList<GroundNumericComparison> numericConditions)
    {
        ArgumentNullException.ThrowIfNull(literals);
        Literal<Fact>[] snapshot = literals.ToArray();
        foreach (Literal<Fact> literal in snapshot)
        {
            ArgumentNullException.ThrowIfNull(literal);
            if (!ReferenceEquals(literal.Value.Context, Context)) throw new ArgumentException("Literal belongs to a different problem.", nameof(literals));
        }
        ArgumentNullException.ThrowIfNull(numericConditions);
        GroundNumericComparison[] comparisonSnapshot = numericConditions.ToArray();
        foreach (GroundNumericComparison comparison in comparisonSnapshot)
            ValidateNumericComparison(comparison, NoVariables, nameof(numericConditions));
        return new GroundConjunctiveCondition(this, snapshot, comparisonSnapshot);
    }

    /// <summary>
    /// Build a conjunctive condition from a list of variables and a mixed list
    /// of typed literals. Each literal must be a <c>Literal&lt;Atom&lt;T&gt;&gt;</c>
    /// for T ∈ {Static, Fluent, Derived}.
    /// </summary>
    public ConjunctiveCondition NewConjunctiveCondition(
        IReadOnlyList<Variable> parameters,
        IEnumerable<Literal> literals,
        IReadOnlyList<NumericComparison> numericConditions)
    {
        ArgumentNullException.ThrowIfNull(parameters);
        ArgumentNullException.ThrowIfNull(literals);
        ArgumentNullException.ThrowIfNull(numericConditions);

        var parameterSet = new HashSet<Variable>(ReferenceEqualityComparer.Instance);
        foreach (Variable parameter in parameters)
        {
            if (parameter is null)
                throw new ArgumentException("Condition parameters cannot contain null values.", nameof(parameters));
            if (!ContainsDynamicVariable(parameter))
                throw new ArgumentException(
                    $"Variable '{parameter.Name}' belongs to a different problem or was not created by this problem.",
                    nameof(parameters));
            if (!parameterSet.Add(parameter))
                throw new ArgumentException($"Variable '{parameter.Name}' occurs more than once.", nameof(parameters));
        }

        Literal[] materializedLiterals = literals.ToArray();
        foreach (Literal literal in materializedLiterals)
        {
            if (literal is null)
                throw new ArgumentException("Condition literals cannot contain null values.", nameof(literals));
            switch (literal)
            {
                case Literal<Atom<Fluent>> fluent:
                    ValidateConditionLiteral(fluent, parameterSet, nameof(literals));
                    break;
                case Literal<Atom<Static>> stat:
                    ValidateConditionLiteral(stat, parameterSet, nameof(literals));
                    break;
                case Literal<Atom<Derived>> derived:
                    ValidateConditionLiteral(derived, parameterSet, nameof(literals));
                    break;
                default:
                    throw new ArgumentException(
                        $"Unsupported condition literal type '{literal.GetType().Name}'.",
                        nameof(literals));
            }
        }

        foreach (NumericComparison comparison in numericConditions)
            ValidateNumericComparison(comparison, parameterSet, nameof(numericConditions));
        return ConjunctiveCondition.Of(this, parameters, materializedLiterals, numericConditions);
    }

    private State BuildInitialState()
    {
        int highestTrueFactIndex = -1;
        foreach (var fact in _initialFluentFacts)
        {
            var fluentIndex = new FluentIndex(fact.LocalIndex);
            if (!ReferenceEquals(fact.Context, Context) || !Context.IsValid(fluentIndex))
            {
                throw new InvalidOperationException(
                    $"Initial fluent fact '{fact}' is not registered in this problem context.");
            }

            highestTrueFactIndex = Math.Max(highestTrueFactIndex, fact.LocalIndex);
        }

        int wordCount = highestTrueFactIndex < 0 ? 0 : highestTrueFactIndex / 64 + 1;
        var bitboard = new ulong[wordCount];
        foreach (var fact in _initialFluentFacts)
        {
            int arrayIndex = fact.LocalIndex / 64;
            int bitIndex = fact.LocalIndex % 64;
            bitboard[arrayIndex] |= 1UL << bitIndex;
        }
        ReadOnlySpan<double> numericValues = Context.NumericLayout.InitialValues;
        return new State(Context, bitboard,
            numericValues.ToArray(), takeOwnership: true);
    }

    internal void ValidateActionBinding(
        ActionSchema schema,
        IReadOnlyList<Constant> arguments,
        string schemaParameterName,
        string argumentsParameterName)
        => Context.ValidateActionBinding(schema, arguments, schemaParameterName, argumentsParameterName);

    internal bool ContainsVariable(Variable variable)
        => ContainsDynamicVariable(variable) || Domain.Contains(variable);

    private bool ContainsDynamicVariable(Variable variable)
        => _dynamicVariables.TryGetValue(variable, out _);

    internal void ValidateNumericComparison(
        NumericComparison comparison,
        IReadOnlySet<Variable> variables,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(comparison, parameterName);
        ValidateNumericExpression(comparison.Left);
        ValidateNumericExpression(comparison.Right);

        void ValidateNumericExpression(NumericExpression expression)
        {
            switch (expression)
            {
                case NumericConstant:
                    return;
                case NumericBinaryExpression binary:
                    ValidateNumericExpression(binary.Left);
                    ValidateNumericExpression(binary.Right);
                    return;
                case GroundFunctionCall call:
                    if (!ReferenceEquals(call.Context, Context))
                        throw new ArgumentException("Numeric expression belongs to a different problem.", parameterName);
                    return;
                case FunctionCall call:
                    ValidateNumericFunctionArguments(call.Function, call.Arguments, variables, parameterName);
                    return;
                default:
                    throw new ArgumentException("Unknown numeric expression.", parameterName);
            }
        }
    }

    internal void ValidateConditionLiteral<T>(
        Literal<Atom<T>> literal,
        IReadOnlySet<Variable> parameters,
        string parameterName)
        where T : IPredicateType
    {
        if (literal is null)
            throw new ArgumentException("Condition literals cannot contain null values.", parameterName);
        if (literal.Polarity is not (Polarity.Positive or Polarity.Negative))
            throw new ArgumentException($"Condition literal has invalid polarity '{literal.Polarity}'.", parameterName);
        if (literal.Value is null)
            throw new ArgumentException("Condition literal values cannot be null.", parameterName);

        ValidateConditionAtom(literal.Value, parameters, parameterName);
    }

    // Ground callers pass an empty variable set; lifted callers pass the variables in scope.
    internal void ValidateNumericFunctionArguments(
        NumericFunction function,
        IReadOnlyList<ITerm> arguments,
        IReadOnlySet<Variable> variables,
        string parameterName)
    {
        ArgumentNullException.ThrowIfNull(function);
        ArgumentNullException.ThrowIfNull(arguments);
        if (!Domain.Contains(function))
            throw new ArgumentException(
                $"Numeric function '{function.Name}' belongs to a different domain.",
                parameterName);
        if (function.Parameters.Count != arguments.Count)
            throw new ArgumentException(
                $"Numeric function '{function.Name}' expects {function.Parameters.Count} arguments, got {arguments.Count}.",
                parameterName);

        for (int i = 0; i < arguments.Count; i++)
        {
            string type = arguments[i] switch
            {
                null => throw new ArgumentException("Arguments cannot contain null values.", parameterName),
                Constant constant when Context.ContainsObject(constant) => constant.Type,
                Variable variable when variables.Contains(variable) => variable.Type,
                ITerm term => throw new ArgumentException(
                    $"Argument '{term}' of '{function.Name}' is not an object of this problem or a variable in scope.",
                    parameterName)
            };
            if (!Domain.IsCompatible(type, function.Parameters[i].Type))
                throw new ArgumentException(
                    $"Argument '{arguments[i]}' has type '{type}', expected '{function.Parameters[i].Type}'.",
                    parameterName);
        }
    }

    private void ValidateAtom(
        Predicate predicate,
        IReadOnlyList<ITerm> terms,
        string predicateParameterName,
        string termsParameterName)
    {
        ArgumentNullException.ThrowIfNull(predicate, predicateParameterName);
        ArgumentNullException.ThrowIfNull(terms, termsParameterName);
        if (!Domain.Contains(predicate))
            throw new ArgumentException(
                $"Predicate '{predicate.Name}' belongs to a different domain.",
                predicateParameterName);
        if (predicate.Parameters.Count != terms.Count)
            throw new ArgumentException(
                $"Predicate '{predicate.Name}' expects {predicate.Parameters.Count} terms, got {terms.Count}.",
                termsParameterName);

        for (int i = 0; i < terms.Count; i++)
        {
            ITerm term = terms[i]
                ?? throw new ArgumentException("Terms cannot contain null values.", termsParameterName);
            string termType = term switch
            {
                Constant constant when Context.ContainsObject(constant) => constant.Type,
                Constant constant => throw new ArgumentException(
                    $"Object '{constant.Name}' belongs to a different problem.", termsParameterName),
                Variable variable when ContainsDynamicVariable(variable) => variable.Type,
                Variable variable => throw new ArgumentException(
                    $"Variable '{variable.Name}' belongs to a different problem or was not created by this problem.",
                    termsParameterName),
                _ => throw new ArgumentException($"Unsupported term type '{term.GetType().Name}'.", termsParameterName)
            };

            if (!Domain.IsCompatible(termType, predicate.Parameters[i].Type))
                throw new ArgumentException(
                    $"Term '{term.Name}' has type '{termType}', expected '{predicate.Parameters[i].Type}'.",
                    termsParameterName);
        }
    }

    private void ValidateConditionAtom(
        Atom atom,
        IReadOnlySet<Variable> parameters,
        string parameterName)
    {
        if (!Domain.Contains(atom.Predicate))
            throw new ArgumentException(
                $"Predicate '{atom.Predicate.Name}' belongs to a different domain.",
                parameterName);
        if (atom.Predicate.Parameters.Count != atom.Arguments.Count)
            throw new ArgumentException(
                $"Predicate '{atom.Predicate.Name}' has the wrong arity.",
                parameterName);

        for (int i = 0; i < atom.Arguments.Count; i++)
        {
            ITerm term = atom.Arguments[i];
            string type = term switch
            {
                Constant constant when Context.ContainsObject(constant) => constant.Type,
                Constant constant => throw new ArgumentException(
                    $"Object '{constant.Name}' belongs to a different problem.", parameterName),
                Variable variable when parameters.Contains(variable) => variable.Type,
                Variable variable => throw new ArgumentException(
                    $"Variable '{variable.Name}' is not a parameter of this condition.", parameterName),
                _ => throw new ArgumentException($"Unsupported term type '{term.GetType().Name}'.", parameterName)
            };

            if (!Domain.IsCompatible(type, atom.Predicate.Parameters[i].Type))
                throw new ArgumentException(
                    $"Term '{term.Name}' has type '{type}', expected '{atom.Predicate.Parameters[i].Type}'.",
                    parameterName);
        }
    }
}
