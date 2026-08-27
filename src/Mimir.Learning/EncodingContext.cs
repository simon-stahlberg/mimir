using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Learning;

public sealed class EncodingContext
{
    internal sealed record EncodedFact(
        Predicate Predicate,
        int[] LocalObjectIds);

    internal sealed record StaticRelationGroup(
        Predicate Predicate,
        int[] LocalObjectIds);

    internal sealed record TypeRelationGroup(
        string TypeName,
        int[] LocalObjectIds);

    internal sealed class ProblemData
    {
        public required Problem Problem { get; init; }
        public required Dictionary<Constant, int> LocalObjectIds { get; init; }
        public required IReadOnlyList<EncodedFact> StaticFacts { get; init; }
        public required IReadOnlyList<StaticRelationGroup> StaticRelations { get; init; }
        public required IReadOnlyList<TypeRelationGroup> TypeRelations { get; init; }
    }

    private readonly Dictionary<Problem, ProblemData> _problemData = new(
        ReferenceEqualityComparer.Instance);
    private readonly Dictionary<string, List<int>> _relations = new(
        StringComparer.Ordinal);
    private readonly Dictionary<string, IReadOnlyList<int>> _relationViews = new(
        StringComparer.Ordinal);
    private readonly List<KeyValuePair<string, IReadOnlyList<int>>> _relationEntries = [];
    private readonly List<List<int>> _relationValues = [];
    private readonly List<int> _nodeSizes = [];
    private readonly List<int> _objectSizes = [];
    private readonly List<int> _objectIndices = [];
    private readonly List<int> _actionSizes = [];
    private readonly List<int> _actionIndices = [];
    private readonly List<int> _virtualSizes = [];
    private readonly List<int> _virtualIndices = [];
    private readonly List<int> _auxiliarySizes = [];
    private readonly List<int> _auxiliaryIndices = [];
    private readonly List<int> _currentActionIds = [];
    private readonly List<int> _currentVirtualIds = [];
    private readonly List<int> _currentAuxiliaryIds = [];
    private readonly ReadOnlyCollection<int> _currentActionIdsView;
    private readonly ReadOnlyCollection<int> _currentVirtualIdsView;
    private readonly ReadOnlyCollection<int> _currentAuxiliaryIdsView;
    private Domain? _domain;
    private Problem? _currentProblem;
    private ProblemData? _currentProblemData;
    private Dictionary<Constant, int>? _currentObjectIds;
    private IReadOnlyDictionary<Constant, int>? _currentObjectIdsView;
    private IReadOnlyList<int>? _currentObjectIdValues;
    private Dictionary<(int First, int Second), int>? _currentObjectPairIds;
    private int _currentNodeOffset;
    private int _nodeCount;

    public EncodingContext()
    {
        Relations = new ReadOnlyDictionary<string, IReadOnlyList<int>>(
            _relationViews);
        NodeSizes = _nodeSizes.AsReadOnly();
        ObjectSizes = _objectSizes.AsReadOnly();
        ObjectIndices = _objectIndices.AsReadOnly();
        ActionSizes = _actionSizes.AsReadOnly();
        ActionIndices = _actionIndices.AsReadOnly();
        VirtualSizes = _virtualSizes.AsReadOnly();
        VirtualIndices = _virtualIndices.AsReadOnly();
        AuxiliarySizes = _auxiliarySizes.AsReadOnly();
        AuxiliaryIndices = _auxiliaryIndices.AsReadOnly();
        _currentActionIdsView = _currentActionIds.AsReadOnly();
        _currentVirtualIdsView = _currentVirtualIds.AsReadOnly();
        _currentAuxiliaryIdsView = _currentAuxiliaryIds.AsReadOnly();
    }

    public int BatchCount => _nodeSizes.Count;
    public int NodeCount => _nodeCount;
    public int CurrentNodeOffset
    {
        get
        {
            EnsureInstanceActive();
            return _currentNodeOffset;
        }
    }

    public int CurrentNodeCount
    {
        get
        {
            EnsureInstanceActive();
            return _nodeCount - _currentNodeOffset;
        }
    }

    public IReadOnlyList<int> NodeSizes { get; }
    public IReadOnlyList<int> ObjectSizes { get; }
    public IReadOnlyList<int> ObjectIndices { get; }
    public IReadOnlyList<int> ActionSizes { get; }
    public IReadOnlyList<int> ActionIndices { get; }
    public IReadOnlyList<int> VirtualSizes { get; }
    public IReadOnlyList<int> VirtualIndices { get; }
    public IReadOnlyList<int> AuxiliarySizes { get; }
    public IReadOnlyList<int> AuxiliaryIndices { get; }
    public IReadOnlyDictionary<string, IReadOnlyList<int>> Relations { get; }

    public Problem Problem
        => _currentProblem
            ?? throw new InvalidOperationException("No encoding instance is active.");

    public IReadOnlyDictionary<Constant, int> ObjectIds
        => _currentObjectIdsView
            ?? throw new InvalidOperationException("No encoding instance is active.");

    public IReadOnlyList<int> ObjectIdValues
        => _currentObjectIdValues
            ?? throw new InvalidOperationException("No encoding instance is active.");

    public IReadOnlyList<int> ActionIds
    {
        get
        {
            EnsureInstanceActive();
            return _currentActionIdsView;
        }
    }

    public IReadOnlyList<int> VirtualIds
    {
        get
        {
            EnsureInstanceActive();
            return _currentVirtualIdsView;
        }
    }

    public IReadOnlyList<int> AuxiliaryIds
    {
        get
        {
            EnsureInstanceActive();
            return _currentAuxiliaryIdsView;
        }
    }

    public void BeginInstance(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        if (_currentProblem is not null)
        {
            throw new InvalidOperationException(
                "End the active encoding instance before beginning another one.");
        }
        if (_domain is not null && !ReferenceEquals(problem.Domain, _domain))
        {
            throw new ArgumentException(
                "Every problem in an encoding batch must share the exact Domain.",
                nameof(problem));
        }

        if (!_problemData.TryGetValue(problem, out ProblemData? data))
        {
            data = CreateProblemData(problem);
            _problemData.Add(problem, data);
        }

        EnsureBatchCanAllocate(problem.AllObjects.Count);
        int nodeOffset = _nodeCount;
        var objectIds = new Dictionary<Constant, int>(
            ReferenceEqualityComparer.Instance);
        var objectIdValues = new int[problem.AllObjects.Count];
        for (int localId = 0; localId < problem.AllObjects.Count; localId++)
        {
            int objectId = checked(nodeOffset + localId);
            if (!objectIds.TryAdd(problem.AllObjects[localId], objectId))
            {
                throw new InvalidOperationException(
                    "Problem.AllObjects contains the same object reference more than once.");
            }
            objectIdValues[localId] = objectId;
        }

        _domain ??= problem.Domain;
        _currentProblem = problem;
        _currentProblemData = data;
        _currentObjectIds = objectIds;
        _currentObjectIdsView = new ReadOnlyDictionary<Constant, int>(objectIds);
        _currentObjectIdValues = Array.AsReadOnly(objectIdValues);
        _currentObjectPairIds = [];
        _currentNodeOffset = nodeOffset;
        _currentActionIds.Clear();
        _currentVirtualIds.Clear();
        _currentAuxiliaryIds.Clear();
        _objectIndices.AddRange(objectIdValues);
        _nodeCount = checked(_nodeCount + objectIdValues.Length);
    }

    public void EndInstance()
    {
        EnsureInstanceActive();

        _nodeSizes.Add(_nodeCount - _currentNodeOffset);
        _objectSizes.Add(ObjectIdValues.Count);
        _actionSizes.Add(_currentActionIds.Count);
        _virtualSizes.Add(_currentVirtualIds.Count);
        _auxiliarySizes.Add(_currentAuxiliaryIds.Count);

        _currentProblem = null;
        _currentProblemData = null;
        _currentObjectIds = null;
        _currentObjectIdsView = null;
        _currentObjectIdValues = null;
        _currentObjectPairIds = null;
        _currentNodeOffset = 0;
        _currentActionIds.Clear();
        _currentVirtualIds.Clear();
        _currentAuxiliaryIds.Clear();
    }

    public int GetObjectId(Constant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        Dictionary<Constant, int> objectIds = _currentObjectIds
            ?? throw new InvalidOperationException("No encoding instance is active.");
        if (!objectIds.TryGetValue(value, out int objectId))
        {
            throw new ArgumentException(
                "Object does not belong to the active encoding problem.",
                nameof(value));
        }
        return objectId;
    }

    public int NewActionId()
        => Allocate(_currentActionIds, _actionIndices);

    public int NewVirtualId()
        => Allocate(_currentVirtualIds, _virtualIndices);

    public int NewOrExistingVirtualId()
        => _currentVirtualIds.Count > 0
            ? _currentVirtualIds[^1]
            : NewVirtualId();

    public int NewAuxiliaryId()
        => Allocate(_currentAuxiliaryIds, _auxiliaryIndices);

    public int NewOrExistingObjectPairId(Constant first, Constant second)
    {
        int firstId = GetObjectId(first);
        int secondId = GetObjectId(second);
        if (firstId == secondId)
            return firstId;

        Dictionary<(int First, int Second), int> pairIds = _currentObjectPairIds
            ?? throw new InvalidOperationException("No encoding instance is active.");
        if (pairIds.TryGetValue((firstId, secondId), out int pairId))
            return pairId;

        pairId = NewAuxiliaryId();
        pairIds.Add((firstId, secondId), pairId);
        return pairId;
    }

    public bool TryGetObjectPairId(
        Constant first,
        Constant second,
        out int pairId)
    {
        int firstId = GetObjectId(first);
        int secondId = GetObjectId(second);
        if (firstId == secondId)
        {
            pairId = firstId;
            return true;
        }

        Dictionary<(int First, int Second), int> pairIds = _currentObjectPairIds
            ?? throw new InvalidOperationException("No encoding instance is active.");
        return pairIds.TryGetValue((firstId, secondId), out pairId);
    }

    internal ProblemData CurrentProblemData
        => _currentProblemData
            ?? throw new InvalidOperationException("No encoding instance is active.");

    internal int RelationCount => _relationEntries.Count;

    internal KeyValuePair<string, IReadOnlyList<int>> GetRelation(int relationIndex)
    {
        if (relationIndex < 0 || relationIndex >= _relationEntries.Count)
        {
            throw new ArgumentOutOfRangeException(
                nameof(relationIndex),
                relationIndex,
                "Relation index is outside this encoding context.");
        }
        return _relationEntries[relationIndex];
    }

    /// <summary>
    /// Gets the total number of flattened int32 values across all relations.
    /// </summary>
    /// <exception cref="OverflowException">
    /// The combined relation length exceeds <see cref="int.MaxValue"/>.
    /// </exception>
    public int RelationValueCount
    {
        get
        {
            int count = 0;
            foreach (List<int> values in _relationValues)
                count = checked(count + values.Count);
            return count;
        }
    }

    /// <summary>
    /// Copies every flattened relation into <paramref name="destination"/> in
    /// the enumeration order of <see cref="Relations"/>.
    /// </summary>
    /// <returns>The number of copied int32 values.</returns>
    public int CopyRelationValues(Span<int> destination)
    {
        int required = RelationValueCount;
        if (destination.Length < required)
        {
            throw new ArgumentException(
                $"The destination length {destination.Length} is smaller than the required {required} values.",
                nameof(destination));
        }

        int offset = 0;
        foreach (List<int> values in _relationValues)
        {
            Span<int> source = CollectionsMarshal.AsSpan(values);
            source.CopyTo(destination[offset..]);
            offset += source.Length;
        }
        return offset;
    }

    internal void EnsureCanAllocate(int count)
    {
        EnsureInstanceActive();
        EnsureBatchCanAllocate(count);
    }

    internal List<int> GetOrCreateRelation(string name)
    {
        ArgumentNullException.ThrowIfNull(name);
        EnsureInstanceActive();
        if (_relations.TryGetValue(name, out List<int>? values))
            return values;

        values = [];
        IReadOnlyList<int> view = values.AsReadOnly();
        _relations.Add(name, values);
        _relationViews.Add(name, view);
        _relationEntries.Add(KeyValuePair.Create(name, view));
        _relationValues.Add(values);
        return values;
    }

    private int Allocate(List<int> currentIds, List<int> batchIndices)
    {
        EnsureCanAllocate(1);
        int nodeId = _nodeCount;
        currentIds.Add(nodeId);
        batchIndices.Add(nodeId);
        _nodeCount = checked(_nodeCount + 1);
        return nodeId;
    }

    private void EnsureBatchCanAllocate(int count)
    {
        if (count < 0)
        {
            throw new ArgumentOutOfRangeException(
                nameof(count),
                count,
                "Allocation count cannot be negative.");
        }

        try
        {
            _ = checked(_nodeCount + count);
        }
        catch (OverflowException exception)
        {
            throw new OverflowException(
                "The encoding batch cannot contain more than Int32.MaxValue nodes.",
                exception);
        }
    }

    private void EnsureInstanceActive()
    {
        if (_currentProblem is null)
            throw new InvalidOperationException("No encoding instance is active.");
    }

    private static ProblemData CreateProblemData(Problem problem)
    {
        var localObjectIds = new Dictionary<Constant, int>(
            ReferenceEqualityComparer.Instance);
        for (int localId = 0; localId < problem.AllObjects.Count; localId++)
        {
            if (!localObjectIds.TryAdd(problem.AllObjects[localId], localId))
            {
                throw new InvalidOperationException(
                    "Problem.AllObjects contains the same object reference more than once.");
            }
        }

        var staticFacts = new List<EncodedFact>();
        var staticGroups = new Dictionary<Predicate, List<int>>(
            ReferenceEqualityComparer.Instance);
        foreach (Fact<Static> fact in problem._initialStaticFacts)
        {
            if (ReferenceEquals(fact.Predicate, problem.Domain.EqualityPredicate))
                continue;

            int[] argumentIds = GetLocalObjectIds(fact, localObjectIds);
            staticFacts.Add(new EncodedFact(fact.Predicate, argumentIds));
            if (!staticGroups.TryGetValue(fact.Predicate, out List<int>? group))
            {
                group = [];
                staticGroups.Add(fact.Predicate, group);
            }
            group.AddRange(argumentIds);
        }

        StaticRelationGroup[] staticRelations = staticGroups
            .Select(pair => new StaticRelationGroup(
                pair.Key,
                pair.Value.ToArray()))
            .ToArray();
        var typeNames = new List<string> { "object" };
        if (problem.Domain.UsesTyping)
            typeNames.AddRange(problem.Domain.TypeHierarchy.Keys);

        var typeRelations = new List<TypeRelationGroup>();
        foreach (string typeName in typeNames)
        {
            int[] ids = problem.AllObjects
                .Select((value, localId) => (value, localId))
                .Where(pair => problem.Domain.IsCompatible(
                    pair.value.Type,
                    typeName))
                .Select(pair => pair.localId)
                .ToArray();
            typeRelations.Add(new TypeRelationGroup(typeName, ids));
        }

        return new ProblemData
        {
            Problem = problem,
            LocalObjectIds = localObjectIds,
            StaticFacts = staticFacts.AsReadOnly(),
            StaticRelations = Array.AsReadOnly(staticRelations),
            TypeRelations = typeRelations.AsReadOnly(),
        };
    }

    private static int[] GetLocalObjectIds(
        Fact fact,
        Dictionary<Constant, int> localObjectIds)
    {
        var result = new int[fact.Arguments.Count];
        for (int index = 0; index < fact.Arguments.Count; index++)
        {
            if (!localObjectIds.TryGetValue(fact.Arguments[index], out int localId))
            {
                throw new InvalidOperationException(
                    "A fact contains an object outside Problem.AllObjects.");
            }
            result[index] = localId;
        }
        return result;
    }
}
