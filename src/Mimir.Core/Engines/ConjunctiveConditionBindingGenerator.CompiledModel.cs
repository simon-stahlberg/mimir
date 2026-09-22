using Mimir.Core.Algorithms.Graph;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public sealed partial class ConjunctiveConditionBindingGenerator
{
    internal sealed class CompiledConjunctiveConditionData
    {
        internal int VariableCount { get; init; }
        internal Problem Problem { get; init; } = null!;
        internal CompiledNumericComparison[] DeferredNumericConditions { get; set; } = Array.Empty<CompiledNumericComparison>();
        internal int PositiveFluentCount { get; set; }
        internal bool StaticNullaryValid { get; set; }
        internal bool HasDeferredDerivedBinaryConstraints { get; set; }
        internal bool HasDerivedConstraints { get; set; }
        internal int SynchronizedFluentFactCount { get; set; } = -1;

        internal Constant[][] StaticCandidateDomains { get; set; } = Array.Empty<Constant[]>();
        internal Dictionary<Constant, int>[] StaticCandidateIndices { get; set; } =
            Array.Empty<Dictionary<Constant, int>>();
        internal int[] CandidateDomainOffsets { get; set; } = Array.Empty<int>();
        internal int CandidateVertexCount { get; set; }
        internal BitGraph StaticCompatibilityGraph { get; set; } = new(0);

        internal List<StaticConstraint> StaticHigherArityConstraints { get; } = new();
        internal List<DynamicConstraint> StateDependentNullaryConstraints { get; } = new();
        internal List<IUnaryConstraint>[] StateDependentUnaryConstraints { get; set; } =
            Array.Empty<List<IUnaryConstraint>>();
        internal FluentBinaryConstraintGroup[] FluentBinaryConstraintGroups { get; set; } =
            Array.Empty<FluentBinaryConstraintGroup>();
        internal List<DerivedBinaryConstraint>?[,]? DeferredDerivedBinaryConstraints { get; set; }
        internal List<DynamicConstraint> StateDependentHigherArityConstraints { get; } = new();
        internal List<PositiveRelationIndex> PositiveStaticRelations { get; } = new();
        internal List<CompiledConditionLiteral> PositiveFluentHigherArityLiterals { get; } = new();
    }

    internal readonly record struct FluentBinaryConstraintGroup(
        int LeftVariableIndex,
        int RightVariableIndex,
        IBinaryConstraint[] Constraints);

    internal enum PredicateEvaluationType
    {
        Static,
        StaticDerived,
        Fluent,
        Derived,
    }

    internal readonly struct CompiledConditionLiteral
    {
        private readonly Problem _problem;
        private readonly Predicate _predicate;
        private readonly int[] _variableSlots;
        private readonly Constant?[] _constantArguments;

        public PredicateEvaluationType EvaluationType { get; }
        public Polarity Polarity { get; }
        public int PositiveFluentOrdinal { get; }
        public int[] DistinctVariableIndices { get; }

        public CompiledConditionLiteral(
            Problem problem,
            PredicateEvaluationType evaluationType,
            Predicate predicate,
            IReadOnlyList<ITerm> arguments,
            Polarity polarity,
            IReadOnlyDictionary<Variable, int> variableIndices,
            int positiveFluentOrdinal)
        {
            _problem = problem;
            _predicate = predicate;
            EvaluationType = evaluationType;
            Polarity = polarity;
            PositiveFluentOrdinal = positiveFluentOrdinal;
            _variableSlots = new int[arguments.Count];
            _constantArguments = new Constant?[arguments.Count];

            var distinct = new List<int>();
            for (int i = 0; i < arguments.Count; i++)
            {
                _variableSlots[i] = -1;
                if (arguments[i] is Variable variable)
                {
                    if (!variableIndices.TryGetValue(variable, out int variableIndex))
                        throw new ArgumentException(
                            $"Condition references variable '{variable.Name}' that is not present in the compiled variable list.",
                            nameof(variableIndices));

                    _variableSlots[i] = variableIndex;
                    if (!distinct.Contains(variableIndex))
                        distinct.Add(variableIndex);
                }
                else
                {
                    _constantArguments[i] = (Constant)arguments[i];
                }
            }

            DistinctVariableIndices = distinct.ToArray();
        }

        public bool Evaluate(Constant[] binding, BindingEvaluationState? state)
        {
            if (EvaluationType is PredicateEvaluationType.StaticDerived or PredicateEvaluationType.Derived)
            {
                IReadOnlyList<Constant> arguments = ResolveArguments(binding);
                var predicate = (Mimir.Core.Schemas.Predicate<Derived>)_predicate;
                bool derivedIsTrue = EvaluationType == PredicateEvaluationType.StaticDerived
                    ? _problem.Context.IsStaticDerivedTruePrevalidated(
                        predicate,
                        arguments)
                    : state?.IsStateDependentDerivedTruePrevalidated(
                        predicate,
                        arguments)
                        ?? throw new InvalidOperationException(
                            "Derived conditions require an extended state.");
                return Polarity == Polarity.Positive ? derivedIsTrue : !derivedIsTrue;
            }

            FactIndex? factIndex = ResolveFactIndex(binding);
            if (factIndex is null
                && EvaluationType == PredicateEvaluationType.Static
                && _problem.Domain.IsEqualityPredicate(_predicate))
            {
                RegisterFact(binding);
                factIndex = ResolveFactIndex(binding)
                    ?? throw new InvalidOperationException(
                        "A required fact was not registered for the current binding.");
            }

            bool isTrue = false;

            if (factIndex != null)
            {
                Fact fact = _problem.Context.GetFact(factIndex.Value);
                switch (EvaluationType)
                {
                    case PredicateEvaluationType.Static:
                    {
                        var staticFact = (Fact<Static>)fact;
                        int word = staticFact.LocalIndex / 64;
                        int bit = staticFact.LocalIndex % 64;
                        if (word < _problem.Context.StaticBitboardWords.Length)
                            isTrue = (_problem.Context.StaticBitboardWords[word] & (1UL << bit)) != 0;
                        break;
                    }
                    case PredicateEvaluationType.Fluent:
                        if (state == null)
                            throw new InvalidOperationException("Fluent conditions require a state.");
                        isTrue = state.Value.IsFluentTrue(
                            new FluentIndex(((Fact<Fluent>)fact).LocalIndex),
                            PositiveFluentOrdinal);
                        break;
                }
            }

            return Polarity == Polarity.Positive ? isTrue : !isTrue;
        }

        private IReadOnlyList<Constant> ResolveArguments(Constant[] binding)
        {
            if (_variableSlots.Length == 0)
                return Array.Empty<Constant>();

            var arguments = new Constant[_variableSlots.Length];
            for (int i = 0; i < _variableSlots.Length; i++)
                arguments[i] = ResolveArgument(i, binding);
            return arguments;
        }

        public IReadOnlyList<Constant> GroundArguments(Constant[] binding)
            => ResolveArguments(binding);

        public IReadOnlyList<Constant> GroundArguments(
            int leftVariableIndex,
            Constant leftValue,
            int rightVariableIndex,
            Constant rightValue)
        {
            var arguments = new Constant[_variableSlots.Length];
            for (int i = 0; i < _variableSlots.Length; i++)
            {
                int variableIndex = _variableSlots[i];
                if (variableIndex == leftVariableIndex)
                {
                    arguments[i] = leftValue;
                    continue;
                }
                if (variableIndex == rightVariableIndex)
                {
                    arguments[i] = rightValue;
                    continue;
                }
                if (variableIndex == -1)
                {
                    arguments[i] = _constantArguments[i]!;
                    continue;
                }

                throw new InvalidOperationException(nameof(variableIndex));
            }

            return arguments;
        }

        public bool EvaluateStateDependentDerived(
            IReadOnlyList<Constant> arguments,
            BindingEvaluationState state)
        {
            if (EvaluationType != PredicateEvaluationType.Derived)
                throw new InvalidOperationException(nameof(EvaluationType));

            bool isTrue = state.IsStateDependentDerivedTruePrevalidated(
                (Mimir.Core.Schemas.Predicate<Derived>)_predicate,
                arguments);
            return Polarity == Polarity.Positive ? isTrue : !isTrue;
        }

        public FactIndex? ResolveFactIndex(Constant[] binding)
        {
            switch (_variableSlots.Length)
            {
                case 0:
                    return _problem.Context.GetFactIndex(_predicate);
                case 1:
                    return _problem.Context.GetFactIndex(_predicate, ResolveArgument(0, binding));
                case 2:
                    return _problem.Context.GetFactIndex(_predicate, ResolveArgument(0, binding), ResolveArgument(1, binding));
                case 3:
                    return _problem.Context.GetFactIndex(_predicate, ResolveArgument(0, binding), ResolveArgument(1, binding), ResolveArgument(2, binding));
                default:
                {
                    return _problem.Context.GetFactIndex(
                        _predicate,
                        ResolveArguments(binding));
                }
            }
        }

        public void RegisterFact(Constant[] binding)
        {
            if (_variableSlots.Length == 0)
            {
                switch (EvaluationType)
                {
                    case PredicateEvaluationType.Static:
                        _problem.Context.RegisterFact((Mimir.Core.Schemas.Predicate<Static>)_predicate, Array.Empty<Constant>());
                        break;
                    case PredicateEvaluationType.Fluent:
                        _problem.Context.RegisterFact((Mimir.Core.Schemas.Predicate<Fluent>)_predicate, Array.Empty<Constant>());
                        break;
                }

                return;
            }

            IReadOnlyList<Constant> arguments = ResolveArguments(binding);

            switch (EvaluationType)
            {
                case PredicateEvaluationType.Static:
                    _problem.Context.RegisterFact((Mimir.Core.Schemas.Predicate<Static>)_predicate, arguments);
                    break;
                case PredicateEvaluationType.Fluent:
                    _problem.Context.RegisterFact((Mimir.Core.Schemas.Predicate<Fluent>)_predicate, arguments);
                    break;
            }
        }

        public IReadOnlyList<Fact> GetRegisteredFacts()
            => _problem.Context.GetFacts(_predicate);

        public IReadOnlyList<Fact> GetCandidateFacts(
            BindingEvaluationState? state)
        {
            IReadOnlyList<Fact> registeredFacts = GetRegisteredFacts();
            if (EvaluationType != PredicateEvaluationType.Fluent
                || state is not BindingEvaluationState evaluationState)
            {
                return registeredFacts;
            }

            return evaluationState.GetCandidateFacts(
                (Mimir.Core.Schemas.Predicate<Fluent>)_predicate,
                PositiveFluentOrdinal,
                registeredFacts);
        }

        public bool IsTrue(Fact fact, BindingEvaluationState? state)
        {
            return EvaluationType switch
            {
                PredicateEvaluationType.Static => _problem.Context.IsTrue(
                    new StaticIndex(((Fact<Static>)fact).LocalIndex)),
                PredicateEvaluationType.Fluent => state is BindingEvaluationState evaluationState
                    ? evaluationState.IsFluentTrue(
                        new FluentIndex(((Fact<Fluent>)fact).LocalIndex),
                        PositiveFluentOrdinal)
                    : throw new InvalidOperationException(
                        "Fluent relation evaluation requires a state."),
                _ => throw new InvalidOperationException(
                    $"Predicate evaluation type '{EvaluationType}' cannot be materialized as a sparse relation.")
            };
        }

        public bool TryProjectFact(
            Fact fact,
            IReadOnlyList<Dictionary<Constant, int>> partitionIndices,
            int[] projectedLocalValues)
        {
            Array.Fill(projectedLocalValues, -1);

            for (int argumentIndex = 0; argumentIndex < _variableSlots.Length; argumentIndex++)
            {
                Constant factArgument = fact.Arguments[argumentIndex];
                int variableIndex = _variableSlots[argumentIndex];
                if (variableIndex < 0)
                {
                    if (!ReferenceEquals(factArgument, _constantArguments[argumentIndex]))
                        return false;
                    continue;
                }

                int projectedIndex = Array.IndexOf(DistinctVariableIndices, variableIndex);
                if (!partitionIndices[variableIndex].TryGetValue(
                        factArgument,
                        out int localValue))
                {
                    return false;
                }

                if (projectedLocalValues[projectedIndex] >= 0)
                {
                    if (projectedLocalValues[projectedIndex] != localValue)
                        return false;
                    continue;
                }

                projectedLocalValues[projectedIndex] = localValue;
            }

            return true;
        }

        private Constant ResolveArgument(int index, Constant[] binding)
        {
            int variableIndex = _variableSlots[index];
            return variableIndex >= 0 ? binding[variableIndex] : _constantArguments[index]!;
        }
    }
}
