using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Predicate = Mimir.Core.Schemas.Predicate;
using System.Runtime.CompilerServices;


namespace Mimir.Core.Grounding;

public class InstanceContext
{
    private readonly struct FactKey : IEquatable<FactKey>
    {
        private readonly Predicate _predicate;
        private readonly int _arity;
        private readonly Constant? _arg1;
        private readonly Constant? _arg2;
        private readonly Constant? _arg3;
        private readonly Constant? _arg4;
        private readonly Constant[]? _extraArgs;

        public FactKey(Predicate predicate)
        {
            _predicate = predicate;
            _arity = 0;
            _arg1 = null;
            _arg2 = null;
            _arg3 = null;
            _arg4 = null;
            _extraArgs = null;
        }

        public FactKey(Predicate predicate, Constant arg1)
        {
            _predicate = predicate;
            _arity = 1;
            _arg1 = arg1;
            _arg2 = null;
            _arg3 = null;
            _arg4 = null;
            _extraArgs = null;
        }

        public FactKey(Predicate predicate, Constant arg1, Constant arg2)
        {
            _predicate = predicate;
            _arity = 2;
            _arg1 = arg1;
            _arg2 = arg2;
            _arg3 = null;
            _arg4 = null;
            _extraArgs = null;
        }

        public FactKey(Predicate predicate, Constant arg1, Constant arg2, Constant arg3)
        {
            _predicate = predicate;
            _arity = 3;
            _arg1 = arg1;
            _arg2 = arg2;
            _arg3 = arg3;
            _arg4 = null;
            _extraArgs = null;
        }

        public FactKey(
            Predicate predicate,
            Constant arg1,
            Constant arg2,
            Constant arg3,
            Constant arg4)
        {
            _predicate = predicate;
            _arity = 4;
            _arg1 = arg1;
            _arg2 = arg2;
            _arg3 = arg3;
            _arg4 = arg4;
            _extraArgs = null;
        }

        public FactKey(Predicate predicate, IReadOnlyList<Constant> arguments)
        {
            _predicate = predicate;
            _arity = arguments.Count;
            _arg1 = arguments.Count > 0 ? arguments[0] : null;
            _arg2 = arguments.Count > 1 ? arguments[1] : null;
            _arg3 = arguments.Count > 2 ? arguments[2] : null;
            _arg4 = arguments.Count > 3 ? arguments[3] : null;

            if (arguments.Count > 4)
            {
                _extraArgs = new Constant[arguments.Count - 4];
                for (int i = 4; i < arguments.Count; i++)
                    _extraArgs[i - 4] = arguments[i];
            }
            else
            {
                _extraArgs = null;
            }
        }

        public bool Equals(FactKey other)
        {
            if (!ReferenceEquals(_predicate, other._predicate)) return false;
            if (_arity != other._arity) return false;
            if (!ReferenceEquals(_arg1, other._arg1)) return false;
            if (!ReferenceEquals(_arg2, other._arg2)) return false;
            if (!ReferenceEquals(_arg3, other._arg3)) return false;
            if (!ReferenceEquals(_arg4, other._arg4)) return false;

            if (_extraArgs == null) return other._extraArgs == null;
            if (other._extraArgs == null || _extraArgs.Length != other._extraArgs.Length) return false;

            for (int i = 0; i < _extraArgs.Length; i++)
                if (!ReferenceEquals(_extraArgs[i], other._extraArgs[i]))
                    return false;

            return true;
        }

        public override bool Equals(object? obj) => obj is FactKey other && Equals(other);

        public override int GetHashCode()
        {
            var hash = new HashCode();
            hash.Add(RuntimeHelpers.GetHashCode(_predicate));
            hash.Add(_arity);
            hash.Add(_arg1 == null ? 0 : RuntimeHelpers.GetHashCode(_arg1));
            hash.Add(_arg2 == null ? 0 : RuntimeHelpers.GetHashCode(_arg2));
            hash.Add(_arg3 == null ? 0 : RuntimeHelpers.GetHashCode(_arg3));
            hash.Add(_arg4 == null ? 0 : RuntimeHelpers.GetHashCode(_arg4));
            if (_extraArgs != null)
            {
                foreach (Constant arg in _extraArgs) hash.Add(RuntimeHelpers.GetHashCode(arg));
            }

            return hash.ToHashCode();
        }
    }

    private readonly List<Fact> _allFacts = new();
    private readonly Dictionary<FactKey, FactIndex> _factToIndex = new();
    private readonly Dictionary<Predicate, List<Fact>> _factsByPredicate =
        new(ReferenceEqualityComparer.Instance);
    private ulong[] _staticBitboard = Array.Empty<ulong>();

    private readonly List<Fact<Fluent>> _fluents = new();
    private readonly List<Fact<Static>> _statics = new();
    private readonly List<Fact<Derived>> _derived = new();
    private readonly HashSet<Constant> _objects;
    private readonly Dictionary<string, Constant[]> _compatibleObjectsByType;
    private readonly Dictionary<DerivedFactKey, DerivedEvaluationSlot>
        _derivedEvaluationSlots = new();
    private readonly List<DerivedFactKey> _derivedKeysByEvaluationSlot = new();
    private readonly List<DerivedEvaluationSlot>
        _derivedEvaluationSlotsByDerivedIndex = new();
    private readonly Stack<DerivedEvaluationWorkspace>
        _derivedEvaluationWorkspaces = new();
    private readonly Dictionary<ActionSchema, ActionGroundingPlan>
        _actionGroundingPlans = new(ReferenceEqualityComparer.Instance);
    private readonly Dictionary<ActionSchema, ActionComponentCache?>
        _actionComponentCaches = new(ReferenceEqualityComparer.Instance);
    private readonly Stack<ActionGroundingWorkspace>
        _actionGroundingWorkspaces = new();
    private int _observedActionComponentGen2Collections = GC.CollectionCount(2);
    private DerivedPredicateClosure? _derivedClosure;

    internal ReadOnlySpan<ulong> StaticBitboardWords => _staticBitboard;

    internal void SetStaticBitboardWords(ulong[] words)
    {
        ArgumentNullException.ThrowIfNull(words);
        _staticBitboard = words.ToArray();
    }

    internal void InitializeDerivedClosure()
    {
        if (_derivedClosure is not null)
            throw new InvalidOperationException(nameof(InitializeDerivedClosure));

        _derivedClosure = new DerivedPredicateClosure(
            this,
            Problem.Domain.DerivedPlan);
    }

    internal ExtendedState Expand(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.Context, this))
            throw new ArgumentException(nameof(state));

        return GetDerivedClosure().Expand(state);
    }

    internal bool IsStaticDerived(Mimir.Core.Schemas.Predicate<Derived> predicate)
    {
        ArgumentNullException.ThrowIfNull(predicate);
        if (!Problem.Domain.Contains(predicate))
            throw new ArgumentException(nameof(predicate));

        return IsStaticDerivedPrevalidated(predicate);
    }

    internal bool IsStaticDerivedPrevalidated(
        Mimir.Core.Schemas.Predicate<Derived> predicate)
        => GetDerivedClosure().IsStatic(predicate);

    internal bool IsStaticDerivedTruePrevalidated(
        Mimir.Core.Schemas.Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments)
        => GetDerivedClosure().IsStaticTrue(predicate, arguments);

    internal bool IsStaticDerivedTruePrevalidated(Fact<Derived> fact)
        => GetDerivedClosure().IsStaticTrue(
            GetDerivedEvaluationSlot(new DerivedIndex(fact.LocalIndex)));

    internal bool IsDerivedTruePrevalidated(
        Mimir.Core.Schemas.Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments,
        DerivedPredicateEvaluation? evaluation)
    {
        if (IsStaticDerivedPrevalidated(predicate))
            return IsStaticDerivedTruePrevalidated(predicate, arguments);

        return IsStateDependentDerivedTruePrevalidated(
            predicate,
            arguments,
            evaluation);
    }

    internal bool IsDerivedTruePrevalidated(
        Fact<Derived> fact,
        DerivedPredicateEvaluation? evaluation)
    {
        DerivedEvaluationSlot slot = GetDerivedEvaluationSlot(
            new DerivedIndex(fact.LocalIndex));
        if (IsStaticDerivedPrevalidated(fact.Predicate))
            return GetDerivedClosure().IsStaticTrue(slot);

        return evaluation?.IsTrue(fact.Predicate, slot)
            ?? throw new InvalidOperationException(nameof(evaluation));
    }

    internal bool IsStateDependentDerivedTruePrevalidated(
        Mimir.Core.Schemas.Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments,
        DerivedPredicateEvaluation? evaluation)
        => evaluation?.IsTrue(predicate, arguments)
            ?? throw new InvalidOperationException(nameof(evaluation));

    internal bool IsStateDependentDerivedTruePrevalidated(
        Fact<Derived> fact,
        DerivedPredicateEvaluation? evaluation)
        => evaluation?.IsTrue(
            fact.Predicate,
            GetDerivedEvaluationSlot(new DerivedIndex(fact.LocalIndex)))
            ?? throw new InvalidOperationException(nameof(evaluation));

    internal IReadOnlyList<Fact<Derived>> GetTrueDerivedFacts(
        DerivedPredicateEvaluation? evaluation)
        => GetDerivedClosure().GetTrueFacts(evaluation);

    private DerivedPredicateClosure GetDerivedClosure()
        => _derivedClosure
            ?? throw new InvalidOperationException(nameof(InitializeDerivedClosure));

    public Problem Problem { get; }

    internal InstanceContext(Problem problem, IEnumerable<Constant> objects)
    {
        Problem = problem ?? throw new ArgumentNullException(nameof(problem));
        ArgumentNullException.ThrowIfNull(objects);
        Constant[] objectArray = objects.ToArray();
        _objects = new HashSet<Constant>(
            objectArray,
            ReferenceEqualityComparer.Instance);
        _compatibleObjectsByType = new Dictionary<string, Constant[]>(
            StringComparer.OrdinalIgnoreCase)
        {
            ["object"] = objectArray,
        };
        foreach (string type in problem.Domain.TypeHierarchy.Keys)
        {
            _compatibleObjectsByType.Add(
                type,
                objectArray
                    .Where(candidate => problem.Domain.IsCompatible(candidate.Type, type))
                    .ToArray());
        }
    }

    public IReadOnlyList<Fact> AllFacts => _allFacts.AsReadOnly();
    public IReadOnlyList<Fact<Fluent>> Fluents => _fluents.AsReadOnly();
    public IReadOnlyList<Fact<Static>> Statics => _statics.AsReadOnly();
    public IReadOnlyList<Fact<Derived>> Derived => _derived.AsReadOnly();

    internal int FluentCount => _fluents.Count;
    internal int StaticCount => _statics.Count;
    internal int DerivedCount => _derived.Count;
    internal int DerivedEvaluationSlotCount => _derivedKeysByEvaluationSlot.Count;

    internal Constant[] GetCompatibleObjects(string type)
        => _compatibleObjectsByType[type];

    internal ActionGroundingPlan GetActionGroundingPlan(ActionSchema schema)
    {
        ArgumentNullException.ThrowIfNull(schema);
        if (!Problem.Domain.Contains(schema))
        {
            throw new ArgumentException(
                $"Action schema '{schema.Name}' belongs to a different domain.",
                nameof(schema));
        }

        if (_actionGroundingPlans.TryGetValue(
                schema,
                out ActionGroundingPlan? plan))
        {
            return plan;
        }

        ActionGroundingPlan compiled = ActionGroundingPlan.Compile(this, schema);
        _actionGroundingPlans.Add(schema, compiled);
        return compiled;
    }

    internal ActionComponentCache? GetActionComponentCache(
        ActionGroundingPlan plan)
    {
        ArgumentNullException.ThrowIfNull(plan);
        if (!ReferenceEquals(plan.Context, this))
        {
            throw new InvalidOperationException(
                "Action grounding plan belongs to a different problem context.");
        }

        if (_actionComponentCaches.TryGetValue(
                plan.Schema,
                out ActionComponentCache? cache))
        {
            return cache;
        }

        var created = new ActionComponentCache(
            plan.PreconditionRelevantParameters,
            plan.TransitionRelevantParameters);
        if (!created.HasCaches)
        {
            _actionComponentCaches.Add(plan.Schema, null);
            return null;
        }

        _actionComponentCaches.Add(plan.Schema, created);
        return created;
    }

    internal void ScavengeActionComponentCaches()
    {
        int gen2Collections = GC.CollectionCount(2);
        if (gen2Collections == _observedActionComponentGen2Collections)
            return;

        _observedActionComponentGen2Collections = gen2Collections;
        foreach (ActionComponentCache? cache in _actionComponentCaches.Values)
            cache?.Scavenge();
    }

    internal ActionGroundingWorkspace RentActionGroundingWorkspace(
        ActionGroundingPlan plan)
    {
        ArgumentNullException.ThrowIfNull(plan);
        if (!ReferenceEquals(plan.Context, this))
        {
            throw new InvalidOperationException(
                "Action grounding plan belongs to a different problem context.");
        }

        ActionGroundingWorkspace workspace =
            _actionGroundingWorkspaces.TryPop(
                out ActionGroundingWorkspace? available)
                ? available
                : new ActionGroundingWorkspace();
        workspace.Prepare(plan);
        return workspace;
    }

    internal void ReturnActionGroundingWorkspace(
        ActionGroundingWorkspace workspace)
    {
        ArgumentNullException.ThrowIfNull(workspace);
        workspace.PrepareForReturn();
        _actionGroundingWorkspaces.Push(workspace);
    }

    internal DerivedEvaluationWorkspace RentDerivedEvaluationWorkspace()
        => _derivedEvaluationWorkspaces.TryPop(
            out DerivedEvaluationWorkspace? workspace)
            ? workspace
            : new DerivedEvaluationWorkspace();

    internal void ReturnDerivedEvaluationWorkspace(
        DerivedEvaluationWorkspace workspace)
    {
        ArgumentNullException.ThrowIfNull(workspace);
        workspace.PrepareForReturn();
        _derivedEvaluationWorkspaces.Push(workspace);
    }

    internal bool TryGetDerivedEvaluationSlot(
        DerivedFactKey key,
        out DerivedEvaluationSlot slot)
        => _derivedEvaluationSlots.TryGetValue(key, out slot);

    internal DerivedEvaluationSlot GetOrAddDerivedEvaluationSlot(
        DerivedFactKey key)
    {
        if (_derivedEvaluationSlots.TryGetValue(key, out DerivedEvaluationSlot slot))
            return slot;

        slot = new DerivedEvaluationSlot(_derivedKeysByEvaluationSlot.Count);
        _derivedEvaluationSlots.Add(key, slot);
        _derivedKeysByEvaluationSlot.Add(key);
        return slot;
    }

    internal DerivedFactKey GetDerivedEvaluationKey(DerivedEvaluationSlot slot)
    {
        if (slot.Value < 0 || slot.Value >= _derivedKeysByEvaluationSlot.Count)
            throw new ArgumentOutOfRangeException(nameof(slot));

        return _derivedKeysByEvaluationSlot[slot.Value];
    }

    private DerivedEvaluationSlot GetDerivedEvaluationSlot(DerivedIndex index)
    {
        if (!IsValid(index))
            throw new ArgumentOutOfRangeException(nameof(index));

        return _derivedEvaluationSlotsByDerivedIndex[index.Value];
    }

    internal bool IsValid(FluentIndex index)
        => index.Value >= 0 && index.Value < _fluents.Count;

    internal bool IsValid(StaticIndex index)
        => index.Value >= 0 && index.Value < _statics.Count;

    internal bool IsValid(DerivedIndex index)
        => index.Value >= 0 && index.Value < _derived.Count;

    internal Fact GetFact(FactIndex index)
    {
        if (index.Value < 0 || index.Value >= _allFacts.Count)
        {
            throw new ArgumentOutOfRangeException(
                nameof(index),
                index.Value,
                "The fact index does not reference a registered fact.");
        }

        return _allFacts[index.Value];
    }

    internal Fact<Fluent> GetFact(FluentIndex index)
    {
        if (!IsValid(index))
        {
            throw new ArgumentOutOfRangeException(
                nameof(index),
                index.Value,
                "The fluent index does not reference a registered fluent fact.");
        }

        return _fluents[index.Value];
    }

    internal Fact<Static> GetFact(StaticIndex index)
    {
        if (!IsValid(index))
        {
            throw new ArgumentOutOfRangeException(
                nameof(index),
                index.Value,
                "The static index does not reference a registered static fact.");
        }

        return _statics[index.Value];
    }

    internal Fact<Derived> GetFact(DerivedIndex index)
    {
        if (!IsValid(index))
        {
            throw new ArgumentOutOfRangeException(
                nameof(index),
                index.Value,
                "The derived index does not reference a registered derived fact.");
        }

        return _derived[index.Value];
    }

    internal IReadOnlyList<Fact> GetFacts(Predicate predicate)
    {
        ArgumentNullException.ThrowIfNull(predicate);
        return _factsByPredicate.TryGetValue(predicate, out List<Fact>? facts)
            ? facts
            : Array.Empty<Fact>();
    }

    internal bool IsTrue(StaticIndex factIndex)
    {
        if (!IsValid(factIndex))
        {
            throw new ArgumentOutOfRangeException(
                nameof(factIndex),
                factIndex.Value,
                "The static index does not reference a registered static fact.");
        }

        int arrayIndex = factIndex.Value / 64;
        int bitIndex = factIndex.Value % 64;
        if (arrayIndex >= _staticBitboard.Length) return false;

        return (_staticBitboard[arrayIndex] & (1UL << bitIndex)) != 0;
    }

    internal Fact<T> RegisterFact<T>(Mimir.Core.Schemas.Predicate<T> predicate, IReadOnlyList<Constant> arguments)
        where T : IPredicateType

    {
        ValidatePredicateArguments(predicate, arguments, nameof(predicate), nameof(arguments));
        var key = new FactKey(predicate, arguments);
        if (_factToIndex.TryGetValue(key, out FactIndex index))
        {
            var existingFact = _allFacts[index.Value];
            if (existingFact is not Fact<T> typedFact)
                throw new InvalidOperationException(
                    $"Fact {predicate.Name} was previously registered as a different type.");
            return typedFact;
        }

        var globalIndex = new FactIndex(_allFacts.Count);

        Fact<T> fact;
        if (typeof(T) == typeof(Fluent))
        {
            fact = new Fact<T>(this, globalIndex, _fluents.Count, predicate, arguments);
            _fluents.Add((Fact<Fluent>)(object)fact);
        }
        else if (typeof(T) == typeof(Static))
        {
            fact = new Fact<T>(this, globalIndex, _statics.Count, predicate, arguments);
            _statics.Add((Fact<Static>)(object)fact);
            if (Problem.Domain.IsEqualityPredicate(predicate)
                && ReferenceEquals(arguments[0], arguments[1]))
            {
                SetStaticFactTrue(((Fact<Static>)(object)fact).LocalIndex);
            }
        }
        else if (typeof(T) == typeof(Derived))
        {
            var derivedPredicate = (Mimir.Core.Schemas.Predicate<Derived>)(object)predicate;
            DerivedEvaluationSlot evaluationSlot = GetOrAddDerivedEvaluationSlot(
                new DerivedFactKey(derivedPredicate, arguments));
            fact = new Fact<T>(this, globalIndex, _derived.Count, predicate, arguments);
            _derived.Add((Fact<Derived>)(object)fact);
            _derivedEvaluationSlotsByDerivedIndex.Add(evaluationSlot);
        }
        else
        {
            throw new NotSupportedException($"Unsupported fact type: {typeof(T)}");
        }

        _allFacts.Add(fact);
        _factToIndex[key] = globalIndex;
        if (!_factsByPredicate.TryGetValue(predicate, out List<Fact>? predicateFacts))
        {
            predicateFacts = new List<Fact>();
            _factsByPredicate.Add(predicate, predicateFacts);
        }
        predicateFacts.Add(fact);
        return fact;
    }

    internal Fact<T> RegisterFact<T>(Mimir.Core.Schemas.Predicate<T> predicate, Constant arg1)
        where T : IPredicateType
    {
        Fact<T>? existingFact = FindRegisteredFact(predicate, GetFactIndex(predicate, arg1));
        return existingFact ?? RegisterFact(predicate, new[] { arg1 });
    }

    internal Fact<T> RegisterFact<T>(
        Mimir.Core.Schemas.Predicate<T> predicate,
        Constant arg1,
        Constant arg2)
        where T : IPredicateType
    {
        Fact<T>? existingFact = FindRegisteredFact(predicate, GetFactIndex(predicate, arg1, arg2));
        return existingFact ?? RegisterFact(predicate, new[] { arg1, arg2 });
    }

    internal Fact<T> RegisterFact<T>(
        Mimir.Core.Schemas.Predicate<T> predicate,
        Constant arg1,
        Constant arg2,
        Constant arg3)
        where T : IPredicateType
    {
        Fact<T>? existingFact = FindRegisteredFact(
            predicate,
            GetFactIndex(predicate, arg1, arg2, arg3));
        return existingFact ?? RegisterFact(predicate, new[] { arg1, arg2, arg3 });
    }

    private Fact<T>? FindRegisteredFact<T>(
        Mimir.Core.Schemas.Predicate<T> predicate,
        FactIndex? index)
        where T : IPredicateType
    {
        if (index is not FactIndex registeredIndex)
            return null;

        return _allFacts[registeredIndex.Value] as Fact<T>
            ?? throw new InvalidOperationException(
                $"Fact {predicate.Name} was previously registered as a different type.");
    }

    private void SetStaticFactTrue(int localIndex)
    {
        int arrayIndex = localIndex / 64;
        if (arrayIndex >= _staticBitboard.Length)
            Array.Resize(ref _staticBitboard, arrayIndex + 1);
        _staticBitboard[arrayIndex] |= 1UL << (localIndex % 64);
    }

    public Fact<T>? FindFact<T>(Mimir.Core.Schemas.Predicate<T> predicate, IReadOnlyList<Constant> arguments)
        where T : IPredicateType
    {
        ValidatePredicateArguments(predicate, arguments, nameof(predicate), nameof(arguments));
        var key = new FactKey(predicate, arguments);
        if (!_factToIndex.TryGetValue(key, out FactIndex index)) return null;

        return _allFacts[index.Value] as Fact<T>
            ?? throw new InvalidOperationException(
                $"Fact '{predicate.Name}' was registered with an incompatible predicate type.");
    }

    internal FactIndex? GetPrevalidatedFactIndex(
        Predicate predicate,
        IReadOnlyList<Constant> arguments)
        => _factToIndex.TryGetValue(new FactKey(predicate, arguments), out FactIndex index)
            ? index
            : null;

    internal FactIndex? GetPrevalidatedFactIndex(Predicate predicate)
        => _factToIndex.TryGetValue(new FactKey(predicate), out FactIndex index)
            ? index
            : null;

    internal FactIndex? GetPrevalidatedFactIndex(Predicate predicate, Constant arg1)
        => _factToIndex.TryGetValue(new FactKey(predicate, arg1), out FactIndex index)
            ? index
            : null;

    internal FactIndex? GetPrevalidatedFactIndex(
        Predicate predicate,
        Constant arg1,
        Constant arg2)
        => _factToIndex.TryGetValue(new FactKey(predicate, arg1, arg2), out FactIndex index)
            ? index
            : null;

    internal FactIndex? GetPrevalidatedFactIndex(
        Predicate predicate,
        Constant arg1,
        Constant arg2,
        Constant arg3)
        => _factToIndex.TryGetValue(new FactKey(predicate, arg1, arg2, arg3), out FactIndex index)
            ? index
            : null;

    internal FactIndex? GetPrevalidatedFactIndex(
        Predicate predicate,
        Constant arg1,
        Constant arg2,
        Constant arg3,
        Constant arg4)
        => _factToIndex.TryGetValue(
            new FactKey(predicate, arg1, arg2, arg3, arg4),
            out FactIndex index)
            ? index
            : null;

    internal FactIndex? GetFactIndex(Predicate predicate, IReadOnlyList<Constant> arguments)
    {
        ValidatePredicateArguments(predicate, arguments, nameof(predicate), nameof(arguments));
        return GetPrevalidatedFactIndex(predicate, arguments);
    }

    internal FactIndex? GetFactIndex(Predicate predicate)
    {
        ValidatePredicateAndArity(predicate, 0, nameof(predicate), "arguments");
        return GetPrevalidatedFactIndex(predicate);
    }

    internal FactIndex? GetFactIndex(Predicate predicate, Constant arg1)
    {
        ValidatePredicateAndArity(predicate, 1, nameof(predicate), "arguments");
        ValidatePredicateArgument(predicate, arg1, 0, "arguments");
        return GetPrevalidatedFactIndex(predicate, arg1);
    }

    internal FactIndex? GetFactIndex(Predicate predicate, Constant arg1, Constant arg2)
    {
        ValidatePredicateAndArity(predicate, 2, nameof(predicate), "arguments");
        ValidatePredicateArgument(predicate, arg1, 0, "arguments");
        ValidatePredicateArgument(predicate, arg2, 1, "arguments");
        return GetPrevalidatedFactIndex(predicate, arg1, arg2);
    }

    internal FactIndex? GetFactIndex(Predicate predicate, Constant arg1, Constant arg2, Constant arg3)
    {
        ValidatePredicateAndArity(predicate, 3, nameof(predicate), "arguments");
        ValidatePredicateArgument(predicate, arg1, 0, "arguments");
        ValidatePredicateArgument(predicate, arg2, 1, "arguments");
        ValidatePredicateArgument(predicate, arg3, 2, "arguments");
        return GetPrevalidatedFactIndex(predicate, arg1, arg2, arg3);
    }

    internal void ValidatePredicateArguments(
        Predicate predicate,
        IReadOnlyList<Constant> arguments,
        string predicateParameterName,
        string argumentsParameterName)
    {
        ArgumentNullException.ThrowIfNull(predicate, predicateParameterName);
        ArgumentNullException.ThrowIfNull(arguments, argumentsParameterName);

        ValidatePredicateAndArity(
            predicate,
            arguments.Count,
            predicateParameterName,
            argumentsParameterName);

        for (int i = 0; i < arguments.Count; i++)
            ValidatePredicateArgument(predicate, arguments[i], i, argumentsParameterName);
    }

    private void ValidatePredicateAndArity(
        Predicate predicate,
        int argumentCount,
        string predicateParameterName,
        string argumentsParameterName)
    {
        ArgumentNullException.ThrowIfNull(predicate, predicateParameterName);

        if (!Problem.Domain.Contains(predicate))
            throw new ArgumentException(
                $"Predicate '{predicate.Name}' belongs to a different domain.",
                predicateParameterName);
        if (predicate.Parameters.Count != argumentCount)
            throw new ArgumentException(
                $"Predicate '{predicate.Name}' expects {predicate.Parameters.Count} arguments, got {argumentCount}.",
                argumentsParameterName);
    }

    private void ValidatePredicateArgument(
        Predicate predicate,
        Constant? argument,
        int argumentIndex,
        string argumentsParameterName)
    {
        if (argument is null)
            throw new ArgumentException("Arguments cannot contain null values.", argumentsParameterName);
        if (!_objects.Contains(argument))
            throw new ArgumentException(
                $"Object '{argument.Name}' belongs to a different problem.",
                argumentsParameterName);
        if (!Problem.Domain.IsCompatible(argument.Type, predicate.Parameters[argumentIndex].Type))
            throw new ArgumentException(
                $"Object '{argument.Name}' has type '{argument.Type}', expected '{predicate.Parameters[argumentIndex].Type}'.",
                argumentsParameterName);
    }

    internal void ValidateActionBinding(
        ActionSchema schema,
        IReadOnlyList<Constant> arguments,
        string schemaParameterName,
        string argumentsParameterName)
    {
        ArgumentNullException.ThrowIfNull(schema, schemaParameterName);
        ArgumentNullException.ThrowIfNull(arguments, argumentsParameterName);

        if (!Problem.Domain.Contains(schema))
            throw new ArgumentException(
                $"Action schema '{schema.Name}' belongs to a different domain.",
                schemaParameterName);
        if (schema.Parameters.Count != arguments.Count)
            throw new ArgumentException(
                $"Action schema '{schema.Name}' expects {schema.Parameters.Count} arguments, got {arguments.Count}.",
                argumentsParameterName);

        for (int i = 0; i < arguments.Count; i++)
        {
            Constant argument = arguments[i]
                ?? throw new ArgumentException("Arguments cannot contain null values.", argumentsParameterName);
            if (!_objects.Contains(argument))
                throw new ArgumentException(
                    $"Object '{argument.Name}' belongs to a different problem.",
                    argumentsParameterName);
            if (!Problem.Domain.IsCompatible(argument.Type, schema.Parameters[i].Type))
                throw new ArgumentException(
                    $"Object '{argument.Name}' has type '{argument.Type}', expected '{schema.Parameters[i].Type}'.",
                    argumentsParameterName);
        }
    }

    internal bool ContainsObject(Constant constant) => _objects.Contains(constant);
}
