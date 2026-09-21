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

    internal readonly Dictionary<NumericFunctionKey, double> _numericFunctionValues = new();
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
        ProgrammaticProblemInputs inputs,
        ApplicableActionGeneratorType generatorType)
    {
        ArgumentNullException.ThrowIfNull(inputs);
        return new Problem(domain, definition, sourcePath: null, generatorType, inputs);
    }

    private Problem(
        Domain domain,
        ProblemDefinition astProblem,
        string? sourcePath,
        ApplicableActionGeneratorType generatorType,
        ProgrammaticProblemInputs? programmaticInputs = null)
    {
        ArgumentNullException.ThrowIfNull(domain);
        ArgumentNullException.ThrowIfNull(astProblem);
        if (!Enum.IsDefined(generatorType))
            throw new ArgumentOutOfRangeException(nameof(generatorType), generatorType, null);

        ProblemDefinition validatedProblem = programmaticInputs is null
            ? PddlLoading.Execute(
                PddlDocumentType.Problem,
                sourcePath,
                () => ValidateDefinition(domain, astProblem))
            : ValidateProgrammaticDefinition(domain, astProblem);

        Domain = domain;
        Name = validatedProblem.Name;
        Requirements = Array.AsReadOnly(
            validatedProblem.Requirements.Select(requirement => requirement.ToPddlString()).ToArray());
        GeneratorType = generatorType;

        PddlProblemTranslator builder = programmaticInputs is null
            ? PddlLoading.Execute(
                PddlDocumentType.Problem,
                sourcePath,
                () => new PddlProblemTranslator(domain, this, validatedProblem))
            : new PddlProblemTranslator(domain, this, validatedProblem, programmaticInputs);

        Context = builder.Context;
        DeclaredObjects = Array.AsReadOnly(builder.DeclaredObjects.ToArray());
        AllObjects = Array.AsReadOnly(builder.AllObjects.ToArray());
        ObjectLookup = new ReadOnlyDictionary<string, Constant>(
            builder.ObjectLookup.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        AllPredicates = new ReadOnlyDictionary<string, Predicate>(
            builder.AllPredicates.ToDictionary(pair => pair.Key, pair => pair.Value, StringComparer.OrdinalIgnoreCase));
        Goal = Array.AsReadOnly(builder.Goal.ToArray());
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

    public double GetNumericFunctionValue(NumericFunction function, IReadOnlyList<Constant> arguments)
    {
        ArgumentNullException.ThrowIfNull(function);
        ArgumentNullException.ThrowIfNull(arguments);
        if (function.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
        {
            throw new NotSupportedException(
                "Cumulative total-cost is planner bookkeeping and cannot be queried as a problem numeric function value.");
        }

        ValidateNumericFunctionArguments(function, arguments);
        if (_numericFunctionValues.TryGetValue(new NumericFunctionKey(function, arguments), out double value))
            return value;

        throw new InvalidOperationException($"Numeric function '{function.Name}' is missing an initialization for the provided arguments.");
    }

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

    /// <summary>
    /// Build a conjunctive condition from a list of variables and a mixed list
    /// of typed literals. Each literal must be a <c>Literal&lt;Atom&lt;T&gt;&gt;</c>
    /// for T ∈ {Static, Fluent, Derived}.
    /// </summary>
    public ConjunctiveCondition NewConjunctiveCondition(
        IReadOnlyList<Variable> parameters,
        IEnumerable<Literal> literals)
    {
        ArgumentNullException.ThrowIfNull(parameters);
        ArgumentNullException.ThrowIfNull(literals);

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

        return ConjunctiveCondition.Of(this, parameters, materializedLiterals);
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
        return new State(Context, bitboard);
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

    internal void ValidateNumericFunctionArguments(
        NumericFunction function,
        IReadOnlyList<Constant> arguments)
    {
        ArgumentNullException.ThrowIfNull(function);
        ArgumentNullException.ThrowIfNull(arguments);
        if (!Domain.Contains(function))
            throw new ArgumentException(
                $"Numeric function '{function.Name}' belongs to a different domain.",
                nameof(function));
        if (function.Parameters.Count != arguments.Count)
            throw new ArgumentException(
                $"Numeric function '{function.Name}' expects {function.Parameters.Count} arguments, got {arguments.Count}.",
                nameof(arguments));

        for (int i = 0; i < arguments.Count; i++)
        {
            Constant argument = arguments[i]
                ?? throw new ArgumentException("Arguments cannot contain null values.", nameof(arguments));
            if (!Context.ContainsObject(argument))
                throw new ArgumentException(
                    $"Object '{argument.Name}' belongs to a different problem.",
                    nameof(arguments));
            if (!Domain.IsCompatible(argument.Type, function.Parameters[i].Type))
                throw new ArgumentException(
                    $"Object '{argument.Name}' has type '{argument.Type}', expected '{function.Parameters[i].Type}'.",
                    nameof(arguments));
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
