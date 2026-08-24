using Mimir.Core.Grounding;

namespace Mimir.Core.Schemas;

public sealed class ProblemObjectListBuilder
{
    private readonly ProblemBuilder _parent;
    private readonly List<BuilderTypedNameSpec> _objects = new();
    private readonly HashSet<string> _names = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal ProblemObjectListBuilder(ProblemBuilder parent)
    {
        _parent = parent;
    }

    public ProblemObjectListBuilder Add(string name, string type = "object")
    {
        EnsureOpen();
        name = BuilderName.RequireName(name, nameof(name));
        type = BuilderName.RequireName(type, nameof(type));
        _parent.ValidateObject(name, type);
        if (!_names.Add(name))
            throw new ArgumentException($"Object '{name}' was added more than once.", nameof(name));

        _objects.Add(new BuilderTypedNameSpec(name, type));
        return this;
    }

    public ProblemBuilder Close()
    {
        EnsureOpen();
        _parent.CommitObjects(this, _objects);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The problem object list builder is closed.");
    }
}

public sealed class InitialStateBuilder
{
    private readonly ProblemBuilder _parent;
    private readonly List<BuilderProblemFactSpec> _facts = new();
    private readonly List<BuilderProblemNumericSpec> _numericInitializations = new();
    private readonly HashSet<string> _numericKeys = new(StringComparer.OrdinalIgnoreCase);
    private bool _closed;

    internal InitialStateBuilder(ProblemBuilder parent)
    {
        _parent = parent;
    }

    public InitialStateBuilder AddFact(string predicateName, params string[] arguments)
    {
        EnsureOpen();
        predicateName = BuilderName.RequirePredicateReference(predicateName, nameof(predicateName));
        string[] copy = BuilderName.CopyTerms(arguments, nameof(arguments));
        _parent.ValidateInitialFact(predicateName, copy);
        _facts.Add(new BuilderProblemFactSpec(predicateName, copy));
        return this;
    }

    public InitialStateBuilder AddNumericInitialization(
        string functionName,
        double value,
        params string[] arguments)
    {
        EnsureOpen();
        functionName = BuilderName.RequireName(functionName, nameof(functionName));
        if (functionName.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
        {
            throw new ArgumentException(
                "The built-in total-cost function is reserved for planner bookkeeping.",
                nameof(functionName));
        }

        string[] copy = BuilderName.CopyTerms(arguments, nameof(arguments));
        decimal pddlValue = BuilderName.ToPddlNumber(value, nameof(value));
        _parent.ValidateNumericInitialization(functionName, copy);
        string key = string.Join('\0', new[] { functionName }.Concat(copy));
        if (!_numericKeys.Add(key))
        {
            throw new ArgumentException(
                $"Numeric function '{functionName}' was initialized more than once for the same arguments.",
                nameof(arguments));
        }

        _numericInitializations.Add(
            new BuilderProblemNumericSpec(functionName, copy, value, pddlValue));
        return this;
    }

    public ProblemBuilder Close()
    {
        EnsureOpen();
        _parent.CommitInitialState(this, _facts, _numericInitializations);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The initial state builder is closed.");
    }
}

public sealed class GoalBuilder
{
    private readonly ProblemBuilder _parent;
    private readonly List<BuilderProblemGoalSpec> _goals = new();
    private bool _closed;

    internal GoalBuilder(ProblemBuilder parent)
    {
        _parent = parent;
    }

    public GoalBuilder Add(string predicateName, params string[] arguments)
        => Add(predicateName, Polarity.Positive, arguments);

    public GoalBuilder Add(
        string predicateName,
        Polarity polarity,
        params string[] arguments)
    {
        EnsureOpen();
        predicateName = BuilderName.RequirePredicateReference(predicateName, nameof(predicateName));
        if (!Enum.IsDefined(polarity))
            throw new ArgumentOutOfRangeException(nameof(polarity), polarity, null);
        string[] copy = BuilderName.CopyTerms(arguments, nameof(arguments));
        _parent.ValidateGoal(predicateName, copy);
        _goals.Add(new BuilderProblemGoalSpec(predicateName, polarity, copy));
        return this;
    }

    public ProblemBuilder Close()
    {
        EnsureOpen();
        _parent.CommitGoal(this, _goals);
        _closed = true;
        return _parent;
    }

    private void EnsureOpen()
    {
        if (_closed) throw new InvalidOperationException("The goal builder is closed.");
    }
}

internal sealed record BuilderProblemFactSpec(
    string PredicateName,
    IReadOnlyList<string> Arguments);

internal sealed record BuilderProblemGoalSpec(
    string PredicateName,
    Polarity Polarity,
    IReadOnlyList<string> Arguments);

internal sealed record BuilderProblemNumericSpec(
    string FunctionName,
    IReadOnlyList<string> Arguments,
    double Value,
    decimal PddlValue);

internal sealed class ProgrammaticProblemInputs
{
    internal IReadOnlyList<BuilderProblemNumericSpec> NumericInitializations { get; }

    internal ProgrammaticProblemInputs(
        IReadOnlyList<BuilderProblemNumericSpec> numericInitializations)
    {
        NumericInitializations = Array.AsReadOnly(numericInitializations.ToArray());
    }
}
