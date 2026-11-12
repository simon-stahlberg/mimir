using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public enum SerialErrorKind
{
    UnsatisfiedPrecondition,
    UnmetGoal,
    UnboundParameter
}

public enum SerialRepairKind
{
    NoOp,
    Insert,
    Bind,
    Rebind
}

public sealed class SerialRepair
{
    public static readonly SerialRepair NoOp = new(SerialRepairKind.NoOp, -1, -1, null);

    public SerialRepairKind Kind { get; }
    public int CandidateIndex { get; }
    public int ParameterIndex { get; }
    public Constant? Object { get; }

    private SerialRepair(SerialRepairKind kind, int candidateIndex, int parameterIndex, Constant? value)
    {
        Kind = kind;
        CandidateIndex = candidateIndex;
        ParameterIndex = parameterIndex;
        Object = value;
    }

    internal static SerialRepair Insert(int candidateIndex) => new(SerialRepairKind.Insert, candidateIndex, -1, null);
    internal static SerialRepair Bind(int parameterIndex, Constant value) => new(SerialRepairKind.Bind, -1, parameterIndex, value);
    internal static SerialRepair Rebind(int parameterIndex, Constant value) => new(SerialRepairKind.Rebind, -1, parameterIndex, value);

    public override string ToString() => Kind switch
    {
        SerialRepairKind.Insert => $"Insert({CandidateIndex})",
        SerialRepairKind.Bind => $"Bind({ParameterIndex}, {Object})",
        SerialRepairKind.Rebind => $"Rebind({ParameterIndex}, {Object})",
        _ => Kind.ToString()
    };
}

public sealed class SerialError
{
    public SerialErrorKind Kind { get; }
    public int StepId { get; }
    public Literal<Fact>? Literal { get; }
    public int ParameterIndex { get; }
    public IReadOnlyList<SerialRepair> Repairs { get; }

    internal SerialError(SerialErrorKind kind, int stepId, Literal<Fact>? literal, int parameterIndex, IReadOnlyList<SerialRepair> repairs)
    {
        if (repairs.Count == 0 || repairs[0].Kind != SerialRepairKind.NoOp)
            throw new ArgumentException("The first repair of an error must be NoOp.", nameof(repairs));

        Kind = kind;
        StepId = stepId;
        Literal = literal;
        ParameterIndex = parameterIndex;
        Repairs = repairs;
    }

    public override string ToString() => Kind switch
    {
        SerialErrorKind.UnsatisfiedPrecondition => $"UnsatisfiedPrecondition(step {StepId}, {Literal})",
        SerialErrorKind.UnmetGoal => $"UnmetGoal({Literal})",
        _ => $"UnboundParameter(step {StepId}, parameter {ParameterIndex})"
    };
}

public sealed class SerialSupport
{
    public int ProducerStepId { get; }
    public int ConsumerStepId { get; }
    public Literal<Fact> Literal { get; }

    internal SerialSupport(int producerStepId, int consumerStepId, Literal<Fact> literal)
    {
        ProducerStepId = producerStepId;
        ConsumerStepId = consumerStepId;
        Literal = literal;
    }

    public override string ToString() => $"{ProducerStepId}->{ConsumerStepId} {Literal}";
}

public sealed class SerialDiagnosis
{
    public IReadOnlyList<SerialError> Errors { get; }
    public IReadOnlyList<GhostStep> Candidates { get; }
    public IReadOnlyList<SerialSupport> Supports { get; }
    public IReadOnlyList<int> UnusedStepIds { get; }
    public IReadOnlyList<Fact<Fluent>> FinalTrueFacts { get; }
    public IReadOnlyList<Fact<Fluent>> FinalUnknownFacts { get; }
    public bool IsSolution => Errors.Count == 0;
    internal SerialPlan Plan { get; }
    internal int Version { get; }

    internal SerialDiagnosis(
        IReadOnlyList<SerialError> errors,
        IReadOnlyList<GhostStep> candidates,
        IReadOnlyList<SerialSupport> supports,
        IReadOnlyList<int> unusedStepIds,
        IReadOnlyList<Fact<Fluent>> finalTrueFacts,
        IReadOnlyList<Fact<Fluent>> finalUnknownFacts,
        SerialPlan plan,
        int version)
    {
        Errors = errors;
        Candidates = candidates;
        Supports = supports;
        UnusedStepIds = unusedStepIds;
        FinalTrueFacts = finalTrueFacts;
        FinalUnknownFacts = finalUnknownFacts;
        Plan = plan;
        Version = version;
    }
}

internal enum Truth
{
    False,
    True,
    Unknown
}

// The state of the simulation: the last value written to each fact, and the open effects that have made
// the facts they match unknown since.
internal sealed class SerialState
{
    private readonly Problem _problem;
    private readonly Dictionary<Fact<Fluent>, (Truth Value, int Time, int ProducerStepId)> _written = new();
    private readonly List<(SerialOpenEffect Effect, int Time)> _opened = new();
    private int _clock;

    public SerialState(Problem problem) => _problem = problem;

    // ProducerStepId is the step that wrote the value, -1 for the initial state.
    public (Truth Value, int ProducerStepId) Lookup(Fact<Fluent> fact)
    {
        if (!_written.TryGetValue(fact, out (Truth Value, int Time, int ProducerStepId) entry))
            entry = (_problem.InitialState.IsTrue(fact) ? Truth.True : Truth.False, 0, -1);

        foreach ((SerialOpenEffect effect, int time) in _opened)
        {
            if (time > entry.Time && effect.Matches(fact, _problem.Domain)) return (Truth.Unknown, -1);
        }

        return (entry.Value, entry.ProducerStepId);
    }

    public Truth Holds(Fact<Fluent> fact, bool isPositive)
    {
        Truth value = Lookup(fact).Value;
        if (value == Truth.Unknown) return Truth.Unknown;
        return (value == Truth.True) == isPositive ? Truth.True : Truth.False;
    }

    private Truth Holds(IReadOnlyList<Literal<Fact>> condition)
    {
        Truth result = Truth.True;
        foreach (Literal<Fact> literal in condition)
        {
            Truth value = literal.Value switch
            {
                Fact<Fluent> fluent => Holds(fluent, literal.IsPositive),
                Fact<Static> fact => StaticFilter.Holds(_problem, fact.Predicate, fact.Arguments.ToArray()) == literal.IsPositive ? Truth.True : Truth.False,
                _ => throw new NotSupportedException($"Unsupported condition literal {literal}.")
            };
            if (value == Truth.False) return Truth.False;
            if (value == Truth.Unknown) result = Truth.Unknown;
        }

        return result;
    }

    // Every effect is applied whether or not the preconditions of the step hold.
    public void Apply(SerialStep step)
    {
        // Conditions are judged in the state before the step, and deletes are applied before adds so that an
        // add wins over a delete of the same fact, both as in ExtendedState.Apply.
        Truth[] fires = step.Effects.Select(effect => Holds(effect.Condition)).ToArray();
        foreach (bool isPositive in new[] { false, true })
        {
            foreach (SerialOpenEffect effect in step.OpenEffects)
            {
                if (effect.IsPositive == isPositive) _opened.Add((effect, ++_clock));
            }

            for (int i = 0; i < fires.Length; i++)
            {
                SerialEffect effect = step.Effects[i];
                if (effect.IsPositive != isPositive || fires[i] == Truth.False) continue;

                Truth value = isPositive ? Truth.True : Truth.False;
                _written[effect.Fact] = (fires[i] == Truth.Unknown ? Truth.Unknown : value, ++_clock, step.Id);
            }
        }
    }

    public List<Fact<Fluent>> FactsWithValue(Truth value)
    {
        var candidates = new HashSet<Fact<Fluent>>(_problem.InitialState.GetTrueFacts());
        candidates.UnionWith(_written.Keys);
        if (value == Truth.Unknown)
        {
            foreach ((SerialOpenEffect effect, _) in _opened) AddInstances(effect, 0, effect.Arguments.ToArray(), candidates);
        }

        return candidates.Where(fact => Lookup(fact).Value == value).OrderBy(fact => fact.LocalIndex).ToList();
    }

    private void AddInstances(SerialOpenEffect effect, int position, Constant?[] arguments, HashSet<Fact<Fluent>> instances)
    {
        if (position == arguments.Length)
        {
            instances.Add(_problem.Context.RegisterFact(effect.Predicate, arguments.Select(argument => argument!).ToArray()));
            return;
        }

        if (effect.Arguments[position] is not null)
        {
            AddInstances(effect, position + 1, arguments, instances);
            return;
        }

        foreach (Constant candidate in _problem.AllObjects)
        {
            if (!_problem.Domain.IsCompatible(candidate.Type, effect.Types[position])) continue;
            arguments[position] = candidate;
            AddInstances(effect, position + 1, arguments, instances);
        }
    }
}

public sealed partial class SerialPlan
{
    public SerialDiagnosis Diagnose()
    {
        if (_diagnosis is not null) return _diagnosis;

        var state = new SerialState(Problem);
        var errors = new List<SerialError>();
        var candidates = new List<GhostStep>();
        var supports = new List<SerialSupport>();
        foreach (SerialStep step in _steps)
        {
            AddUnboundParameterErrors(step, errors);
            foreach (SerialPrecondition precondition in step.Preconditions)
            {
                var literal = new Literal<Fact>(precondition.Fact, precondition.IsPositive ? Polarity.Positive : Polarity.Negative);
                Truth holds = state.Holds(precondition.Fact, precondition.IsPositive);
                if (holds == Truth.True) supports.Add(new SerialSupport(state.Lookup(precondition.Fact).ProducerStepId, step.Id, literal));
                if (holds != Truth.False) continue;

                List<SerialRepair> repairs = InsertRepairs(literal, candidates);
                AddRebindRepairs(step, precondition, state, repairs);
                errors.Add(new SerialError(SerialErrorKind.UnsatisfiedPrecondition, step.Id, literal, -1, repairs));
            }

            state.Apply(step);
        }

        foreach (Literal<Fact> literal in Goal.Distinct())
        {
            if (literal.Value is not Fact<Fluent> fact) continue;

            Truth holds = state.Holds(fact, literal.IsPositive);
            if (holds == Truth.True) supports.Add(new SerialSupport(state.Lookup(fact).ProducerStepId, -1, literal));
            if (holds == Truth.False) errors.Add(new SerialError(SerialErrorKind.UnmetGoal, -1, literal, -1, InsertRepairs(literal, candidates)));
        }

        HashSet<int> producers = supports.Select(support => support.ProducerStepId).ToHashSet();
        int[] unused = _steps.Where(step => step.IsFullyBound && !producers.Contains(step.Id)).Select(step => step.Id).ToArray();
        _diagnosis = new SerialDiagnosis(errors, candidates, supports, unused,
            state.FactsWithValue(Truth.True), state.FactsWithValue(Truth.Unknown), this, _version);
        return _diagnosis;
    }

    private void AddUnboundParameterErrors(SerialStep step, List<SerialError> errors)
    {
        for (int index = 0; index < step.Binding.Count; index++)
        {
            if (step.Binding[index] is not null) continue;

            var repairs = new List<SerialRepair> { SerialRepair.NoOp };
            Constant?[] trial = step.Binding.ToArray();
            foreach (Constant candidate in Problem.AllObjects)
            {
                if (!Problem.Domain.IsCompatible(candidate.Type, step.Schema.Parameters[index].Type)) continue;
                trial[index] = candidate;
                if (!StaticFilter.Passes(Problem, step.Schema, trial)) continue;
                repairs.Add(SerialRepair.Bind(index, candidate));
            }

            errors.Add(new SerialError(SerialErrorKind.UnboundParameter, step.Id, null, index, repairs));
        }
    }

    private List<SerialRepair> InsertRepairs(Literal<Fact> literal, List<GhostStep> candidates)
    {
        var repairs = new List<SerialRepair> { SerialRepair.NoOp };
        var fact = (Fact<Fluent>)literal.Value;
        foreach (GhostStep candidate in literal.IsPositive ? _achievers.Achievers(fact) : _achievers.Deleters(fact))
        {
            if (!StaticFilter.Passes(Problem, candidate.Schema, candidate.Binding)) continue;

            int index = candidates.IndexOf(candidate);
            if (index < 0)
            {
                index = candidates.Count;
                candidates.Add(candidate);
            }

            repairs.Add(SerialRepair.Insert(index));
        }

        return repairs;
    }

    // Changing one parameter of the failing atom repairs the error when the atom then names a fact that
    // already has the required value before the step.
    private void AddRebindRepairs(SerialStep step, SerialPrecondition precondition, SerialState state, List<SerialRepair> repairs)
    {
        IEnumerable<int> parameterIndices = precondition.Atom.Arguments.OfType<Variable>()
            .Select(variable => SchemaTerms.ParameterIndex(step.Schema, variable))
            .Distinct();
        foreach (int index in parameterIndices)
        {
            Constant?[] trial = step.Binding.ToArray();
            foreach (Constant candidate in Problem.AllObjects)
            {
                if (ReferenceEquals(candidate, step.Binding[index])) continue;
                if (!Problem.Domain.IsCompatible(candidate.Type, step.Schema.Parameters[index].Type)) continue;
                trial[index] = candidate;
                if (!StaticFilter.Passes(Problem, step.Schema, trial)) continue;

                Constant[] arguments = SchemaTerms.ResolveBound(step.Schema, precondition.Atom.Arguments, trial);
                Fact<Fluent> fact = Problem.Context.RegisterFact(precondition.Atom.Predicate, arguments);
                if (state.Holds(fact, precondition.IsPositive) == Truth.True) repairs.Add(SerialRepair.Rebind(index, candidate));
            }
        }
    }
}
