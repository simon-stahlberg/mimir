using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace;

internal sealed record SerialPrecondition(Atom<Fluent> Atom, Fact<Fluent> Fact, bool IsPositive);

// Condition is empty for an unconditional effect.
internal sealed record SerialEffect(Fact<Fluent> Fact, bool IsPositive, IReadOnlyList<Literal<Fact>> Condition);

// An effect whose fact is not determined yet: a null argument is open and ranges over the objects of its type.
internal sealed record SerialOpenEffect(Mimir.Core.Schemas.Predicate<Fluent> Predicate, bool IsPositive, Constant?[] Arguments, string[] Types)
{
    public bool Matches(Fact<Fluent> fact, Domain domain)
    {
        if (!ReferenceEquals(fact.Predicate, Predicate)) return false;
        for (int i = 0; i < Arguments.Length; i++)
        {
            if (Arguments[i] is null && !domain.IsCompatible(fact.Arguments[i].Type, Types[i])) return false;
            if (Arguments[i] is not null && !ReferenceEquals(Arguments[i], fact.Arguments[i])) return false;
        }

        return true;
    }
}

public sealed class SerialStep
{
    private readonly Constant?[] _binding;

    public int Id { get; }
    public ActionSchema Schema { get; }
    public IReadOnlyList<Constant?> Binding { get; }
    // The step whose error this step was inserted to repair, -1 when it was inserted for a goal literal.
    public int PurposeStepId { get; }
    public Literal<Fact> Purpose { get; }
    public bool IsFullyBound => Action is not null;
    internal Action? Action { get; }
    internal IReadOnlyList<SerialPrecondition> Preconditions { get; }
    internal IReadOnlyList<SerialEffect> Effects { get; }
    internal IReadOnlyList<SerialOpenEffect> OpenEffects { get; }

    internal SerialStep(int id, ActionSchema schema, Constant?[] binding, int purposeStepId, Literal<Fact> purpose, Problem problem)
    {
        if (binding.Length != schema.Parameters.Count)
            throw new ArgumentException($"Action {schema.Name} expects {schema.Parameters.Count} arguments, got {binding.Length}.", nameof(binding));

        Id = id;
        Schema = schema;
        _binding = binding;
        Binding = Array.AsReadOnly(binding);
        PurposeStepId = purposeStepId;
        Purpose = purpose;
        if (!StaticFilter.Passes(problem, schema, binding))
            throw new InvalidOperationException($"The static preconditions of {this} do not hold.");

        Preconditions = BoundPreconditions(problem);
        var effects = new List<SerialEffect>();
        var openEffects = new List<SerialOpenEffect>();
        if (binding.All(constant => constant is not null))
        {
            Action = problem.GroundAction(schema, binding.Select(constant => constant!).ToArray());
            foreach (Literal<Fact> literal in Action.Effect.Literals)
                effects.Add(new SerialEffect((Fact<Fluent>)literal.Value, literal.IsPositive, []));
            foreach (GroundConditionalEffect effect in Action.ConditionalEffects)
                effects.Add(new SerialEffect(effect.Effect.Value, effect.Effect.IsPositive, effect.Condition.Literals));
        }
        else
        {
            AddSchemaEffects(problem, effects, openEffects);
        }

        Effects = effects;
        OpenEffects = openEffects;
    }

    // Precondition atoms with an unbound argument cannot be judged yet and are left out.
    private List<SerialPrecondition> BoundPreconditions(Problem problem)
    {
        var preconditions = new List<SerialPrecondition>();
        foreach (Literal<Atom<Fluent>> literal in Schema.FluentPreconditions)
        {
            Constant?[] arguments = SchemaTerms.Resolve(Schema, literal.Value.Arguments, _binding);
            if (arguments.Any(argument => argument is null)) continue;

            Fact<Fluent> fact = problem.Context.RegisterFact(literal.Value.Predicate, arguments.Select(argument => argument!).ToArray());
            if (preconditions.Any(existing => existing.Fact.Equals(fact) && existing.IsPositive == literal.IsPositive)) continue;
            preconditions.Add(new SerialPrecondition(literal.Value, fact, literal.IsPositive));
        }

        return preconditions;
    }

    // The effects of a partially bound step: a conditional effect may or may not fire, so it leaves every fact of
    // its predicate open, and an unconditional one leaves the facts open that its unbound arguments can still name.
    private void AddSchemaEffects(Problem problem, List<SerialEffect> effects, List<SerialOpenEffect> openEffects)
    {
        foreach (ConditionalEffect effect in Schema.Effects)
        {
            Atom<Fluent> atom = effect.Effect.Value;
            Mimir.Core.Schemas.Predicate<Fluent> predicate = atom.Predicate;
            if (!effect.IsUnconditional)
            {
                string[] parameterTypes = predicate.Parameters.Select(parameter => parameter.Type).ToArray();
                openEffects.Add(new SerialOpenEffect(predicate, effect.Effect.IsPositive, new Constant?[parameterTypes.Length], parameterTypes));
                continue;
            }

            Constant?[] arguments = SchemaTerms.Resolve(Schema, atom.Arguments, _binding);
            if (arguments.All(argument => argument is not null))
            {
                Fact<Fluent> fact = problem.Context.RegisterFact(predicate, arguments.Select(argument => argument!).ToArray());
                effects.Add(new SerialEffect(fact, effect.Effect.IsPositive, []));
                continue;
            }

            string[] types = atom.Arguments.Select((term, position) => term is Variable variable ? variable.Type : predicate.Parameters[position].Type).ToArray();
            openEffects.Add(new SerialOpenEffect(predicate, effect.Effect.IsPositive, arguments, types));
        }
    }

    internal SerialStep WithBinding(int parameterIndex, Constant value, Problem problem)
    {
        var binding = (Constant?[])_binding.Clone();
        binding[parameterIndex] = value;
        return new SerialStep(Id, Schema, binding, PurposeStepId, Purpose, problem);
    }

    public override string ToString()
        => $"{Id}:({Schema.Name} {string.Join(" ", _binding.Select(constant => constant?.Name ?? "?"))})";
}

public sealed partial class SerialPlan
{
    private readonly List<SerialStep> _steps = new();
    private readonly AchieverIndex _achievers;
    private int _nextStepId;
    private int _version;
    private SerialDiagnosis? _diagnosis;

    public Problem Problem { get; }
    public IReadOnlyList<Literal<Fact>> Goal { get; }
    public IReadOnlyList<SerialStep> Steps => _steps;
    public int Round { get; private set; }
    internal int Version => _version;

    private SerialPlan(Problem problem, IReadOnlyList<Literal<Fact>> goal, AchieverIndex achievers)
    {
        Problem = problem;
        Goal = goal;
        _achievers = achievers;
    }

    public static SerialPlan Empty(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        return Empty(problem, problem.Goal);
    }

    public static SerialPlan Empty(Problem problem, IReadOnlyList<Literal<Fact>> goal)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(goal);
        if (problem.HasNumericPlanning)
            throw new NotSupportedException("Numeric preconditions, effects or goals are not supported by the serial plan space.");
        if (problem.Domain.Derived.Count > 0)
            throw new NotSupportedException("Derived predicates are not supported by the serial plan space.");

        ValidateGoal(problem, goal);
        return new SerialPlan(problem, goal, new AchieverIndex(problem.Domain));
    }

    // Static goal literals never become errors, so one that does not hold makes the goal unreachable.
    private static void ValidateGoal(Problem problem, IReadOnlyList<Literal<Fact>> goal)
    {
        foreach (Literal<Fact> literal in goal)
        {
            if (!ReferenceEquals(literal.Value.Context, problem.Context))
                throw new ArgumentException($"Goal literal {literal} belongs to a different problem.", nameof(goal));
            if (literal.Value is not Fact<Static> fact) continue;
            if (StaticFilter.Holds(problem, fact.Predicate, fact.Arguments.ToArray()) != literal.IsPositive)
                throw new InvalidOperationException($"Static goal literal {literal} does not hold, so the goal is unreachable.");
        }
    }

    public bool IsSolution() => Diagnose().IsSolution;

    public IReadOnlyList<Action> ToActions()
    {
        if (!IsSolution()) throw new InvalidOperationException($"The plan still has errors; the first is {Diagnose().Errors[0]}.");
        return _steps.Select(step => step.Action!).ToArray();
    }

    public SerialPlan Clone() => Clone(Goal);

    private SerialPlan Clone(IReadOnlyList<Literal<Fact>> goal)
    {
        var clone = new SerialPlan(Problem, goal, _achievers) { Round = Round, _nextStepId = _nextStepId, _version = _version };
        clone._steps.AddRange(_steps);
        return clone;
    }

    public SerialPlan WithGoal(IReadOnlyList<Literal<Fact>> goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        ValidateGoal(Problem, goal);
        return Clone(goal);
    }

    // The steps a satisfied goal literal depends on are the producers of its support and, recursively, of their
    // supports. A kept step can still fail once the steps outside the closures are gone, so the sub-plan is
    // simulated again and the goal literals that depend on a failing step are given up.
    public SerialPlan PruneToAchievedGoal()
    {
        SerialDiagnosis diagnosis = Diagnose();
        var closures = new Dictionary<Literal<Fact>, HashSet<int>>();
        foreach (SerialSupport support in diagnosis.Supports)
        {
            if (support.ConsumerStepId == -1) closures.Add(support.Literal, BackwardClosure(diagnosis, support.ProducerStepId));
        }

        while (true)
        {
            Literal<Fact>[] goal = Goal.Where(literal => literal.Value is Fact<Static> || closures.ContainsKey(literal)).ToArray();
            SerialPlan result = Clone(goal);
            result._steps.RemoveAll(step => !closures.Values.Any(closure => closure.Contains(step.Id)));
            SerialDiagnosis pruned = result.Diagnose();
            if (pruned.IsSolution) return result;

            int closureCount = closures.Count;
            foreach (SerialError error in pruned.Errors)
            {
                if (error.Kind == SerialErrorKind.UnmetGoal) closures.Remove(error.Literal!);
                foreach (Literal<Fact> literal in closures.Keys.Where(literal => closures[literal].Contains(error.StepId)).ToArray())
                    closures.Remove(literal);
            }

            if (closures.Count == closureCount)
                throw new InvalidOperationException($"The pruned plan still has errors that no goal literal accounts for; the first is {pruned.Errors[0]}.");
        }
    }

    private static HashSet<int> BackwardClosure(SerialDiagnosis diagnosis, int producerStepId)
    {
        var steps = new HashSet<int>();
        var frontier = new Stack<int>();
        frontier.Push(producerStepId);
        while (frontier.Count > 0)
        {
            int id = frontier.Pop();
            if (id == -1 || !steps.Add(id)) continue;
            foreach (SerialSupport support in diagnosis.Supports)
            {
                if (support.ConsumerStepId == id) frontier.Push(support.ProducerStepId);
            }
        }

        return steps;
    }
}
