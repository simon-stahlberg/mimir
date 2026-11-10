using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed record PlanSpaceOptions(int TombstoneCapacity = 0, int TombstoneLifespan = 0)
{
    public bool TombstonesEnabled => TombstoneCapacity > 0 && TombstoneLifespan > 0;
}

public enum StepKind
{
    Initial,
    Goal,
    Step
}

public sealed class PlanStep
{
    private static readonly IReadOnlySet<Fact<Fluent>> NoFacts = new HashSet<Fact<Fluent>>();
    private readonly Constant?[] _binding;

    public int Id { get; }
    public StepKind Kind { get; }
    public ActionSchema? Schema { get; }
    public IReadOnlyList<Constant?> Binding { get; }
    public bool IsFullyBound { get; private set; }
    internal IReadOnlyList<Fact<Fluent>> Preconditions { get; private set; } = [];
    internal IReadOnlySet<Fact<Fluent>> AddEffects { get; private set; } = NoFacts;
    internal IReadOnlySet<Fact<Fluent>> DeleteEffects { get; private set; } = NoFacts;

    private PlanStep(int id, StepKind kind, ActionSchema? schema, Constant?[] binding)
    {
        Id = id;
        Kind = kind;
        Schema = schema;
        _binding = binding;
        Binding = Array.AsReadOnly(binding);
    }

    internal static PlanStep CreateInitial(Problem problem)
        => new(0, StepKind.Initial, null, [])
        {
            IsFullyBound = true,
            AddEffects = problem.InitialState.GetTrueFacts().ToHashSet()
        };

    internal static PlanStep CreateGoal(IReadOnlyList<Fact<Fluent>> goalFacts)
        => new(1, StepKind.Goal, null, []) { IsFullyBound = true, Preconditions = goalFacts };

    internal static PlanStep Create(int id, ActionSchema schema, Constant?[] binding, Problem problem)
    {
        if (binding.Length != schema.Parameters.Count)
            throw new ArgumentException($"Action {schema.Name} expects {schema.Parameters.Count} arguments, got {binding.Length}.", nameof(binding));

        var step = new PlanStep(id, StepKind.Step, schema, binding);
        if (binding.All(constant => constant is not null)) step.Ground(problem);
        return step;
    }

    internal void Bind(int parameterIndex, Constant value, Problem problem)
    {
        if (Kind != StepKind.Step) throw new InvalidOperationException($"Step {Id} of kind {Kind} has no parameters.");
        if (_binding[parameterIndex] is not null)
            throw new InvalidOperationException($"Parameter {parameterIndex} of step {Id} is already bound.");

        _binding[parameterIndex] = value;
        if (_binding.All(constant => constant is not null)) Ground(problem);
    }

    private void Ground(Problem problem)
    {
        if (!StaticFilter.Passes(problem, Schema!, _binding))
            throw new InvalidOperationException($"The static preconditions of {this} do not hold.");

        IsFullyBound = true;
        Preconditions = Schema!.FluentPreconditions.Select(literal => Resolve(problem, literal.Value)).Distinct().ToArray();
        var adds = new HashSet<Fact<Fluent>>();
        var deletes = new HashSet<Fact<Fluent>>();
        foreach (ConditionalEffect effect in Schema.Effects)
        {
            Fact<Fluent> fact = Resolve(problem, effect.Effect.Value);
            if (effect.Effect.IsPositive) adds.Add(fact);
            else deletes.Add(fact);
        }

        // STRIPS semantics: an add effect wins over a delete effect on the same fact.
        deletes.ExceptWith(adds);
        AddEffects = adds;
        DeleteEffects = deletes;
    }

    private Fact<Fluent> Resolve(Problem problem, Atom<Fluent> atom)
        => problem.Context.RegisterFact(atom.Predicate, SchemaTerms.ResolveBound(Schema!, atom.Arguments, _binding));

    internal PlanStep Clone()
        => new(Id, Kind, Schema, (Constant?[])_binding.Clone())
        {
            IsFullyBound = IsFullyBound,
            Preconditions = Preconditions,
            AddEffects = AddEffects,
            DeleteEffects = DeleteEffects
        };

    public override string ToString()
    {
        if (Kind != StepKind.Step) return $"{Id}:{Kind}";
        return $"{Id}:({Schema!.Name} {string.Join(" ", _binding.Select(constant => constant?.Name ?? "?"))})";
    }
}

public sealed class CausalLink
{
    public int Id { get; }
    public int ProducerId { get; }
    public int ConsumerId { get; }
    public Fact<Fluent> Fact { get; }

    internal CausalLink(int id, int producerId, int consumerId, Fact<Fluent> fact)
    {
        Id = id;
        ProducerId = producerId;
        ConsumerId = consumerId;
        Fact = fact;
    }

    public override string ToString() => $"{Id}:{ProducerId}->{ConsumerId} {Fact}";
}

public sealed class Tombstone
{
    public ActionSchema Schema { get; }
    public IReadOnlyList<Constant> Binding { get; }
    public int Age { get; internal set; }

    internal Tombstone(ActionSchema schema, IReadOnlyList<Constant> binding)
    {
        Schema = schema;
        Binding = binding;
    }
}

// LinkId is the causal link that caused the constraint (its own ordering or a threat resolution), -1 otherwise.
internal sealed record OrderingConstraint(int Before, int After, int LinkId);

internal sealed class OrderingGraph
{
    private readonly Dictionary<int, int> _slotById = new();
    private readonly List<int> _idBySlot = new();
    private bool[,] _before = new bool[16, 16];

    public void AddStep(int id)
    {
        if (_slotById.ContainsKey(id)) throw new InvalidOperationException($"Step {id} is already ordered.");
        int slot = _idBySlot.Count;
        if (slot >= _before.GetLength(0)) Grow();
        _slotById[id] = slot;
        _idBySlot.Add(id);
    }

    private void Grow()
    {
        int oldSize = _before.GetLength(0);
        var grown = new bool[oldSize * 2, oldSize * 2];
        for (int i = 0; i < oldSize; i++)
            for (int j = 0; j < oldSize; j++)
                grown[i, j] = _before[i, j];
        _before = grown;
    }

    private int Slot(int id)
    {
        if (!_slotById.TryGetValue(id, out int slot)) throw new ArgumentException($"Step {id} is not part of the plan.");
        return slot;
    }

    public bool IsBefore(int before, int after) => _before[Slot(before), Slot(after)];

    public bool TryAdd(int before, int after)
    {
        int beforeSlot = Slot(before);
        int afterSlot = Slot(after);
        if (beforeSlot == afterSlot || _before[afterSlot, beforeSlot]) return false;
        if (_before[beforeSlot, afterSlot]) return true;

        int count = _idBySlot.Count;
        for (int x = 0; x < count; x++)
        {
            if (x != beforeSlot && !_before[x, beforeSlot]) continue;
            for (int y = 0; y < count; y++)
            {
                if (y == afterSlot || _before[afterSlot, y]) _before[x, y] = true;
            }
        }

        return true;
    }

    public void CopyFrom(OrderingGraph other)
    {
        _slotById.Clear();
        _idBySlot.Clear();
        foreach (int id in other._idBySlot)
        {
            _slotById[id] = _idBySlot.Count;
            _idBySlot.Add(id);
        }

        _before = (bool[,])other._before.Clone();
    }

    public void Rebuild(IEnumerable<int> ids, IEnumerable<OrderingConstraint> constraints)
    {
        _slotById.Clear();
        _idBySlot.Clear();
        Array.Clear(_before);
        foreach (int id in ids) AddStep(id);
        foreach (OrderingConstraint constraint in constraints)
            _before[Slot(constraint.Before), Slot(constraint.After)] = true;

        int count = _idBySlot.Count;
        for (int k = 0; k < count; k++)
            for (int i = 0; i < count; i++)
            {
                if (!_before[i, k]) continue;
                for (int j = 0; j < count; j++)
                    if (_before[k, j]) _before[i, j] = true;
            }

        for (int i = 0; i < count; i++)
            if (_before[i, i]) throw new InvalidOperationException($"The ordering constraints contain a cycle through step {_idBySlot[i]}.");
    }

    public IReadOnlyList<(int Before, int After)> Pairs()
    {
        var pairs = new List<(int Before, int After)>();
        int count = _idBySlot.Count;
        for (int i = 0; i < count; i++)
            for (int j = 0; j < count; j++)
                if (_before[i, j]) pairs.Add((_idBySlot[i], _idBySlot[j]));
        return pairs;
    }
}
