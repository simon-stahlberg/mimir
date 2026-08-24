using Mimir.Core.Algorithms.Graph;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;

namespace Mimir.Core.Engines;

public sealed partial class ConjunctiveConditionBindingGenerator
{
    internal enum FluentDeltaMode
    {
        Any,
        AddedThisLayer,
        BeforeThisLayer,
    }

    internal sealed class FluentDelta
    {
        private ulong[] _bits = Array.Empty<ulong>();
        private readonly Dictionary<FluentPredicate, List<Fact>> _factsByPredicate =
            new(ReferenceEqualityComparer.Instance);

        public void Reset(IEnumerable<Fact<Fluent>> facts, int fluentCount)
        {
            int requiredWords = (fluentCount + 63) / 64;
            if (_bits.Length < requiredWords)
                _bits = new ulong[requiredWords];
            else
                Array.Clear(_bits);

            foreach (List<Fact> predicateFacts in _factsByPredicate.Values)
                predicateFacts.Clear();

            foreach (Fact<Fluent> fact in facts)
            {
                int wordIndex = fact.LocalIndex / 64;
                _bits[wordIndex] |= 1UL << (fact.LocalIndex % 64);
                if (!_factsByPredicate.TryGetValue(
                        fact.Predicate,
                        out List<Fact>? predicateFacts))
                {
                    predicateFacts = new List<Fact>();
                    _factsByPredicate.Add(fact.Predicate, predicateFacts);
                }

                predicateFacts.Add(fact);
            }
        }

        public bool Contains(FluentIndex factIndex)
        {
            int wordIndex = factIndex.Value / 64;
            return wordIndex < _bits.Length
                && (_bits[wordIndex] & (1UL << (factIndex.Value % 64))) != 0;
        }

        public bool HasFacts(FluentPredicate predicate)
            => _factsByPredicate.TryGetValue(predicate, out List<Fact>? facts)
                && facts.Count > 0;

        public IReadOnlyList<Fact> GetFacts(FluentPredicate predicate)
            => _factsByPredicate.TryGetValue(predicate, out List<Fact>? facts)
                ? facts
                : Array.Empty<Fact>();
    }

    internal readonly struct BindingEvaluationState
    {
        private readonly ExtendedState? _extendedState;
        private readonly FluentDelta? _delta;
        private readonly FluentDeltaMode[]? _deltaModes;

        public State State { get; }

        public BindingEvaluationState(ExtendedState state)
        {
            _extendedState = state;
            _delta = null;
            _deltaModes = null;
            State = state.State;
        }

        public BindingEvaluationState(State state)
        {
            _extendedState = null;
            _delta = null;
            _deltaModes = null;
            State = state;
        }

        public BindingEvaluationState(
            State state,
            FluentDelta delta,
            FluentDeltaMode[] deltaModes)
        {
            _extendedState = null;
            _delta = delta;
            _deltaModes = deltaModes;
            State = state;
        }

        public bool IsFluentTrue(
            FluentIndex factIndex,
            int positiveFluentOrdinal)
        {
            bool isTrue = State.IsTrue(factIndex);
            if (_delta is null || positiveFluentOrdinal < 0)
                return isTrue;

            bool addedThisLayer = _delta.Contains(factIndex);
            return _deltaModes![positiveFluentOrdinal] switch
            {
                FluentDeltaMode.Any => isTrue,
                FluentDeltaMode.AddedThisLayer => addedThisLayer,
                FluentDeltaMode.BeforeThisLayer => isTrue && !addedThisLayer,
                _ => throw new InvalidOperationException(),
            };
        }

        public IReadOnlyList<Fact> GetCandidateFacts(
            FluentPredicate predicate,
            int positiveFluentOrdinal,
            IReadOnlyList<Fact> registeredFacts)
        {
            if (_delta is null
                || _deltaModes![positiveFluentOrdinal]
                    != FluentDeltaMode.AddedThisLayer)
            {
                return registeredFacts;
            }

            return _delta.GetFacts(predicate);
        }

        public bool IsStateDependentDerivedTruePrevalidated(
            Mimir.Core.Schemas.Predicate<Derived> predicate,
            IReadOnlyList<Constant> arguments)
        {
            if (_extendedState is null)
            {
                throw new InvalidOperationException(
                    "Derived conditions require an extended state.");
            }

            return _extendedState.IsStateDependentDerivedTruePrevalidated(
                predicate,
                arguments);
        }
    }

    internal delegate bool StaticConstraint(Constant[] binding);
    internal delegate bool DynamicConstraint(Constant[] binding, BindingEvaluationState state);

    internal interface IUnaryConstraint
    {
        bool Evaluate(int localIndex, BindingEvaluationState state);
    }

    internal interface IBinaryConstraint
    {
        void Synchronize();
        bool Evaluate(int leftLocalIndex, int rightLocalIndex, BindingEvaluationState state);
    }

    private sealed class FluentUnaryConstraint : IUnaryConstraint
    {
        private readonly int[] _localFactIds;
        private readonly Polarity _polarity;
        private readonly int _positiveFluentOrdinal;

        public FluentUnaryConstraint(
            int[] localFactIds,
            Polarity polarity,
            int positiveFluentOrdinal)
        {
            _localFactIds = localFactIds;
            _polarity = polarity;
            _positiveFluentOrdinal = positiveFluentOrdinal;
        }

        public bool Evaluate(int localIndex, BindingEvaluationState state)
        {
            int factId = _localFactIds[localIndex];
            bool isTrue = factId >= 0
                && state.IsFluentTrue(
                    new FluentIndex(factId),
                    _positiveFluentOrdinal);
            return _polarity == Polarity.Positive ? isTrue : !isTrue;
        }
    }

    private sealed class DerivedUnaryConstraint : IUnaryConstraint
    {
        private readonly CompiledConditionLiteral _literal;
        private readonly IReadOnlyList<Constant>[] _arguments;

        public DerivedUnaryConstraint(
            CompiledConditionLiteral literal,
            IReadOnlyList<Constant>[] arguments)
        {
            _literal = literal;
            _arguments = arguments;
        }

        public bool Evaluate(int localIndex, BindingEvaluationState state)
            => _literal.EvaluateStateDependentDerived(
                _arguments[localIndex],
                state);
    }

    private sealed class FluentBinaryConstraint : IBinaryConstraint
    {
        private readonly CompiledConditionLiteral _literal;
        private readonly IReadOnlyList<Dictionary<Constant, int>> _partitionIndices;
        private readonly int[] _encodedLocalFactIds;
        private readonly int _leftProjectionIndex;
        private readonly int _rightProjectionIndex;
        private readonly int _rightCount;
        private readonly Polarity _polarity;
        private int _processedFactCount;

        public FluentBinaryConstraint(
            CompiledConditionLiteral literal,
            IReadOnlyList<Dictionary<Constant, int>> partitionIndices,
            int leftVariableIndex,
            int rightVariableIndex,
            int leftCount,
            int rightCount)
        {
            _literal = literal;
            _partitionIndices = partitionIndices;
            // Intentionally dense: relaxed grounding can register most or all pairs, and this
            // table is read for every tested binding.
            _encodedLocalFactIds = new int[checked(leftCount * rightCount)];
            _leftProjectionIndex = Array.IndexOf(
                literal.DistinctVariableIndices,
                leftVariableIndex);
            _rightProjectionIndex = Array.IndexOf(
                literal.DistinctVariableIndices,
                rightVariableIndex);
            _rightCount = rightCount;
            _polarity = literal.Polarity;
        }

        public void Synchronize()
        {
            IReadOnlyList<Fact> facts = _literal.GetRegisteredFacts();
            if (_processedFactCount == facts.Count)
                return;

            var projectedLocalValues = new int[2];
            for (int factIndex = _processedFactCount; factIndex < facts.Count; factIndex++)
            {
                var fact = (Fact<Fluent>)facts[factIndex];
                if (!_literal.TryProjectFact(fact, _partitionIndices, projectedLocalValues))
                    continue;

                int leftLocalIndex = projectedLocalValues[_leftProjectionIndex];
                int rightLocalIndex = projectedLocalValues[_rightProjectionIndex];
                _encodedLocalFactIds[(leftLocalIndex * _rightCount) + rightLocalIndex] =
                    checked(fact.LocalIndex + 1);
            }

            _processedFactCount = facts.Count;
        }

        public bool Evaluate(int leftLocalIndex, int rightLocalIndex, BindingEvaluationState state)
        {
            int encodedFactId =
                _encodedLocalFactIds[(leftLocalIndex * _rightCount) + rightLocalIndex];
            bool isTrue = encodedFactId > 0
                && state.IsFluentTrue(
                    new FluentIndex(encodedFactId - 1),
                    _literal.PositiveFluentOrdinal);
            return _polarity == Polarity.Positive ? isTrue : !isTrue;
        }
    }

    internal sealed class DerivedBinaryConstraint
    {
        private readonly CompiledConditionLiteral _literal;
        private readonly Constant[] _leftPartition;
        private readonly Constant[] _rightPartition;
        private readonly int _leftVariableIndex;
        private readonly int _rightVariableIndex;

        public DerivedBinaryConstraint(
            CompiledConditionLiteral literal,
            Constant[] leftPartition,
            Constant[] rightPartition,
            int leftVariableIndex,
            int rightVariableIndex)
        {
            _literal = literal;
            _leftPartition = leftPartition;
            _rightPartition = rightPartition;
            _leftVariableIndex = leftVariableIndex;
            _rightVariableIndex = rightVariableIndex;
        }

        public bool Evaluate(int leftLocalIndex, int rightLocalIndex, BindingEvaluationState state)
        {
            IReadOnlyList<Constant> arguments = _literal.GroundArguments(
                _leftVariableIndex,
                _leftPartition[leftLocalIndex],
                _rightVariableIndex,
                _rightPartition[rightLocalIndex]);
            return _literal.EvaluateStateDependentDerived(arguments, state);
        }
    }

    private sealed class DynamicCompatibilityProvider : KPartiteKClique.ICompatibilityProvider
    {
        private int[] _partitionOffsets = Array.Empty<int>();
        private CompatibilityMasks?[] _pairMasks = Array.Empty<CompatibilityMasks?>();
        private int _capacity;

        public void Reset(int[] partitionOffsets, int variableCount)
        {
            _partitionOffsets = partitionOffsets;
            if (variableCount > _capacity)
            {
                _capacity = variableCount;
                _pairMasks = new CompatibilityMasks?[checked(_capacity * _capacity)];
            }

            foreach (CompatibilityMasks? masks in _pairMasks)
                masks?.Deactivate();
        }

        public bool HasCompatibility(int sourcePartition, int targetPartition)
            => GetMasks(sourcePartition, targetPartition)?.IsActive == true;

        public ReadOnlySpan<ulong> GetCompatibilityMask(int sourcePartition, int sourceVertex, int targetPartition)
        {
            CompatibilityMasks? masks = GetMasks(sourcePartition, targetPartition);
            if (masks is null || !masks.IsActive)
                return default;

            int localIndex = sourceVertex - _partitionOffsets[sourcePartition];
            return masks.GetRow(localIndex);
        }

        public CompatibilityMasks PrepareMasks(
            int sourcePartition,
            int targetPartition,
            int rowCount,
            int wordsPerRow)
        {
            int pairIndex = checked((sourcePartition * _capacity) + targetPartition);
            CompatibilityMasks masks = _pairMasks[pairIndex] ??= new CompatibilityMasks();
            masks.Activate(rowCount, wordsPerRow);
            return masks;
        }

        public void Release()
        {
            _partitionOffsets = Array.Empty<int>();
            foreach (CompatibilityMasks? masks in _pairMasks)
                masks?.Deactivate();
        }

        private CompatibilityMasks? GetMasks(int sourcePartition, int targetPartition)
            => _pairMasks[checked((sourcePartition * _capacity) + targetPartition)];
    }

    private sealed class CompatibilityMasks
    {
        private ulong[] _bits = Array.Empty<ulong>();
        private int _wordsPerRow;

        public bool IsActive { get; private set; }

        public void Activate(int rowCount, int wordsPerRow)
        {
            _wordsPerRow = wordsPerRow;
            int requiredLength = checked(rowCount * wordsPerRow);
            if (_bits.Length < requiredLength)
                _bits = new ulong[requiredLength];

            IsActive = true;
        }

        public void Deactivate() => IsActive = false;

        public Span<ulong> GetMutableRow(int rowIndex)
            => _bits.AsSpan(checked(rowIndex * _wordsPerRow), _wordsPerRow);

        public ReadOnlySpan<ulong> GetRow(int rowIndex)
            => _bits.AsSpan(checked(rowIndex * _wordsPerRow), _wordsPerRow);
    }

    internal sealed class PositiveRelationIndex
    {
        private readonly int[][]?[] _supportsByVariable;

        public int[] VariableIndices { get; }
        public int[][] Rows { get; }

        public PositiveRelationIndex(
            int variableCount,
            int[] variableIndices,
            int[][] rows,
            Constant[][] partitions)
        {
            VariableIndices = variableIndices;
            Rows = rows;
            _supportsByVariable = new int[][]?[variableCount];

            for (int column = 0; column < variableIndices.Length; column++)
            {
                int variableIndex = variableIndices[column];
                var supportLists = new List<int>?[partitions[variableIndex].Length];
                for (int rowIndex = 0; rowIndex < rows.Length; rowIndex++)
                {
                    int localValue = rows[rowIndex][column];
                    supportLists[localValue] ??= new List<int>();
                    supportLists[localValue]!.Add(rowIndex);
                }

                var supports = new int[supportLists.Length][];
                for (int localValue = 0; localValue < supportLists.Length; localValue++)
                {
                    supports[localValue] = supportLists[localValue]?.ToArray()
                        ?? Array.Empty<int>();
                }

                _supportsByVariable[variableIndex] = supports;
            }
        }

        public int[][]? GetSupports(int variableIndex)
            => _supportsByVariable[variableIndex];
    }

    private sealed class PositiveRelationCursor
    {
        private PositiveRelationIndex? _index;
        private readonly List<int> _rootRows = new();
        private List<int>[] _activeRowsByDepth = Array.Empty<List<int>>();

        public bool Reset(
            PositiveRelationIndex index,
            List<int>[] validIndices,
            bool[][] candidateMembership)
        {
            _index = index;
            EnsureDepthCapacity(index.VariableIndices.Length);
            _rootRows.Clear();
            for (int depth = 0; depth < index.VariableIndices.Length; depth++)
                _activeRowsByDepth[depth].Clear();

            int[]? seedRows = null;
            for (int column = 0; column < index.VariableIndices.Length; column++)
            {
                int variableIndex = index.VariableIndices[column];
                if (validIndices[variableIndex].Count != 1)
                    continue;

                int[][] supports = index.GetSupports(variableIndex)
                    ?? throw new InvalidOperationException(
                        "A positive relation variable has no precompiled supports.");
                int[] candidateRows = supports[validIndices[variableIndex][0]];
                if (seedRows is null || candidateRows.Length < seedRows.Length)
                    seedRows = candidateRows;
            }

            if (seedRows is null)
            {
                for (int rowIndex = 0; rowIndex < index.Rows.Length; rowIndex++)
                {
                    if (IsRowValid(index, rowIndex, candidateMembership))
                        _rootRows.Add(rowIndex);
                }
            }
            else
            {
                for (int seedIndex = 0; seedIndex < seedRows.Length; seedIndex++)
                {
                    int rowIndex = seedRows[seedIndex];
                    if (IsRowValid(index, rowIndex, candidateMembership))
                        _rootRows.Add(rowIndex);
                }
            }

            return _rootRows.Count > 0;
        }

        private static bool IsRowValid(
            PositiveRelationIndex index,
            int rowIndex,
            bool[][] candidateMembership)
        {
            int[] row = index.Rows[rowIndex];
            for (int column = 0; column < index.VariableIndices.Length; column++)
            {
                int variableIndex = index.VariableIndices[column];
                if (!candidateMembership[variableIndex][row[column]])
                    return false;
            }

            return true;
        }

        public bool TryExtend(
            ReadOnlySpan<int> assignedPartitions,
            int variableIndex,
            int localValue)
        {
            PositiveRelationIndex index = _index
                ?? throw new InvalidOperationException();
            int[][]? supports = index.GetSupports(variableIndex);
            if (supports is null)
                return true;

            int relationDepth = 0;
            for (int depth = 0; depth < assignedPartitions.Length - 1; depth++)
            {
                if (index.GetSupports(assignedPartitions[depth]) is not null)
                    relationDepth++;
            }

            List<int> activeRows = _activeRowsByDepth[relationDepth];
            activeRows.Clear();

            List<int> parentRows = relationDepth == 0
                ? _rootRows
                : _activeRowsByDepth[relationDepth - 1];
            int[] supportedRows = supports[localValue];
            int parentIndex = 0;
            int supportIndex = 0;
            while (parentIndex < parentRows.Count
                && supportIndex < supportedRows.Length)
            {
                int parentRow = parentRows[parentIndex];
                int supportedRow = supportedRows[supportIndex];
                if (parentRow < supportedRow)
                {
                    parentIndex++;
                    continue;
                }
                if (supportedRow < parentRow)
                {
                    supportIndex++;
                    continue;
                }

                activeRows.Add(parentRow);
                parentIndex++;
                supportIndex++;
            }

            return activeRows.Count > 0;
        }

        public void Release()
        {
            _index = null;
            _rootRows.Clear();
            foreach (List<int> activeRows in _activeRowsByDepth)
                activeRows.Clear();
        }

        private void EnsureDepthCapacity(int depthCount)
        {
            if (_activeRowsByDepth.Length >= depthCount)
                return;

            int previousLength = _activeRowsByDepth.Length;
            Array.Resize(ref _activeRowsByDepth, depthCount);
            for (int depth = previousLength; depth < depthCount; depth++)
                _activeRowsByDepth[depth] = new List<int>();
        }
    }

    private sealed class PositiveRelationConstraintSet
    {
        private int[] _partitionOffsets = Array.Empty<int>();
        private readonly List<PositiveRelationCursor> _cursors = new();
        private int _count;

        public void Reset(int[] partitionOffsets)
        {
            Clear();
            _partitionOffsets = partitionOffsets;
        }

        public bool Add(
            PositiveRelationIndex index,
            List<int>[] validIndices,
            bool[][] candidateMembership)
        {
            if (_count == _cursors.Count)
                _cursors.Add(new PositiveRelationCursor());

            PositiveRelationCursor cursor = _cursors[_count];
            if (!cursor.Reset(index, validIndices, candidateMembership))
            {
                cursor.Release();
                return false;
            }

            _count++;
            return true;
        }

        public bool TryExtend(
            ReadOnlySpan<int> clique,
            ReadOnlySpan<int> assignedPartitions)
        {
            int depth = assignedPartitions.Length - 1;
            int variableIndex = assignedPartitions[depth];
            int localValue = clique[variableIndex] - _partitionOffsets[variableIndex];

            for (int relationIndex = 0; relationIndex < _count; relationIndex++)
            {
                if (!_cursors[relationIndex].TryExtend(
                        assignedPartitions,
                        variableIndex,
                        localValue))
                {
                    return false;
                }
            }

            return true;
        }

        public void Clear()
        {
            for (int relationIndex = 0; relationIndex < _count; relationIndex++)
                _cursors[relationIndex].Release();

            _count = 0;
            _partitionOffsets = Array.Empty<int>();
        }
    }

    private sealed class EvaluationWorkspace
    {
        private List<int>[] _validIndices = Array.Empty<List<int>>();
        private bool[][] _candidateMembership = Array.Empty<bool[]>();
        private bool _candidateMembershipRequired;
        private int[][] _graphPartitions = Array.Empty<int[]>();
        private int[] _graphPartitionCounts = Array.Empty<int>();
        private Constant[] _binding = Array.Empty<Constant>();

        public List<int>[] ValidIndices => _validIndices;
        public bool[][]? CandidateMembership =>
            _candidateMembershipRequired ? _candidateMembership : null;
        public int[][] GraphPartitions => _graphPartitions;
        public int[] GraphPartitionCounts => _graphPartitionCounts;
        public Constant[] Binding => _binding;
        public PositiveRelationConstraintSet PositiveRelations { get; } = new();
        public DynamicCompatibilityProvider CompatibilityProvider { get; } = new();
        public KPartiteKClique.SearchWorkspace CliqueSearch { get; } = new();

        public void Prepare(CompiledConjunctiveConditionData compiledCondition)
        {
            int variableCount = compiledCondition.VariableCount;
            if (_binding.Length < variableCount)
                _binding = new Constant[variableCount];

            _candidateMembershipRequired =
                compiledCondition.PositiveStaticRelations.Count
                + compiledCondition.PositiveFluentHigherArityLiterals.Count > 0;
            if (_validIndices.Length < variableCount)
            {
                int previousLength = _validIndices.Length;
                Array.Resize(ref _validIndices, variableCount);
                Array.Resize(ref _graphPartitions, variableCount);
                Array.Resize(ref _graphPartitionCounts, variableCount);
                for (int variableIndex = previousLength;
                     variableIndex < variableCount;
                     variableIndex++)
                {
                    _validIndices[variableIndex] = new List<int>();
                    _graphPartitions[variableIndex] = Array.Empty<int>();
                }
            }

            if (_candidateMembershipRequired
                && _candidateMembership.Length < variableCount)
            {
                int previousLength = _candidateMembership.Length;
                Array.Resize(ref _candidateMembership, variableCount);
                for (int variableIndex = previousLength;
                     variableIndex < variableCount;
                     variableIndex++)
                {
                    _candidateMembership[variableIndex] = Array.Empty<bool>();
                }
            }

            for (int variableIndex = 0;
                 variableIndex < variableCount;
                 variableIndex++)
            {
                _validIndices[variableIndex].Clear();
                _graphPartitionCounts[variableIndex] = 0;
                if (_candidateMembershipRequired
                    && _candidateMembership[variableIndex].Length
                    < compiledCondition.StaticCandidateDomains[variableIndex].Length)
                {
                    _candidateMembership[variableIndex] =
                        new bool[compiledCondition.StaticCandidateDomains[variableIndex].Length];
                }
                if (_graphPartitions[variableIndex].Length
                    < compiledCondition.StaticCandidateDomains[variableIndex].Length)
                {
                    _graphPartitions[variableIndex] =
                        new int[compiledCondition.StaticCandidateDomains[variableIndex].Length];
                }
            }

            PositiveRelations.Reset(compiledCondition.CandidateDomainOffsets);
            CompatibilityProvider.Reset(compiledCondition.CandidateDomainOffsets, variableCount);
        }

        public void Release(int variableCount)
        {
            Array.Clear(_binding, 0, variableCount);
            if (_candidateMembershipRequired)
            {
                for (int variableIndex = 0;
                     variableIndex < variableCount;
                     variableIndex++)
                {
                    foreach (int localIndex in _validIndices[variableIndex])
                        _candidateMembership[variableIndex][localIndex] = false;
                }
            }

            for (int variableIndex = 0;
                 variableIndex < variableCount;
                 variableIndex++)
            {
                _validIndices[variableIndex].Clear();
                _graphPartitionCounts[variableIndex] = 0;
            }

            _candidateMembershipRequired = false;
            PositiveRelations.Clear();
            CompatibilityProvider.Release();
        }
    }

    private DynamicCompatibilityProvider? BuildDynamicCompatibilityProvider(
        CompiledConjunctiveConditionData compiledCondition,
        EvaluationWorkspace workspace,
        BindingEvaluationState state)
    {
        if (compiledCondition.FluentBinaryConstraintGroups.Length == 0)
            return null;

        int words = compiledCondition.StaticCompatibilityGraph.WordsPerVertex;
        List<int>[] validIndices = workspace.ValidIndices;
        DynamicCompatibilityProvider provider = workspace.CompatibilityProvider;

        foreach (FluentBinaryConstraintGroup constraintGroup
            in compiledCondition.FluentBinaryConstraintGroups)
        {
            int leftVariableIndex = constraintGroup.LeftVariableIndex;
            int rightVariableIndex = constraintGroup.RightVariableIndex;

            CompatibilityMasks forwardMasks = provider.PrepareMasks(
                leftVariableIndex,
                rightVariableIndex,
                compiledCondition.StaticCandidateDomains[leftVariableIndex].Length,
                words);
            CompatibilityMasks reverseMasks = provider.PrepareMasks(
                rightVariableIndex,
                leftVariableIndex,
                compiledCondition.StaticCandidateDomains[rightVariableIndex].Length,
                words);

            foreach (int leftLocalIndex in validIndices[leftVariableIndex])
                forwardMasks.GetMutableRow(leftLocalIndex).Clear();
            foreach (int rightLocalIndex in validIndices[rightVariableIndex])
                reverseMasks.GetMutableRow(rightLocalIndex).Clear();

            foreach (int leftLocalIndex in validIndices[leftVariableIndex])
            {
                Span<ulong> forwardMask = forwardMasks.GetMutableRow(leftLocalIndex);

                foreach (int rightLocalIndex in validIndices[rightVariableIndex])
                {
                    if (!AreBinaryConstraintsSatisfied(
                        constraintGroup.Constraints,
                        leftLocalIndex,
                        rightLocalIndex,
                        state))
                    {
                        continue;
                    }

                    int rightVertex =
                        compiledCondition.CandidateDomainOffsets[rightVariableIndex] + rightLocalIndex;
                    forwardMask[rightVertex / 64] |= 1UL << (rightVertex % 64);

                    int leftVertex =
                        compiledCondition.CandidateDomainOffsets[leftVariableIndex] + leftLocalIndex;
                    Span<ulong> reverseMask = reverseMasks.GetMutableRow(rightLocalIndex);
                    reverseMask[leftVertex / 64] |= 1UL << (leftVertex % 64);
                }
            }
        }

        return provider;
    }

    private static (int, int) NormalizePair(int left, int right)
        => left < right ? (left, right) : (right, left);
}
