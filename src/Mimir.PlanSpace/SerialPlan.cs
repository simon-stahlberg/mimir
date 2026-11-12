using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace;

public sealed class SerialStep
{
    public int Id { get; }
    public ActionSchema Schema { get; }
    public IReadOnlyList<Constant> Binding { get; }
    internal Action Action { get; }
    // The fluent preconditions of the schema followed by its static ones, so that two steps of one schema can
    // be compared precondition by precondition whatever their bindings are.
    public IReadOnlyList<Literal<Fact>> Preconditions { get; }
    // The fluent literals the step makes true.
    public IReadOnlyList<Literal<Fact>> Effects { get; }

    internal SerialStep(int id, ActionSchema schema, IReadOnlyList<Constant> binding, Problem problem)
    {
        if (binding.Count != schema.Parameters.Count)
            throw new ArgumentException($"Action {schema.Name} expects {schema.Parameters.Count} arguments, got {binding.Count}.", nameof(binding));
        for (int i = 0; i < binding.Count; i++)
        {
            if (!problem.Domain.IsCompatible(binding[i].Type, schema.Parameters[i].Type))
                throw new ArgumentException($"Object {binding[i].Name} cannot be parameter {i} of action {schema.Name}.", nameof(binding));
        }

        Id = id;
        Schema = schema;
        Constant[] arguments = binding.ToArray();
        Binding = Array.AsReadOnly(arguments);
        Action = problem.GroundAction(schema, arguments);

        var preconditions = new List<Literal<Fact>>();
        foreach (Literal<Atom<Fluent>> literal in schema.FluentPreconditions)
        {
            Constant[] atomArguments = SchemaTerms.ResolveBound(schema, literal.Value.Arguments, arguments);
            preconditions.Add(new Literal<Fact>(problem.Context.RegisterFact(literal.Value.Predicate, atomArguments), literal.Polarity));
        }

        foreach (Literal<Atom<Static>> literal in schema.StaticPreconditions)
        {
            Constant[] atomArguments = SchemaTerms.ResolveBound(schema, literal.Value.Arguments, arguments);
            preconditions.Add(new Literal<Fact>(problem.Context.RegisterFact(literal.Value.Predicate, atomArguments), literal.Polarity));
        }

        Preconditions = preconditions;
        Effects = Action.Effect.Literals.ToArray();
    }

    internal SerialStep WithBinding(IReadOnlyList<Constant> binding, Problem problem) => new(Id, Schema, binding, problem);

    public override string ToString() => $"{Id}:({Schema.Name} {string.Join(" ", Binding.Select(constant => constant.Name))})";
}

// A plan is a sequence of ground actions that need not be executable. Between every two steps, before the
// first and after the last there is an insertion point; point i is the one before the step at index i.
public sealed partial class SerialPlan
{
    private readonly List<SerialStep> _steps = new();
    private readonly AchieverIndex _achievers;
    private int _nextStepId;
    private SerialDiagnosis? _diagnosis;

    public Problem Problem { get; }
    public IReadOnlyList<Literal<Fact>> Goal { get; }
    public IReadOnlyList<SerialStep> Steps => _steps;

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
        if (problem.Domain.Actions.Any(schema => schema.ConditionalEffects.Count > 0))
            throw new NotSupportedException("Conditional effects are not supported by the serial plan space.");

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
            if (literal.Value is Fact<Static> fact && StaticFacts.Holds(problem, fact) != literal.IsPositive)
                throw new InvalidOperationException($"Static goal literal {literal} does not hold, so the goal is unreachable.");
        }
    }

    // The objects a parameter of the schema can be bound to, in the order of the problem's objects.
    public IReadOnlyList<Constant> ParameterOptions(ActionSchema schema, int parameterIndex)
    {
        ArgumentNullException.ThrowIfNull(schema);
        string type = schema.Parameters[parameterIndex].Type;
        return Problem.AllObjects.Where(candidate => Problem.Domain.IsCompatible(candidate.Type, type)).ToArray();
    }

    public bool IsSolution() => Diagnose().IsSolution;

    public IReadOnlyList<Action> ToActions()
    {
        if (!IsSolution()) throw new InvalidOperationException($"The plan still has errors; the first is {Diagnose().Errors[0]}.");
        return _steps.Select(step => step.Action).ToArray();
    }

    public SerialPlan Clone() => Clone(Goal);

    private SerialPlan Clone(IReadOnlyList<Literal<Fact>> goal)
    {
        var clone = new SerialPlan(Problem, goal, _achievers) { _nextStepId = _nextStepId };
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
                if (error.Kind == SerialErrorKind.UnmetGoal) closures.Remove(error.Literal);
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
