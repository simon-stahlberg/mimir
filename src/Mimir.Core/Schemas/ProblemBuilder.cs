using System.Collections.Immutable;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using PddlTerm = Mimir.Pddl.Ast.Models.Term;

namespace Mimir.Core.Schemas;

public sealed class ProblemBuilder
{
    private const int ObjectStage = 0;
    private const int InitialStateStage = 1;
    private const int GoalStage = 2;

    private readonly Domain _domain;
    private readonly string _name;
    private readonly ApplicableActionGeneratorType _generatorType;
    private readonly IReadOnlyDictionary<string, Predicate> _predicates;
    private readonly IReadOnlyDictionary<string, NumericFunction> _functions;
    private IReadOnlyList<BuilderTypedNameSpec> _objects = Array.Empty<BuilderTypedNameSpec>();
    private IReadOnlyList<BuilderProblemFactSpec> _initialFacts = Array.Empty<BuilderProblemFactSpec>();
    private IReadOnlyList<BuilderProblemNumericSpec> _numericInitializations =
        Array.Empty<BuilderProblemNumericSpec>();
    private IReadOnlyList<BuilderProblemGoalSpec> _goals = Array.Empty<BuilderProblemGoalSpec>();
    private IReadOnlyDictionary<string, BuilderTypedNameSpec> _objectsByName;
    private IReadOnlyList<LogicalExpressionSpec> _goalExpressions = Array.Empty<LogicalExpressionSpec>();
    private object? _activeChild;
    private int _nextStage;
    private bool _built;

    public ProblemBuilder(
        Domain domain,
        string name,
        ApplicableActionGeneratorType generatorType = ApplicableActionGeneratorType.Grounded)
    {
        ArgumentNullException.ThrowIfNull(domain);
        if (!Enum.IsDefined(generatorType))
            throw new ArgumentOutOfRangeException(nameof(generatorType), generatorType, null);

        _domain = domain;
        _name = BuilderName.RequireName(name, nameof(name));
        _generatorType = generatorType;
        _predicates = domain.Fluents.Cast<Predicate>()
            .Concat(domain.Statics)
            .Concat(domain.Derived)
            .ToDictionary(predicate => predicate.Name, StringComparer.OrdinalIgnoreCase);
        _functions = domain.Functions.ToDictionary(function => function.Name, StringComparer.OrdinalIgnoreCase);
        _objectsByName = domain.Constants
            .Select(constant => new BuilderTypedNameSpec(constant.Name, constant.Type))
            .ToDictionary(constant => constant.Name, StringComparer.OrdinalIgnoreCase);
    }

    public ProblemObjectListBuilder Objects()
        => OpenSection(ObjectStage, () => new ProblemObjectListBuilder(this));

    public InitialStateBuilder InitialState()
        => OpenSection(InitialStateStage, () => new InitialStateBuilder(this));

    public GoalBuilder Goal()
        => OpenSection(GoalStage, () => new GoalBuilder(this));

    public Problem Build()
    {
        EnsureOpen();
        if (_activeChild is not null)
            throw new InvalidOperationException("The open problem section must be closed before building the problem.");

        try
        {
            ProblemDefinition definition = ProgrammaticProblemCompiler.Compile(
                _domain,
                _name,
                _objects,
                _initialFacts,
                _numericInitializations,
                _goals, _goalExpressions);
            Problem problem = Problem.CreateProgrammatic(
                _domain,
                definition,
                _generatorType);
            _built = true;
            return problem;
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

    internal void CommitObjects(
        ProblemObjectListBuilder child,
        IReadOnlyList<BuilderTypedNameSpec> objects)
    {
        EnsureActiveChild(child);
        BuilderTypedNameSpec[] copy = objects.ToArray();
        _objects = Array.AsReadOnly(copy);
        _objectsByName = _domain.Constants
            .Select(constant => new BuilderTypedNameSpec(constant.Name, constant.Type))
            .Concat(copy)
            .ToDictionary(constant => constant.Name, StringComparer.OrdinalIgnoreCase);
        _activeChild = null;
    }

    internal void CommitInitialState(
        InitialStateBuilder child,
        IReadOnlyList<BuilderProblemFactSpec> facts,
        IReadOnlyList<BuilderProblemNumericSpec> numericInitializations)
    {
        EnsureActiveChild(child);
        _initialFacts = Array.AsReadOnly(facts.ToArray());
        _numericInitializations = Array.AsReadOnly(numericInitializations.ToArray());
        _activeChild = null;
    }

    internal void CommitGoal(GoalBuilder child, IReadOnlyList<BuilderProblemGoalSpec> goals, IReadOnlyList<LogicalExpressionSpec> expressions)
    {
        EnsureActiveChild(child);
        _goals = Array.AsReadOnly(goals.ToArray());
        _goalExpressions = Array.AsReadOnly(expressions.ToArray());
        _activeChild = null;
    }

    internal void ValidateObject(string name, string type)
    {
        if (!_domain.ContainsType(type))
            throw new ArgumentException($"Type '{type}' is not declared by the domain.", nameof(type));
        if (_domain.Constants.Any(constant => constant.Name.Equals(name, StringComparison.OrdinalIgnoreCase)))
        {
            throw new ArgumentException(
                $"An object or domain constant named '{name}' already exists.",
                nameof(name));
        }
    }

    internal void ValidateInitialFact(string predicateName, IReadOnlyList<string> arguments)
    {
        Predicate predicate = RequirePredicate(predicateName);
        if (predicate is Predicate<Derived> || _domain.IsEqualityPredicate(predicate))
        {
            throw new ArgumentException(
                $"Predicate '{predicateName}' cannot appear in the initial state.",
                nameof(predicateName));
        }

        ValidateArguments(predicate.Parameters, arguments, predicateName);
    }

    internal void ValidateNumericInitialization(
        string functionName,
        IReadOnlyList<string> arguments)
    {
        if (!_functions.TryGetValue(functionName, out NumericFunction? function))
        {
            throw new ArgumentException(
                $"Numeric function '{functionName}' is not declared by the domain.",
                nameof(functionName));
        }

        ValidateArguments(function.Parameters, arguments, functionName);
    }

    internal void ValidateGoal(
        string predicateName,
        IReadOnlyList<string> arguments)
    {
        Predicate predicate = RequirePredicate(predicateName);
        ValidateArguments(predicate.Parameters, arguments, predicateName);
    }

    private T OpenSection<T>(int stage, Func<T> create)
        where T : class
    {
        EnsureOpen();
        if (_activeChild is not null)
            throw new InvalidOperationException("Close the current problem section before opening another section.");
        if (stage < _nextStage)
            throw new InvalidOperationException("Problem sections cannot be reopened or added out of order.");

        T child = create();
        _nextStage = stage + 1;
        _activeChild = child;
        return child;
    }

    private void EnsureOpen()
    {
        if (_built)
            throw new InvalidOperationException("The problem builder has already built a problem.");
    }

    private void EnsureActiveChild(object child)
    {
        EnsureOpen();
        if (!ReferenceEquals(_activeChild, child))
            throw new InvalidOperationException("The problem section is not the active section.");
    }

    private Predicate RequirePredicate(string name)
    {
        if (_predicates.TryGetValue(name, out Predicate? predicate)) return predicate;
        throw new ArgumentException($"Predicate '{name}' is not declared by the domain.", nameof(name));
    }

    private void ValidateArguments(
        IReadOnlyList<Variable> parameters,
        IReadOnlyList<string> arguments,
        string symbolName)
    {
        if (parameters.Count != arguments.Count)
        {
            throw new ArgumentException(
                $"Symbol '{symbolName}' expects {parameters.Count} arguments, got {arguments.Count}.",
                nameof(arguments));
        }

        for (int index = 0; index < arguments.Count; index++)
        {
            string argument = arguments[index];
            if (argument.StartsWith('?'))
                throw new ArgumentException("Problem facts and goals cannot contain variables.", nameof(arguments));
            if (!_objectsByName.TryGetValue(argument, out BuilderTypedNameSpec? value))
            {
                throw new ArgumentException(
                    $"Object or domain constant '{argument}' is not declared.",
                    nameof(arguments));
            }
            if (!_domain.IsCompatible(value.Type, parameters[index].Type))
            {
                throw new ArgumentException(
                    $"Object '{argument}' has type '{value.Type}', expected '{parameters[index].Type}'.",
                    nameof(arguments));
            }
        }
    }
}
