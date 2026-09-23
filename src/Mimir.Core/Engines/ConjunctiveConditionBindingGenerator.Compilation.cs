using Mimir.Core.Algorithms.Graph;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public sealed partial class ConjunctiveConditionBindingGenerator
{
    private CompiledConjunctiveConditionData CompileConditionData(
        Problem problem,
        IReadOnlyList<Variable> variables,
        IReadOnlyDictionary<Variable, int> variableIndices,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        IReadOnlyList<NumericComparison> numericConditions)
    {
        int k = variables.Count;
        var compiledCondition = new CompiledConjunctiveConditionData
        {
            VariableCount = k,
            Context = problem.Context,
            StateDependentUnaryConstraints = new List<IUnaryConstraint>[k]
        };

        var staticNullary = new List<StaticConstraint>();
        var staticUnary = new List<StaticConstraint>[k];
        var staticBinary = new List<StaticConstraint>?[k, k];
        var pendingStateDependentUnaryConstraints = new List<CompiledConditionLiteral>[k];
        var pendingDynamicBinary = new List<CompiledConditionLiteral>?[k, k];
        var compiledLiterals = new List<CompiledConditionLiteral>();
        var positiveStaticHigherArityConstraints = new List<CompiledConditionLiteral>();

        for (int i = 0; i < k; i++)
        {
            staticUnary[i] = new List<StaticConstraint>();
            pendingStateDependentUnaryConstraints[i] = new List<CompiledConditionLiteral>();
            compiledCondition.StateDependentUnaryConstraints[i] = new List<IUnaryConstraint>();
        }

        void AddStaticBinary(int left, int right, StaticConstraint constraint)
        {
            if (staticBinary[left, right] == null)
                staticBinary[left, right] = new List<StaticConstraint>();
            staticBinary[left, right]!.Add(constraint);
        }

        void AddPendingDynamicBinary(int left, int right, CompiledConditionLiteral literal)
        {
            if (pendingDynamicBinary[left, right] == null)
                pendingDynamicBinary[left, right] = new List<CompiledConditionLiteral>();
            pendingDynamicBinary[left, right]!.Add(literal);
        }

        void AnalyzeCondition(
            PredicateEvaluationType evaluationType,
            Predicate predicate,
            IReadOnlyList<ITerm> args,
            Polarity polarity,
            int positiveFluentOrdinal)
        {
            var literal = new CompiledConditionLiteral(
                problem,
                evaluationType,
                predicate,
                args,
                polarity,
                variableIndices,
                positiveFluentOrdinal);
            compiledLiterals.Add(literal);

            if (evaluationType is PredicateEvaluationType.StaticDerived or PredicateEvaluationType.Derived)
                compiledCondition.HasDerivedConstraints = true;

            if (evaluationType is PredicateEvaluationType.Static or PredicateEvaluationType.StaticDerived)
            {
                StaticConstraint constraint = binding => literal.Evaluate(binding, state: null);
                switch (literal.DistinctVariableIndices.Length)
                {
                    case 0:
                        staticNullary.Add(constraint);
                        break;
                    case 1:
                        staticUnary[literal.DistinctVariableIndices[0]].Add(constraint);
                        break;
                    case 2:
                    {
                        var (left, right) = NormalizePair(literal.DistinctVariableIndices[0], literal.DistinctVariableIndices[1]);
                        AddStaticBinary(left, right, constraint);
                        break;
                    }
                    default:
                        if (evaluationType == PredicateEvaluationType.Static
                            && literal.Polarity == Polarity.Positive)
                            positiveStaticHigherArityConstraints.Add(literal);
                        else
                            compiledCondition.StaticHigherArityConstraints.Add(constraint);
                        break;
                }

                return;
            }

            switch (literal.DistinctVariableIndices.Length)
            {
                case 0:
                    compiledCondition.StateDependentNullaryConstraints.Add((binding, state) => literal.Evaluate(binding, state));
                    break;
                case 1:
                    pendingStateDependentUnaryConstraints[literal.DistinctVariableIndices[0]].Add(literal);
                    break;
                case 2:
                {
                    var (left, right) = NormalizePair(literal.DistinctVariableIndices[0], literal.DistinctVariableIndices[1]);
                    AddPendingDynamicBinary(left, right, literal);
                    break;
                }
                default:
                    if (evaluationType == PredicateEvaluationType.Fluent && literal.Polarity == Polarity.Positive)
                        compiledCondition.PositiveFluentHigherArityLiterals.Add(literal);
                    else
                        compiledCondition.StateDependentHigherArityConstraints.Add((binding, state) => literal.Evaluate(binding, state));
                    break;
            }
        }

        int positiveFluentOrdinal = 0;
        foreach (var literal in fluentConditions)
        {
            int literalOrdinal = literal.Polarity == Polarity.Positive
                ? positiveFluentOrdinal++
                : -1;
            AnalyzeCondition(
                PredicateEvaluationType.Fluent,
                literal.Value.Predicate,
                literal.Value.Arguments,
                literal.Polarity,
                literalOrdinal);
        }
        compiledCondition.PositiveFluentCount = positiveFluentOrdinal;
        foreach (var literal in staticConditions)
        {
            AnalyzeCondition(
                PredicateEvaluationType.Static,
                literal.Value.Predicate,
                literal.Value.Arguments,
                literal.Polarity,
                positiveFluentOrdinal: -1);
        }
        foreach (var literal in derivedConditions)
        {
            PredicateEvaluationType evaluationType = problem.Context.IsStaticDerivedPrevalidated(
                literal.Value.Predicate)
                ? PredicateEvaluationType.StaticDerived
                : PredicateEvaluationType.Derived;
            AnalyzeCondition(
                evaluationType,
                literal.Value.Predicate,
                literal.Value.Arguments,
                literal.Polarity,
                positiveFluentOrdinal: -1);
        }

        // Cheapest evaluation first: static comparisons over at most two variables prune candidate domains once;
        // changing comparisons over zero or one variable are pre-grounded per candidate and checked per state;
        // the rest are deferred until the clique binds all of their variables.
        List<CompiledNumericComparison>? pendingNumericUnary = null;
        List<CompiledNumericComparison>? deferredNumeric = null;
        foreach (NumericComparison comparison in numericConditions)
        {
            var compiled = new CompiledNumericComparison(comparison, variableIndices);
            bool changing = NumericEvaluation.DependsOnState(comparison, problem.Domain.ChangingFunctions);
            int[] indices = compiled.VariableIndices;
            if (!changing && indices.Length <= 2)
            {
                StaticConstraint constraint = values => compiled.Evaluate(problem.Context, values, null);
                if (indices.Length == 0) staticNullary.Add(constraint);
                if (indices.Length == 1) staticUnary[indices[0]].Add(constraint);
                if (indices.Length == 2) AddStaticBinary(indices[0], indices[1], constraint);
                continue;
            }
            if (indices.Length == 0)
            {
                GroundNumericComparison grounded = compiled.Ground(problem, Array.Empty<Constant>());
                compiledCondition.StateDependentNullaryConstraints.Add((_, state) => state.State.Holds(grounded));
                continue;
            }
            if (indices.Length == 1)
            {
                (pendingNumericUnary ??= new List<CompiledNumericComparison>()).Add(compiled);
                continue;
            }
            (deferredNumeric ??= new List<CompiledNumericComparison>()).Add(compiled);
        }
        compiledCondition.DeferredNumericConditions = deferredNumeric?.ToArray() ?? Array.Empty<CompiledNumericComparison>();

        var binding = new Constant[k];
        compiledCondition.StaticNullaryValid = AreStaticConstraintsSatisfied(staticNullary, binding);
        if (!compiledCondition.StaticNullaryValid)
            return compiledCondition;

        compiledCondition.StaticCandidateDomains = new Constant[k][];
        compiledCondition.StaticCandidateIndices = new Dictionary<Constant, int>[k];

        for (int i = 0; i < k; i++)
        {
            Variable variable = variables[i];
            Constant[] compatibleObjects =
                problem.Context.GetCompatibleObjects(variable.Type);
            Constant[] partition;
            if (staticUnary[i].Count == 0)
            {
                partition = compatibleObjects;
            }
            else
            {
                var validObjects = new List<Constant>();
                foreach (Constant candidate in compatibleObjects)
                {
                    binding[i] = candidate;
                    if (AreStaticConstraintsSatisfied(staticUnary[i], binding))
                        validObjects.Add(candidate);
                }

                binding[i] = null!;
                partition = validObjects.ToArray();
            }

            compiledCondition.StaticCandidateDomains[i] = partition;
            compiledCondition.StaticCandidateIndices[i] = new Dictionary<Constant, int>(
                partition.Length,
                ReferenceEqualityComparer.Instance);
            for (int localIndex = 0; localIndex < partition.Length; localIndex++)
                compiledCondition.StaticCandidateIndices[i].Add(partition[localIndex], localIndex);
            compiledCondition.CandidateVertexCount += partition.Length;
        }

        compiledCondition.CandidateDomainOffsets = new int[k];
        int currentOffset = 0;
        for (int i = 0; i < k; i++)
        {
            compiledCondition.CandidateDomainOffsets[i] = currentOffset;
            currentOffset += compiledCondition.StaticCandidateDomains[i].Length;
        }

        bool hasEmptyPartition = compiledCondition.StaticCandidateDomains.Any(partition => partition.Length == 0);
        if (k >= 2 && !hasEmptyPartition)
        {
            // Intentionally dense. Do not flag this based only on asymptotic memory usage.
            // It is a valid concern only when an alternative is demonstrated to perform
            // the same work with less memory and no additional runtime cost.
            compiledCondition.StaticCompatibilityGraph = new BitGraph(compiledCondition.CandidateVertexCount);
            for (int i = 0; i < k; i++)
            {
                for (int j = i + 1; j < k; j++)
                {
                    var constraints = staticBinary[i, j];

                    for (int leftLocalIndex = 0; leftLocalIndex < compiledCondition.StaticCandidateDomains[i].Length; leftLocalIndex++)
                    {
                        binding[i] = compiledCondition.StaticCandidateDomains[i][leftLocalIndex];
                        int leftVertex = compiledCondition.CandidateDomainOffsets[i] + leftLocalIndex;

                        for (int rightLocalIndex = 0; rightLocalIndex < compiledCondition.StaticCandidateDomains[j].Length; rightLocalIndex++)
                        {
                            binding[j] = compiledCondition.StaticCandidateDomains[j][rightLocalIndex];
                            int rightVertex = compiledCondition.CandidateDomainOffsets[j] + rightLocalIndex;

                            if (constraints == null || AreStaticConstraintsSatisfied(constraints, binding))
                                compiledCondition.StaticCompatibilityGraph.AddEdge(leftVertex, rightVertex);
                        }

                        binding[j] = null!;
                    }

                    binding[i] = null!;
                }
            }
        }

        PreRegisterProjectedConditionFacts(compiledCondition, compiledLiterals);
        FinalizeDynamicConstraintTables(problem, compiledCondition, pendingStateDependentUnaryConstraints, pendingDynamicBinary, binding);
        foreach (CompiledNumericComparison comparison in pendingNumericUnary ?? [])
        {
            int variableIndex = comparison.VariableIndices[0];
            Constant[] domain = compiledCondition.StaticCandidateDomains[variableIndex];
            var grounded = new GroundNumericComparison[domain.Length];
            for (int i = 0; i < domain.Length; i++)
            {
                binding[variableIndex] = domain[i];
                grounded[i] = comparison.Ground(problem, binding);
            }
            binding[variableIndex] = null!;
            compiledCondition.StateDependentUnaryConstraints[variableIndex].Add(new NumericUnaryConstraint(grounded));
        }
        foreach (CompiledConditionLiteral literal in positiveStaticHigherArityConstraints)
        {
            compiledCondition.PositiveStaticRelations.Add(BuildPositiveRelationIndex(
                literal,
                compiledCondition,
                state: null,
                candidateMembership: null));
        }

        return compiledCondition;
    }

    private static void PreRegisterProjectedConditionFacts(
        CompiledConjunctiveConditionData compiledCondition,
        IReadOnlyList<CompiledConditionLiteral> compiledLiterals)
    {
        if (!compiledCondition.StaticNullaryValid || compiledLiterals.Count == 0)
            return;

        var binding = new Constant[compiledCondition.VariableCount];
        foreach (CompiledConditionLiteral literal in compiledLiterals)
        {
            if (literal.EvaluationType != PredicateEvaluationType.Fluent
                || literal.DistinctVariableIndices.Length >= 2)
            {
                continue;
            }

            RegisterProjection(literal, projectionIndex: 0);
        }

        void RegisterProjection(CompiledConditionLiteral literal, int projectionIndex)
        {
            if (projectionIndex == literal.DistinctVariableIndices.Length)
            {
                literal.RegisterFact(binding);
                return;
            }

            int variableIndex = literal.DistinctVariableIndices[projectionIndex];
            foreach (Constant value in compiledCondition.StaticCandidateDomains[variableIndex])
            {
                binding[variableIndex] = value;
                RegisterProjection(literal, projectionIndex + 1);
            }

            binding[variableIndex] = null!;
        }
    }

    private static void FinalizeDynamicConstraintTables(
        Problem problem,
        CompiledConjunctiveConditionData compiledCondition,
        List<CompiledConditionLiteral>[] pendingStateDependentUnaryConstraints,
        List<CompiledConditionLiteral>?[,] pendingDynamicBinary,
        Constant[] binding)
    {
        if (compiledCondition.VariableCount == 0)
            return;

        var fluentBinaryConstraintGroups = new List<FluentBinaryConstraintGroup>();

        for (int i = 0; i < compiledCondition.VariableCount; i++)
        {
            foreach (var literal in pendingStateDependentUnaryConstraints[i])
            {
                compiledCondition.StateDependentUnaryConstraints[i].Add(BuildUnaryConstraint(
                    problem,
                    literal,
                    compiledCondition.StaticCandidateDomains[i],
                    i,
                    binding));
            }
        }

        for (int i = 0; i < compiledCondition.VariableCount; i++)
        {
            for (int j = i + 1; j < compiledCondition.VariableCount; j++)
            {
                var pendingChecks = pendingDynamicBinary[i, j];
                if (pendingChecks == null)
                    continue;

                List<IBinaryConstraint>? compiledConstraints = null;
                List<DerivedBinaryConstraint>? deferredDerivedConstraints = null;
                foreach (var literal in pendingChecks)
                {
                    if (literal.EvaluationType == PredicateEvaluationType.Fluent)
                    {
                        compiledConstraints ??= new List<IBinaryConstraint>();
                        compiledConstraints.Add(new FluentBinaryConstraint(
                            literal,
                            compiledCondition.StaticCandidateIndices,
                            i,
                            j,
                            compiledCondition.StaticCandidateDomains[i].Length,
                            compiledCondition.StaticCandidateDomains[j].Length));
                        continue;
                    }

                    if (literal.EvaluationType != PredicateEvaluationType.Derived)
                        throw new NotSupportedException(
                            $"Unsupported binary dynamic condition evaluation type: {literal.EvaluationType}");

                    compiledCondition.HasDeferredDerivedBinaryConstraints = true;
                    compiledCondition.DeferredDerivedBinaryConstraints ??=
                        new List<DerivedBinaryConstraint>?[compiledCondition.VariableCount, compiledCondition.VariableCount];
                    deferredDerivedConstraints ??= new List<DerivedBinaryConstraint>();
                    deferredDerivedConstraints.Add(new DerivedBinaryConstraint(
                        literal,
                        compiledCondition.StaticCandidateDomains[i],
                        compiledCondition.StaticCandidateDomains[j],
                        i,
                        j));
                }

                if (compiledConstraints is not null)
                {
                    fluentBinaryConstraintGroups.Add(new FluentBinaryConstraintGroup(
                        i,
                        j,
                        compiledConstraints.ToArray()));
                }
                if (deferredDerivedConstraints is not null)
                    compiledCondition.DeferredDerivedBinaryConstraints![i, j] = deferredDerivedConstraints;
            }
        }

        compiledCondition.FluentBinaryConstraintGroups = fluentBinaryConstraintGroups.ToArray();
    }

    private static IUnaryConstraint BuildUnaryConstraint(
        Problem problem,
        CompiledConditionLiteral literal,
        Constant[] partition,
        int variableIndex,
        Constant[] binding)
    {
        switch (literal.EvaluationType)
        {
            case PredicateEvaluationType.Fluent:
            {
                var factIds = new int[partition.Length];
                Array.Fill(factIds, -1);
                for (int localIndex = 0; localIndex < partition.Length; localIndex++)
                {
                    binding[variableIndex] = partition[localIndex];
                    FactIndex? factIndex = literal.ResolveFactIndex(binding);
                    if (factIndex != null)
                    {
                        factIds[localIndex] = ((Fact<Fluent>)problem.Context.GetFact(
                            factIndex.Value)).LocalIndex;
                    }
                }

                binding[variableIndex] = null!;
                return new FluentUnaryConstraint(
                    factIds,
                    literal.Polarity,
                    literal.PositiveFluentOrdinal);
            }
            case PredicateEvaluationType.Derived:
            {
                var arguments = new IReadOnlyList<Constant>[partition.Length];
                for (int localIndex = 0; localIndex < partition.Length; localIndex++)
                {
                    binding[variableIndex] = partition[localIndex];
                    arguments[localIndex] = literal.GroundArguments(binding);
                }

                binding[variableIndex] = null!;
                return new DerivedUnaryConstraint(literal, arguments);
            }
            default:
                throw new NotSupportedException(
                    $"Unsupported unary dynamic condition evaluation type: {literal.EvaluationType}");
        }
    }
}
