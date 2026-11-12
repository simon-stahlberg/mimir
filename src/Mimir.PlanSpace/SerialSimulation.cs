using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public enum SerialErrorKind
{
    UnsatisfiedPrecondition,
    UnmetGoal
}

public sealed class SerialError
{
    public SerialErrorKind Kind { get; }
    // The step whose precondition fails, -1 for an unmet goal literal.
    public int StepId { get; }
    public Literal<Fact> Literal { get; }
    // The candidates that make the literal true; none for a static literal.
    public IReadOnlyList<int> CandidateIndices { get; }
    // The step that last gave the fact the value the literal rejects, -1 when it has that value from the start.
    public int CulpritStepId { get; }

    internal SerialError(SerialErrorKind kind, int stepId, Literal<Fact> literal, IReadOnlyList<int> candidateIndices, int culpritStepId)
    {
        Kind = kind;
        StepId = stepId;
        Literal = literal;
        CandidateIndices = candidateIndices;
        CulpritStepId = culpritStepId;
    }

    public override string ToString()
        => Kind == SerialErrorKind.UnmetGoal ? $"UnmetGoal({Literal})" : $"UnsatisfiedPrecondition(step {StepId}, {Literal})";
}

public sealed class SerialSupport
{
    // The step that made the literal true, -1 for the initial state.
    public int ProducerStepId { get; }
    // The step that needs the literal, -1 for the goal.
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
    // The achievers of the literals of the errors, each listed once.
    public IReadOnlyList<GhostStep> Candidates { get; }
    public IReadOnlyList<SerialSupport> Supports { get; }
    public IReadOnlyList<int> UnusedStepIds { get; }
    public IReadOnlyList<Fact<Fluent>> FinalTrueFacts { get; }
    public bool IsSolution => Errors.Count == 0;
    internal Problem Problem { get; }

    internal SerialDiagnosis(
        IReadOnlyList<SerialError> errors,
        IReadOnlyList<GhostStep> candidates,
        IReadOnlyList<SerialSupport> supports,
        IReadOnlyList<int> unusedStepIds,
        IReadOnlyList<Fact<Fluent>> finalTrueFacts,
        Problem problem)
    {
        Problem = problem;
        Errors = errors;
        Candidates = candidates;
        Supports = supports;
        UnusedStepIds = unusedStepIds;
        FinalTrueFacts = finalTrueFacts;
    }
}

// The state of the simulation: the last value written to each fact and the step that wrote it.
internal sealed class SerialState
{
    private readonly Problem _problem;
    private readonly Dictionary<Fact<Fluent>, (bool Value, int WriterStepId)> _written = new();

    public SerialState(Problem problem) => _problem = problem;

    // WriterStepId is -1 for a value the fact has from the initial state.
    public (bool Value, int WriterStepId) Lookup(Fact<Fluent> fact)
        => _written.TryGetValue(fact, out (bool Value, int WriterStepId) entry) ? entry : (_problem.InitialState.IsTrue(fact), -1);

    public bool Holds(Literal<Fact> literal) => literal.Value switch
    {
        Fact<Fluent> fluent => Lookup(fluent).Value == literal.IsPositive,
        Fact<Static> fact => StaticFacts.Holds(_problem, fact) == literal.IsPositive,
        _ => throw new NotSupportedException($"Unsupported literal {literal}.")
    };

    // The step that wrote the current value of the fact of a fluent literal, -1 otherwise.
    public int Writer(Literal<Fact> literal) => literal.Value is Fact<Fluent> fluent ? Lookup(fluent).WriterStepId : -1;

    // Every effect is applied whether or not the preconditions of the step hold. Deletes are applied before
    // adds so that an add wins over a delete of the same fact, as in ExtendedState.Apply.
    public void Apply(SerialStep step)
    {
        foreach (bool isPositive in new[] { false, true })
        {
            foreach (Literal<Fact> effect in step.Effects)
            {
                if (effect.IsPositive == isPositive) _written[(Fact<Fluent>)effect.Value] = (isPositive, step.Id);
            }
        }
    }

    public List<Fact<Fluent>> TrueFacts()
    {
        var candidates = new HashSet<Fact<Fluent>>(_problem.InitialState.GetTrueFacts());
        candidates.UnionWith(_written.Keys);
        return candidates.Where(fact => Lookup(fact).Value).OrderBy(fact => fact.LocalIndex).ToList();
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
            foreach (Literal<Fact> literal in step.Preconditions.Distinct())
            {
                if (state.Holds(literal))
                {
                    if (literal.Value is Fact<Fluent>) supports.Add(new SerialSupport(state.Writer(literal), step.Id, literal));
                    continue;
                }

                errors.Add(new SerialError(SerialErrorKind.UnsatisfiedPrecondition, step.Id, literal, CandidatesFor(literal, candidates), state.Writer(literal)));
            }

            state.Apply(step);
        }

        foreach (Literal<Fact> literal in Goal.Distinct())
        {
            if (literal.Value is not Fact<Fluent>) continue;

            if (state.Holds(literal))
            {
                supports.Add(new SerialSupport(state.Writer(literal), -1, literal));
                continue;
            }

            errors.Add(new SerialError(SerialErrorKind.UnmetGoal, -1, literal, CandidatesFor(literal, candidates), state.Writer(literal)));
        }

        HashSet<int> producers = supports.Select(support => support.ProducerStepId).ToHashSet();
        int[] unused = _steps.Where(step => !producers.Contains(step.Id)).Select(step => step.Id).ToArray();
        _diagnosis = new SerialDiagnosis(errors, candidates, supports, unused, state.TrueFacts(), Problem);
        return _diagnosis;
    }

    private int[] CandidatesFor(Literal<Fact> literal, List<GhostStep> candidates)
    {
        if (literal.Value is not Fact<Fluent> fact) return [];

        var indices = new List<int>();
        foreach (GhostStep candidate in literal.IsPositive ? _achievers.Achievers(fact) : _achievers.Deleters(fact))
        {
            int index = candidates.IndexOf(candidate);
            if (index < 0)
            {
                index = candidates.Count;
                candidates.Add(candidate);
            }

            indices.Add(index);
        }

        return indices.ToArray();
    }
}
