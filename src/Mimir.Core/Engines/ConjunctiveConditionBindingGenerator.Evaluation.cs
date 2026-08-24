using Mimir.Core.Algorithms.Graph;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public sealed partial class ConjunctiveConditionBindingGenerator
{
    private readonly Stack<EvaluationWorkspace> _evaluationWorkspaces = new();

    private int EnumerateBindingsCore<TCallbackState>(
        CompiledConjunctiveCondition condition,
        BindingEvaluationState state,
        int maxBindings,
        ref TCallbackState callbackState,
        BindingCallback<TCallbackState>? callback)
    {
        CompiledConjunctiveConditionData compiledCondition = condition.Data;
        if (maxBindings == 0 || !compiledCondition.StaticNullaryValid)
            return 0;

        int k = compiledCondition.VariableCount;
        EvaluationWorkspace? workspace = k == 0
            ? null
            : RentEvaluationWorkspace(compiledCondition);
        Constant[] binding = workspace?.Binding ?? Array.Empty<Constant>();
        var enumeration = new BindingEnumeration<TCallbackState>(
            compiledCondition,
            state,
            binding,
            maxBindings,
            callbackState,
            callback,
            representative: false);
        try
        {
            if (!AreDynamicConstraintsSatisfied(compiledCondition.StateDependentNullaryConstraints, binding, state))
                return 0;

            if (compiledCondition.FluentBinaryConstraintGroups.Length > 0)
            {
                SynchronizeDynamicConstraintTables(
                    compiledCondition,
                    condition.Problem.Context.FluentCount);
            }

            if (k == 0)
            {
                enumeration.Emit();
                return enumeration.Emitted;
            }

            if (k == 1)
            {
                for (int localIndex = 0;
                     localIndex < compiledCondition.StaticCandidateDomains[0].Length;
                     localIndex++)
                {
                    if (!AreUnaryConstraintsSatisfied(
                            compiledCondition.StateDependentUnaryConstraints[0],
                            localIndex,
                            state))
                    {
                        continue;
                    }

                    binding[0] = compiledCondition.StaticCandidateDomains[0][localIndex];
                    if (!AreStaticConstraintsSatisfied(
                            compiledCondition.StaticHigherArityConstraints,
                            binding))
                    {
                        continue;
                    }
                    if (!AreDynamicConstraintsSatisfied(
                            compiledCondition.StateDependentHigherArityConstraints,
                            binding,
                            state))
                    {
                        continue;
                    }
                    if (!enumeration.Emit())
                        break;
                }

                return enumeration.Emitted;
            }

            EvaluationWorkspace cliqueWorkspace = workspace!;
            PrepareCandidateDomains(compiledCondition, state, cliqueWorkspace);
            if (HasEmptyCandidateDomain(cliqueWorkspace.ValidIndices, k))
                return enumeration.Emitted;
            if (!TryBuildPositiveRelationConstraints(
                    compiledCondition,
                    state,
                    cliqueWorkspace,
                    out PositiveRelationConstraintSet? positiveRelations))
            {
                return enumeration.Emitted;
            }

            enumeration.PositiveRelations = positiveRelations;
            PopulateCliquePartitions(compiledCondition, cliqueWorkspace);
            DynamicCompatibilityProvider? compatibilityProvider =
                BuildDynamicCompatibilityProvider(compiledCondition, cliqueWorkspace, state);
            KPartiteKClique.PartialCliqueConstraint<
                BindingEnumeration<TCallbackState>>? partialConstraint =
                compiledCondition.HasDeferredDerivedBinaryConstraints || positiveRelations is not null
                    ? TryExtendClique<TCallbackState>
                    : null;

            KPartiteKClique.VisitCliquesTrusted(
                compiledCondition.StaticCompatibilityGraph,
                cliqueWorkspace.GraphPartitions,
                cliqueWorkspace.GraphPartitionCounts,
                k,
                cliqueWorkspace.CliqueSearch,
                compatibilityProvider,
                partialConstraint,
                ref enumeration,
                VisitClique<TCallbackState>);

            return enumeration.Emitted;
        }
        finally
        {
            callbackState = enumeration.CallbackState;
            if (workspace is not null)
                ReturnEvaluationWorkspace(workspace, k);
        }
    }

    private int EnumerateRepresentativeBindingsCore<TCallbackState>(
        CompiledConjunctiveCondition condition,
        BindingEvaluationState state,
        bool[] retainedVariables,
        ref TCallbackState callbackState,
        BindingCallback<TCallbackState> callback)
    {
        CompiledConjunctiveConditionData compiledCondition = condition.Data;
        if (!compiledCondition.StaticNullaryValid)
            return 0;

        int k = compiledCondition.VariableCount;
        EvaluationWorkspace? workspace = k == 0
            ? null
            : RentEvaluationWorkspace(compiledCondition);
        Constant[] binding = workspace?.Binding ?? Array.Empty<Constant>();
        var enumeration = new BindingEnumeration<TCallbackState>(
            compiledCondition,
            state,
            binding,
            int.MaxValue,
            callbackState,
            callback,
            representative: true);
        try
        {
            if (!AreDynamicConstraintsSatisfied(compiledCondition.StateDependentNullaryConstraints, binding, state))
                return 0;
            if (compiledCondition.FluentBinaryConstraintGroups.Length > 0)
            {
                SynchronizeDynamicConstraintTables(
                    compiledCondition,
                    condition.Problem.Context.FluentCount);
            }

            if (k == 0)
            {
                enumeration.Emit();
                return enumeration.Emitted;
            }

            if (k == 1)
            {
                for (int localIndex = 0;
                     localIndex < compiledCondition.StaticCandidateDomains[0].Length;
                     localIndex++)
                {
                    if (!AreUnaryConstraintsSatisfied(
                            compiledCondition.StateDependentUnaryConstraints[0],
                            localIndex,
                            state))
                    {
                        continue;
                    }

                    binding[0] = compiledCondition.StaticCandidateDomains[0][localIndex];
                    if (!AreStaticConstraintsSatisfied(
                            compiledCondition.StaticHigherArityConstraints,
                            binding))
                    {
                        continue;
                    }
                    if (!AreDynamicConstraintsSatisfied(
                            compiledCondition.StateDependentHigherArityConstraints,
                            binding,
                            state))
                    {
                        continue;
                    }

                    enumeration.Emit();
                    break;
                }

                return enumeration.Emitted;
            }

            EvaluationWorkspace cliqueWorkspace = workspace!;
            PrepareCandidateDomains(compiledCondition, state, cliqueWorkspace);
            if (HasEmptyCandidateDomain(cliqueWorkspace.ValidIndices, k))
                return enumeration.Emitted;
            if (!TryBuildPositiveRelationConstraints(
                    compiledCondition,
                    state,
                    cliqueWorkspace,
                    out PositiveRelationConstraintSet? positiveRelations))
            {
                return enumeration.Emitted;
            }

            enumeration.PositiveRelations = positiveRelations;
            PopulateCliquePartitions(compiledCondition, cliqueWorkspace);
            KPartiteKClique.ICompatibilityProvider? compatibilityProvider =
                BuildDynamicCompatibilityProvider(compiledCondition, cliqueWorkspace, state);
            KPartiteKClique.VisitRepresentativeCliquesTrusted(
                compiledCondition.StaticCompatibilityGraph,
                cliqueWorkspace.GraphPartitions,
                cliqueWorkspace.GraphPartitionCounts,
                k,
                cliqueWorkspace.CliqueSearch,
                compatibilityProvider,
                TryExtendClique<TCallbackState>,
                retainedVariables,
                ref enumeration,
                VisitClique<TCallbackState>);

            return enumeration.Emitted;
        }
        finally
        {
            callbackState = enumeration.CallbackState;
            if (workspace is not null)
                ReturnEvaluationWorkspace(workspace, k);
        }
    }

    private struct BindingEnumeration<TCallbackState>
    {
        public CompiledConjunctiveConditionData Data { get; }
        public BindingEvaluationState State { get; }
        public Constant[] Binding { get; }
        public int MaxBindings { get; }
        public BindingCallback<TCallbackState>? Callback { get; }
        public bool Representative { get; }
        public TCallbackState CallbackState;
        public int Emitted;
        public PositiveRelationConstraintSet? PositiveRelations;

        public BindingEnumeration(
            CompiledConjunctiveConditionData compiledCondition,
            BindingEvaluationState state,
            Constant[] binding,
            int maxBindings,
            TCallbackState callbackState,
            BindingCallback<TCallbackState>? callback,
            bool representative)
        {
            Data = compiledCondition;
            State = state;
            Binding = binding;
            MaxBindings = maxBindings;
            CallbackState = callbackState;
            Callback = callback;
            Representative = representative;
            Emitted = 0;
            PositiveRelations = null;
        }

        public bool Emit()
        {
            Emitted++;
            if (Callback is not null
                && !Callback(
                    Binding.AsSpan(0, Data.VariableCount),
                    ref CallbackState))
            {
                return false;
            }

            return Emitted < MaxBindings;
        }
    }

    private static bool VisitClique<TCallbackState>(
        ReadOnlySpan<int> clique,
        ref BindingEnumeration<TCallbackState> enumeration)
    {
        if (enumeration.Representative)
            return enumeration.Emit();

        PopulateBinding(enumeration.Data, clique, enumeration.Binding);
        if (!AreStaticConstraintsSatisfied(
                enumeration.Data.StaticHigherArityConstraints,
                enumeration.Binding))
        {
            return true;
        }
        if (!AreDynamicConstraintsSatisfied(
                enumeration.Data.StateDependentHigherArityConstraints,
                enumeration.Binding,
                enumeration.State))
        {
            return true;
        }

        return enumeration.Emit();
    }

    private static bool TryExtendClique<TCallbackState>(
        ReadOnlySpan<int> clique,
        ReadOnlySpan<int> assignedPartitions,
        ref BindingEnumeration<TCallbackState> enumeration)
    {
        if (enumeration.PositiveRelations is not null
            && !enumeration.PositiveRelations.TryExtend(
                clique,
                assignedPartitions))
        {
            return false;
        }

        if (enumeration.Data.HasDeferredDerivedBinaryConstraints
            && !AreDeferredDerivedConstraintsSatisfied(
                enumeration.Data,
                clique,
                assignedPartitions,
                enumeration.State))
        {
            return false;
        }

        if (!enumeration.Representative
            || assignedPartitions.Length < enumeration.Data.VariableCount)
        {
            return true;
        }

        PopulateBinding(enumeration.Data, clique, enumeration.Binding);
        return AreStaticConstraintsSatisfied(
                enumeration.Data.StaticHigherArityConstraints,
                enumeration.Binding)
            && AreDynamicConstraintsSatisfied(
                enumeration.Data.StateDependentHigherArityConstraints,
                enumeration.Binding,
                enumeration.State);
    }

    private static void PopulateBinding(
        CompiledConjunctiveConditionData compiledCondition,
        ReadOnlySpan<int> clique,
        Constant[] binding)
    {
        for (int variableIndex = 0;
             variableIndex < compiledCondition.VariableCount;
             variableIndex++)
        {
            int localIndex =
                clique[variableIndex] - compiledCondition.CandidateDomainOffsets[variableIndex];
            binding[variableIndex] =
                compiledCondition.StaticCandidateDomains[variableIndex][localIndex];
        }
    }

    private static void PrepareCandidateDomains(
        CompiledConjunctiveConditionData compiledCondition,
        BindingEvaluationState state,
        EvaluationWorkspace workspace)
    {
        List<int>[] validIndices = workspace.ValidIndices;
        for (int variableIndex = 0;
             variableIndex < compiledCondition.VariableCount;
             variableIndex++)
        {
            List<int> validDomainIndices = validIndices[variableIndex];
            for (int localIndex = 0;
                 localIndex < compiledCondition.StaticCandidateDomains[variableIndex].Length;
                 localIndex++)
            {
                if (!AreUnaryConstraintsSatisfied(
                    compiledCondition.StateDependentUnaryConstraints[variableIndex],
                    localIndex,
                    state))
                {
                    continue;
                }

                validDomainIndices.Add(localIndex);
            }
        }

        bool[][]? candidateMembership = workspace.CandidateMembership;
        if (candidateMembership is null)
            return;

        // Fusing these passes adds a branch to every candidate test and measurably
        // regresses lifted searches, so keep the flat post-filter pass.
        for (int variableIndex = 0;
             variableIndex < compiledCondition.VariableCount;
             variableIndex++)
        {
            foreach (int localIndex in validIndices[variableIndex])
                candidateMembership[variableIndex][localIndex] = true;
        }
    }

    private static bool HasEmptyCandidateDomain(
        List<int>[] validIndices,
        int variableCount)
    {
        for (int variableIndex = 0; variableIndex < variableCount; variableIndex++)
        {
            if (validIndices[variableIndex].Count == 0)
                return true;
        }

        return false;
    }

    private static void PopulateCliquePartitions(
        CompiledConjunctiveConditionData compiledCondition,
        EvaluationWorkspace workspace)
    {
        List<int>[] validIndices = workspace.ValidIndices;
        int[][] graphPartitions = workspace.GraphPartitions;
        int[] graphPartitionCounts = workspace.GraphPartitionCounts;
        for (int variableIndex = 0;
             variableIndex < compiledCondition.VariableCount;
             variableIndex++)
        {
            graphPartitionCounts[variableIndex] = validIndices[variableIndex].Count;
            for (int candidateIndex = 0;
                 candidateIndex < validIndices[variableIndex].Count;
                 candidateIndex++)
            {
                graphPartitions[variableIndex][candidateIndex] =
                    compiledCondition.CandidateDomainOffsets[variableIndex]
                    + validIndices[variableIndex][candidateIndex];
            }
        }
    }

    private static bool AreDeferredDerivedConstraintsSatisfied(
        CompiledConjunctiveConditionData compiledCondition,
        ReadOnlySpan<int> clique,
        ReadOnlySpan<int> assignedPartitions,
        BindingEvaluationState state)
    {
        if (assignedPartitions.Length < 2)
            return true;

        int newestPartition = assignedPartitions[^1];
        for (int assignedIndex = 0; assignedIndex < assignedPartitions.Length - 1; assignedIndex++)
        {
            int otherPartition = assignedPartitions[assignedIndex];
            (int leftPartition, int rightPartition) = NormalizePair(
                newestPartition,
                otherPartition);
            List<DerivedBinaryConstraint>? constraints =
                compiledCondition.DeferredDerivedBinaryConstraints![leftPartition, rightPartition];
            if (constraints is null)
                continue;

            int leftLocalIndex = clique[leftPartition] - compiledCondition.CandidateDomainOffsets[leftPartition];
            int rightLocalIndex = clique[rightPartition] - compiledCondition.CandidateDomainOffsets[rightPartition];
            foreach (DerivedBinaryConstraint constraint in constraints)
            {
                if (!constraint.Evaluate(leftLocalIndex, rightLocalIndex, state))
                    return false;
            }
        }

        return true;
    }

    private static bool TryBuildPositiveRelationConstraints(
        CompiledConjunctiveConditionData compiledCondition,
        BindingEvaluationState state,
        EvaluationWorkspace workspace,
        out PositiveRelationConstraintSet? constraintSet)
    {
        int relationCount =
            compiledCondition.PositiveStaticRelations.Count + compiledCondition.PositiveFluentHigherArityLiterals.Count;
        if (relationCount == 0)
        {
            constraintSet = null;
            return true;
        }

        List<int>[] validIndices = workspace.ValidIndices;
        bool[][] candidateMembership = workspace.CandidateMembership
            ?? throw new InvalidOperationException(
                "Positive relation candidate membership was not prepared.");
        PositiveRelationConstraintSet relations = workspace.PositiveRelations;
        foreach (PositiveRelationIndex index in compiledCondition.PositiveStaticRelations)
        {
            if (!relations.Add(index, validIndices, candidateMembership))
            {
                constraintSet = null;
                return false;
            }
        }

        foreach (CompiledConditionLiteral literal in compiledCondition.PositiveFluentHigherArityLiterals)
        {
            PositiveRelationIndex index = BuildPositiveRelationIndex(
                literal,
                compiledCondition,
                state,
                candidateMembership);
            if (!relations.Add(index, validIndices, candidateMembership))
            {
                constraintSet = null;
                return false;
            }
        }

        constraintSet = relations;
        return true;
    }

    private static PositiveRelationIndex BuildPositiveRelationIndex(
        CompiledConditionLiteral literal,
        CompiledConjunctiveConditionData compiledCondition,
        BindingEvaluationState? state,
        bool[][]? candidateMembership)
    {
        var rows = new List<int[]>();
        var projectedLocalValues = new int[literal.DistinctVariableIndices.Length];
        foreach (Fact fact in literal.GetCandidateFacts(state))
        {
            if (!literal.IsTrue(fact, state)
                || !literal.TryProjectFact(
                    fact,
                    compiledCondition.StaticCandidateIndices,
                    projectedLocalValues))
            {
                continue;
            }

            if (candidateMembership is not null
                && !AreProjectedValuesValid(
                    literal.DistinctVariableIndices,
                    projectedLocalValues,
                    candidateMembership))
            {
                continue;
            }

            rows.Add(projectedLocalValues.ToArray());
        }

        return new PositiveRelationIndex(
            compiledCondition.VariableCount,
            literal.DistinctVariableIndices,
            rows.ToArray(),
            compiledCondition.StaticCandidateDomains);
    }

    private static bool AreProjectedValuesValid(
        int[] variableIndices,
        int[] projectedLocalValues,
        bool[][] candidateMembership)
    {
        for (int column = 0; column < variableIndices.Length; column++)
        {
            if (!candidateMembership[variableIndices[column]][projectedLocalValues[column]])
            {
                return false;
            }
        }

        return true;
    }

    private static bool AreStaticConstraintsSatisfied(List<StaticConstraint> constraints, Constant[] binding)
    {
        for (int i = 0; i < constraints.Count; i++)
        {
            if (!constraints[i](binding))
                return false;
        }

        return true;
    }

    private static bool AreDynamicConstraintsSatisfied(List<DynamicConstraint> constraints, Constant[] binding, BindingEvaluationState state)
    {
        for (int i = 0; i < constraints.Count; i++)
        {
            if (!constraints[i](binding, state))
                return false;
        }

        return true;
    }

    private static bool AreUnaryConstraintsSatisfied(List<IUnaryConstraint> constraints, int localIndex, BindingEvaluationState state)
    {
        for (int i = 0; i < constraints.Count; i++)
        {
            if (!constraints[i].Evaluate(localIndex, state))
                return false;
        }

        return true;
    }

    private static void SynchronizeDynamicConstraintTables(
        CompiledConjunctiveConditionData compiledCondition,
        int fluentCount)
    {
        if (compiledCondition.SynchronizedFluentFactCount == fluentCount)
            return;

        foreach (FluentBinaryConstraintGroup constraintGroup
            in compiledCondition.FluentBinaryConstraintGroups)
        {
            foreach (IBinaryConstraint constraint in constraintGroup.Constraints)
                constraint.Synchronize();
        }

        compiledCondition.SynchronizedFluentFactCount = fluentCount;
    }

    private static bool AreBinaryConstraintsSatisfied(
        IBinaryConstraint[] constraints,
        int leftLocalIndex,
        int rightLocalIndex,
        BindingEvaluationState state)
    {
        for (int i = 0; i < constraints.Length; i++)
        {
            if (!constraints[i].Evaluate(leftLocalIndex, rightLocalIndex, state))
                return false;
        }

        return true;
    }

    private EvaluationWorkspace RentEvaluationWorkspace(
        CompiledConjunctiveConditionData compiledCondition)
    {
        EvaluationWorkspace workspace = _evaluationWorkspaces.Count == 0
            ? new EvaluationWorkspace()
            : _evaluationWorkspaces.Pop();
        workspace.Prepare(compiledCondition);
        return workspace;
    }

    private void ReturnEvaluationWorkspace(
        EvaluationWorkspace workspace,
        int variableCount)
    {
        workspace.Release(variableCount);
        _evaluationWorkspaces.Push(workspace);
    }

}
