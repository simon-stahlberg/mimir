using System.Runtime.CompilerServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace;

public sealed partial class PartialOrderPlan
{
    private static readonly ConditionalWeakTable<Domain, AchieverIndex> AchieverIndices = new();

    private readonly List<PlanStep> _steps = new();
    private readonly Dictionary<int, PlanStep> _stepById = new();
    private readonly List<CausalLink> _links = new();
    private readonly Dictionary<int, CausalLink> _linkById = new();
    private readonly HashSet<(int Consumer, Fact<Fluent> Fact)> _supported = new();
    private readonly List<OrderingConstraint> _orderings = new();
    private readonly List<Tombstone> _tombstones = new();
    private readonly OrderingGraph _ordering = new();
    private int _nextStepId = 2;
    private int _nextLinkId;
    private int _version;

    public Problem Problem { get; }
    public IReadOnlyList<Literal<Fact>> Goal { get; }
    public PlanSpaceOptions Options { get; }
    public int Round { get; private set; }
    public IReadOnlyList<PlanStep> Steps => _steps;
    public IReadOnlyList<CausalLink> Links => _links;
    public IReadOnlyList<Tombstone> Tombstones => _tombstones;
    public IReadOnlyList<(int Before, int After)> OrderingClosure => _ordering.Pairs();
    internal AchieverIndex Achievers { get; }
    internal int Version => _version;

    private PartialOrderPlan(Problem problem, IReadOnlyList<Literal<Fact>> goal, PlanSpaceOptions options)
    {
        Problem = problem;
        Goal = goal;
        Options = options;
        Achievers = AchieverIndices.GetValue(problem.Domain, domain => new AchieverIndex(domain));
    }

    public static PartialOrderPlan Empty(Problem problem, PlanSpaceOptions options)
    {
        ArgumentNullException.ThrowIfNull(problem);
        return Empty(problem, problem.Goal, options);
    }

    public static PartialOrderPlan Empty(Problem problem, IReadOnlyList<Literal<Fact>> goal, PlanSpaceOptions options)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(goal);
        ArgumentNullException.ThrowIfNull(options);
        ValidateSupported(problem);
        ValidateGoal(problem, goal);

        var plan = new PartialOrderPlan(problem, goal, options);
        plan.RegisterStep(PlanStep.CreateInitial(problem));
        plan.RegisterStep(PlanStep.CreateGoal(GoalFacts(problem, goal)));
        plan.AddOrdering(0, 1, -1);
        return plan;
    }

    public static PartialOrderPlan FromSequentialPlan(Problem problem, IReadOnlyList<Action> plan, PlanSpaceOptions options)
    {
        ArgumentNullException.ThrowIfNull(problem);
        return FromSequentialPlan(problem, plan, problem.Goal, options);
    }

    public static PartialOrderPlan FromSequentialPlan(Problem problem, IReadOnlyList<Action> plan, IReadOnlyList<Literal<Fact>> goal, PlanSpaceOptions options)
    {
        ArgumentNullException.ThrowIfNull(plan);
        PartialOrderPlan result = Empty(problem, goal, options);
        var position = new Dictionary<int, int> { [0] = -1, [1] = plan.Count };
        var sequence = new List<PlanStep>();
        foreach (Action action in plan)
        {
            if (!ReferenceEquals(action.Context, problem.Context))
                throw new ArgumentException($"Action {action} belongs to a different problem.", nameof(plan));

            PlanStep step = result.CreateStep(action.Schema, action.Arguments.ToArray());
            position[step.Id] = sequence.Count;
            sequence.Add(step);
        }

        sequence.Add(result.GetStep(1));
        for (int consumerIndex = 0; consumerIndex < sequence.Count; consumerIndex++)
        {
            PlanStep consumer = sequence[consumerIndex];
            foreach (Fact<Fluent> fact in consumer.Preconditions)
            {
                int producerId = 0;
                for (int producerIndex = consumerIndex - 1; producerIndex >= 0; producerIndex--)
                {
                    if (!sequence[producerIndex].AddEffects.Contains(fact)) continue;
                    producerId = sequence[producerIndex].Id;
                    break;
                }

                if (producerId == 0 && !result.GetStep(0).AddEffects.Contains(fact))
                    throw new InvalidOperationException($"Precondition {fact} of step {consumer} holds neither initially nor after an earlier action.");
                if (result.TryAddLink(producerId, consumer.Id, fact) is null)
                    throw new InvalidOperationException($"Linking {producerId} to {consumer.Id} for {fact} created a cycle.");
            }
        }

        foreach (CausalLink link in result._links.ToArray())
        {
            foreach (PlanStep threatener in result._steps)
            {
                if (!result.IsThreat(link, threatener)) continue;

                bool added;
                if (position[threatener.Id] < position[link.ProducerId])
                    added = result.AddOrdering(threatener.Id, link.ProducerId, link.Id);
                else if (position[threatener.Id] > position[link.ConsumerId])
                    added = result.AddOrdering(link.ConsumerId, threatener.Id, link.Id);
                else
                    throw new InvalidOperationException($"Step {threatener} deletes {link.Fact} between its producer {link.ProducerId} and consumer {link.ConsumerId}; the sequential plan is invalid.");

                if (!added) throw new InvalidOperationException($"Resolving the threat of {threatener} on link {link} created a cycle.");
            }
        }

        FlawSet flaws = result.ComputeFlaws();
        if (flaws.Flaws.Count > 0)
            throw new InvalidOperationException($"The deordered plan still has {flaws.Flaws.Count} flaws; the first is {flaws.Flaws[0]}.");
        return result;
    }

    private static void ValidateSupported(Problem problem)
    {
        if (problem.HasNumericPlanning)
            throw new NotSupportedException("Numeric preconditions, effects or goals are not supported by the plan space.");

        foreach (ActionSchema schema in problem.Domain.Actions)
        {
            if (schema.Effects.Any(effect => !effect.IsUnconditional))
                throw new NotSupportedException($"Action {schema.Name} has conditional or quantified effects, which the plan space does not support.");
            if (schema.DerivedPreconditions.Count > 0)
                throw new NotSupportedException($"Action {schema.Name} uses derived predicates in its precondition, which the plan space does not support.");
            if (schema.FluentPreconditions.Any(literal => literal.IsNegative))
                throw new NotSupportedException($"Action {schema.Name} has a negative fluent precondition, which the plan space does not support.");
        }
    }

    private static void ValidateGoal(Problem problem, IReadOnlyList<Literal<Fact>> goal)
    {
        foreach (Literal<Fact> literal in goal)
        {
            if (!ReferenceEquals(literal.Value.Context, problem.Context))
                throw new ArgumentException($"Goal literal {literal} belongs to a different problem.", nameof(goal));
            if (literal.IsNegative)
                throw new NotSupportedException($"Negative goal literal {literal} is not supported by the plan space.");
            if (literal.Value is Fact<Derived>)
                throw new NotSupportedException($"Derived goal literal {literal} is not supported by the plan space.");
        }
    }

    private static List<Fact<Fluent>> GoalFacts(Problem problem, IReadOnlyList<Literal<Fact>> goal)
    {
        var facts = new List<Fact<Fluent>>();
        foreach (Literal<Fact> literal in goal)
        {
            if (literal.Value is Fact<Fluent> fluent)
            {
                if (!facts.Contains(fluent)) facts.Add(fluent);
                continue;
            }

            var staticFact = (Fact<Static>)literal.Value;
            if (!StaticFilter.Holds(problem, staticFact.Predicate, staticFact.Arguments.ToArray()))
                throw new InvalidOperationException($"Static goal literal {literal} does not hold in the initial state, so the problem is unsolvable.");
        }

        return facts;
    }

    public bool IsBefore(int stepA, int stepB) => _ordering.IsBefore(stepA, stepB);

    public PlanStep GetStep(int id)
    {
        if (!_stepById.TryGetValue(id, out PlanStep? step)) throw new ArgumentException($"Step {id} is not part of the plan.", nameof(id));
        return step;
    }

    public CausalLink GetLink(int id)
    {
        if (!_linkById.TryGetValue(id, out CausalLink? link)) throw new ArgumentException($"Link {id} is not part of the plan.", nameof(id));
        return link;
    }

    public bool IsSolution() => !HasFlaws();

    // Mirrors ComputeFlaws without enumerating resolvers or unifying ghosts.
    private bool HasFlaws()
    {
        foreach (PlanStep step in _steps)
        {
            if (!step.IsFullyBound) return true;
            foreach (Fact<Fluent> fact in step.Preconditions)
            {
                if (!_supported.Contains((step.Id, fact))) return true;
            }
        }

        foreach (CausalLink link in _links)
        {
            foreach (PlanStep threatener in _steps)
            {
                if (IsThreat(link, threatener)) return true;
            }
        }

        return false;
    }

    private bool IsThreat(CausalLink link, PlanStep threatener) => IsThreat(link, threatener, _ordering);

    private static bool IsThreat(CausalLink link, PlanStep threatener, OrderingGraph ordering)
    {
        if (threatener.Kind != StepKind.Step || !threatener.IsFullyBound) return false;
        if (threatener.Id == link.ProducerId || threatener.Id == link.ConsumerId) return false;
        if (!threatener.DeleteEffects.Contains(link.Fact)) return false;
        return !ordering.IsBefore(threatener.Id, link.ProducerId) && !ordering.IsBefore(link.ConsumerId, threatener.Id);
    }

    public IReadOnlyList<Action> Linearize()
    {
        if (HasFlaws()) throw new InvalidOperationException("The plan still has flaws or partially bound steps.");

        var remaining = _steps.Where(step => step.Kind == StepKind.Step).ToList();
        var actions = new List<Action>(remaining.Count);
        while (remaining.Count > 0)
        {
            PlanStep next = remaining.First(step => remaining.All(other => other == step || !IsBefore(other.Id, step.Id)));
            remaining.Remove(next);
            actions.Add(Problem.GroundAction(next.Schema!, next.Binding.Select(constant => constant!).ToArray()));
        }

        return actions;
    }

    public PartialOrderPlan Clone() => Clone(Goal);

    private PartialOrderPlan Clone(IReadOnlyList<Literal<Fact>> goal)
    {
        var clone = new PartialOrderPlan(Problem, goal, Options)
        {
            Round = Round,
            _nextStepId = _nextStepId,
            _nextLinkId = _nextLinkId,
            _version = _version
        };
        foreach (PlanStep step in _steps) clone.RegisterStep(step.Clone());
        foreach (CausalLink link in _links)
        {
            var copy = new CausalLink(link.Id, link.ProducerId, link.ConsumerId, link.Fact);
            clone._links.Add(copy);
            clone._linkById[copy.Id] = copy;
        }

        clone._supported.UnionWith(_supported);
        clone._orderings.AddRange(_orderings);
        clone._ordering.CopyFrom(_ordering);
        foreach (Tombstone tombstone in _tombstones)
            clone._tombstones.Add(new Tombstone(tombstone.Schema, tombstone.Binding) { Age = tombstone.Age });
        return clone;
    }

    private void RegisterStep(PlanStep step)
    {
        _steps.Add(step);
        _stepById.Add(step.Id, step);
        _ordering.AddStep(step.Id);
    }

    private PlanStep CreateStep(ActionSchema schema, Constant?[] binding)
    {
        PlanStep step = PlanStep.Create(_nextStepId++, schema, binding, Problem);
        RegisterStep(step);
        AddOrdering(0, step.Id, -1);
        AddOrdering(step.Id, 1, -1);
        return step;
    }

    private bool AddOrdering(int before, int after, int linkId)
    {
        if (!_ordering.TryAdd(before, after)) return false;
        _orderings.Add(new OrderingConstraint(before, after, linkId));
        return true;
    }

    private CausalLink? TryAddLink(int producerId, int consumerId, Fact<Fluent> fact)
    {
        int linkId = _nextLinkId;
        if (!AddOrdering(producerId, consumerId, linkId)) return null;

        _nextLinkId++;
        var link = new CausalLink(linkId, producerId, consumerId, fact);
        _links.Add(link);
        _linkById.Add(linkId, link);
        if (!_supported.Add((consumerId, fact)))
            throw new InvalidOperationException($"Step {consumerId} already has a link for {fact}.");
        return link;
    }

    // The caller rebuilds the ordering closure once all deletions of a round are done.
    private void RemoveStep(int id)
    {
        PlanStep step = GetStep(id);
        foreach (CausalLink link in _links.Where(link => link.ProducerId == id || link.ConsumerId == id).ToArray())
            RemoveLink(link);

        _orderings.RemoveAll(constraint => constraint.Before == id || constraint.After == id);
        _steps.Remove(step);
        _stepById.Remove(id);

        if (!Options.TombstonesEnabled || !step.IsFullyBound) return;
        _tombstones.Add(new Tombstone(step.Schema!, step.Binding.Select(constant => constant!).ToArray()));
        while (_tombstones.Count > Options.TombstoneCapacity) _tombstones.RemoveAt(0);
    }

    // Also drops the threat-resolution orderings that only existed to protect this link; the caller rebuilds the closure.
    private void RemoveLink(CausalLink link)
    {
        _links.Remove(link);
        _linkById.Remove(link.Id);
        _supported.Remove((link.ConsumerId, link.Fact));
        _orderings.RemoveAll(constraint => constraint.LinkId == link.Id);
    }
}
