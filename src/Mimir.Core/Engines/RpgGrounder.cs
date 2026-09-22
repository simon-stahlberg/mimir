namespace Mimir.Core.Engines;

using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

public class RpgGrounder : IGrounder
{
    private sealed class CompiledAction
    {
        public required ActionGroundingPlan GroundingPlan { get; init; }
        public required CompiledConjunctiveCondition ReachabilityCondition { get; init; }
        public required IReadOnlyList<Predicate<Fluent>> PositiveFluentPredicates { get; init; }
        public required IReadOnlyList<Predicate<Fluent>> PositiveFluentDependencies { get; init; }
        public required ConjunctiveConditionBindingGenerator.FluentDeltaMode[] DeltaModes { get; init; }
    }

    private struct GroundingVisitor
    {
        private readonly Problem _problem;
        private readonly CompiledAction _compiledAction;
        private readonly State _relaxedState;
        private readonly List<GroundAction> _reachableActions;
        private readonly List<GroundConditionalEffect> _pendingConditionalEffects;
        private readonly List<Fact<Fluent>> _additionalFacts;
        private readonly HashSet<Fact<Fluent>> _additionalFactSet;
        private readonly HashSet<Predicate<Fluent>> _addedPredicates;

        public GroundingVisitor(
            Problem problem,
            CompiledAction compiledAction,
            State relaxedState,
            List<GroundAction> reachableActions,
            List<GroundConditionalEffect> pendingConditionalEffects,
            List<Fact<Fluent>> additionalFacts,
            HashSet<Fact<Fluent>> additionalFactSet,
            HashSet<Predicate<Fluent>> addedPredicates)
        {
            _problem = problem;
            _compiledAction = compiledAction;
            _relaxedState = relaxedState;
            _reachableActions = reachableActions;
            _pendingConditionalEffects = pendingConditionalEffects;
            _additionalFacts = additionalFacts;
            _additionalFactSet = additionalFactSet;
            _addedPredicates = addedPredicates;
        }

        public bool Visit(ReadOnlySpan<Constant> binding)
        {
            Constant[] arguments = binding.ToArray();
            GroundAction action = ActionBuilder.BuildActionWithOwnedArguments(
                _compiledAction.GroundingPlan,
                arguments,
                _problem);
            // Action costs are state-independent, so an undefined cost makes the action inapplicable everywhere.
            if (double.IsNaN(action.Cost))
                return true;
            _reachableActions.Add(action);

            foreach (int localIndex in BitboardOps.EnumerateSetBits(action.AddEffects))
            {
                AddFact(
                    _relaxedState,
                    _problem.Context.GetFact(new FluentIndex(localIndex)),
                    _additionalFactSet,
                    _additionalFacts,
                    _addedPredicates);
            }

            foreach (GroundConditionalEffect conditionalEffect in action.ConditionalEffects)
            {
                if (conditionalEffect.LiteralEffect is not { IsPositive: true } literal)
                    continue;

                if (!BitboardOps.StaticPreconditionHolds(
                        conditionalEffect.PositiveStaticConditions,
                        conditionalEffect.NegativeStaticConditions,
                        _problem.Context.StaticBitboardWords))
                {
                    continue;
                }

                if (!StaticNumericGuardsHold(conditionalEffect, _relaxedState))
                    continue;

                if (_relaxedState.ContainsAll(
                        conditionalEffect.PositiveFluentConditions))
                {
                    AddFact(
                        _relaxedState,
                        literal.Value,
                        _additionalFactSet,
                        _additionalFacts,
                        _addedPredicates);
                    continue;
                }

                _pendingConditionalEffects.Add(conditionalEffect);
            }

            return true;
        }
    }

    private static bool VisitBinding(
        ReadOnlySpan<Constant> binding,
        ref GroundingVisitor visitor)
        => visitor.Visit(binding);

    public IEnumerable<GroundAction> Ground(Problem problem, State startState)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(startState);
        if (!ReferenceEquals(startState.Context, problem.Context))
            throw new ArgumentException("Start state belongs to a different problem.", nameof(startState));

        problem.Context.ScavengeActionComponentCaches();
        var bindingGenerator = new ConjunctiveConditionBindingGenerator();
        CompiledAction[] compiledActions = CompileActions(problem, bindingGenerator);
        var reachableActions = new List<GroundAction>();
        var pendingConditionalEffects = new List<GroundConditionalEffect>();
        var additionalFacts = new List<Fact<Fluent>>();
        var additionalFactSet = new HashSet<Fact<Fluent>>(ReferenceEqualityComparer.Instance);
        var addedPredicates = new HashSet<Predicate<Fluent>>(ReferenceEqualityComparer.Instance);
        var delta = new ConjunctiveConditionBindingGenerator.FluentDelta();
        delta.Reset(startState.GetTrueFacts(), problem.Context.FluentCount);
        var actionsToEvaluate = new List<int>(compiledActions.Length);
        var scheduledActions = new bool[compiledActions.Length];
        var dependentsByPredicate =
            new Dictionary<Predicate<Fluent>, List<int>>(
                ReferenceEqualityComparer.Instance);
        for (int actionIndex = 0; actionIndex < compiledActions.Length; actionIndex++)
        {
            actionsToEvaluate.Add(actionIndex);
            foreach (Predicate<Fluent> predicate
                     in compiledActions[actionIndex].PositiveFluentDependencies)
            {
                if (!dependentsByPredicate.TryGetValue(
                        predicate,
                        out List<int>? dependents))
                {
                    dependents = new List<int>();
                    dependentsByPredicate.Add(predicate, dependents);
                }

                dependents.Add(actionIndex);
            }
        }

        State relaxedState = startState;

        while (true)
        {
            additionalFacts.Clear();
            additionalFactSet.Clear();
            addedPredicates.Clear();

            foreach (int actionIndex in actionsToEvaluate)
            {
                CompiledAction compiledAction = compiledActions[actionIndex];
                var visitor = new GroundingVisitor(
                    problem,
                    compiledAction,
                    relaxedState,
                    reachableActions,
                    pendingConditionalEffects,
                    additionalFacts,
                    additionalFactSet,
                    addedPredicates);

                if (compiledAction.PositiveFluentPredicates.Count == 0)
                {
                    bindingGenerator.EnumerateBindingsUsingFluentState(
                        compiledAction.ReachabilityCondition,
                        relaxedState,
                        ref visitor,
                        VisitBinding);
                    continue;
                }

                Array.Fill(
                    compiledAction.DeltaModes,
                    ConjunctiveConditionBindingGenerator.FluentDeltaMode.Any);
                for (int driverOrdinal = 0;
                     driverOrdinal < compiledAction.PositiveFluentPredicates.Count;
                     driverOrdinal++)
                {
                    Predicate<Fluent> driverPredicate =
                        compiledAction.PositiveFluentPredicates[driverOrdinal];
                    if (delta.HasFacts(driverPredicate))
                    {
                        compiledAction.DeltaModes[driverOrdinal] =
                            ConjunctiveConditionBindingGenerator.FluentDeltaMode.AddedThisLayer;
                        bindingGenerator.EnumerateBindingsUsingFluentDelta(
                            compiledAction.ReachabilityCondition,
                            relaxedState,
                            delta,
                            compiledAction.DeltaModes,
                            ref visitor,
                            VisitBinding);
                    }

                    compiledAction.DeltaModes[driverOrdinal] =
                        ConjunctiveConditionBindingGenerator.FluentDeltaMode.BeforeThisLayer;
                }
            }

            int retainedConditionalEffectCount = 0;
            int conditionalEffectCount = pendingConditionalEffects.Count;
            for (int i = 0; i < conditionalEffectCount; i++)
            {
                GroundConditionalEffect conditionalEffect = pendingConditionalEffects[i];
                if (!relaxedState.ContainsAll(conditionalEffect.PositiveFluentConditions))
                {
                    pendingConditionalEffects[retainedConditionalEffectCount++] = conditionalEffect;
                    continue;
                }

                // Negative fluent and derived guards are not monotonic in the accumulated
                // relaxed state, so they cannot soundly rule out the effect's reachability.
                AddFact(
                    relaxedState,
                    conditionalEffect.RequiredLiteralEffect.Value,
                    additionalFactSet,
                    additionalFacts,
                    addedPredicates);
            }

            if (retainedConditionalEffectCount < conditionalEffectCount)
            {
                pendingConditionalEffects.RemoveRange(
                    retainedConditionalEffectCount,
                    conditionalEffectCount - retainedConditionalEffectCount);
            }

            if (additionalFacts.Count == 0)
                break;

            relaxedState = relaxedState.WithAdditionalFluentFacts(additionalFacts);
            delta.Reset(additionalFacts, problem.Context.FluentCount);
            Array.Clear(scheduledActions);
            foreach (Predicate<Fluent> predicate in addedPredicates)
            {
                if (!dependentsByPredicate.TryGetValue(
                        predicate,
                        out List<int>? dependents))
                    continue;

                foreach (int actionIndex in dependents)
                    scheduledActions[actionIndex] = true;
            }

            actionsToEvaluate.Clear();
            for (int actionIndex = 0; actionIndex < compiledActions.Length; actionIndex++)
            {
                if (scheduledActions[actionIndex])
                    actionsToEvaluate.Add(actionIndex);
            }
        }

        return reachableActions;
    }

    private static bool StaticNumericGuardsHold(GroundConditionalEffect effect, State state)
    {
        foreach (GroundNumericComparison comparison in effect.NumericConditions)
        {
            if (!NumericEvaluation.DependsOnState(comparison, state.Context.Problem.Domain.ChangingFunctions)
                && !state.Holds(comparison))
                return false;
        }
        return true;
    }

    private static void AddFact(
        State relaxedState,
        Fact<Fluent> fact,
        HashSet<Fact<Fluent>> additionalFactSet,
        List<Fact<Fluent>> additionalFacts,
        HashSet<Predicate<Fluent>> addedPredicates)
    {
        if (relaxedState.IsTrueUnchecked(new FluentIndex(fact.LocalIndex))
            || !additionalFactSet.Add(fact))
        {
            return;
        }

        additionalFacts.Add(fact);
        addedPredicates.Add(fact.Predicate);
    }

    private static CompiledAction[] CompileActions(
        Problem problem,
        ConjunctiveConditionBindingGenerator bindingGenerator)
    {
        var compiledActions = new List<CompiledAction>(problem.Domain.Actions.Count);
        IEqualityComparer<Predicate<Fluent>> predicateComparer = ReferenceEqualityComparer.Instance;

        foreach (ActionSchema schema in problem.Domain.Actions)
        {
            Literal<Atom<Fluent>>[] positiveFluentPreconditions = schema.FluentPreconditions
                .Where(literal => literal.Polarity == Polarity.Positive)
                .ToArray();

            compiledActions.Add(new CompiledAction
            {
                GroundingPlan = problem.Context.GetActionGroundingPlan(schema),
                PositiveFluentPredicates = positiveFluentPreconditions
                    .Select(literal => literal.Value.Predicate)
                    .ToArray(),
                PositiveFluentDependencies = positiveFluentPreconditions
                    .Select(literal => literal.Value.Predicate)
                    .Distinct(predicateComparer)
                    .ToArray(),
                DeltaModes =
                    new ConjunctiveConditionBindingGenerator.FluentDeltaMode[
                        positiveFluentPreconditions.Length],
                ReachabilityCondition = bindingGenerator.Compile(
                    problem,
                    schema.Parameters,
                    positiveFluentPreconditions,
                    schema.StaticPreconditions,
                    Array.Empty<Literal<Atom<Derived>>>(),
                    schema.NumericPreconditions.Where(comparison =>
                        !NumericEvaluation.DependsOnState(comparison, problem.Domain.ChangingFunctions)).ToArray()),
            });
        }

        return compiledActions.ToArray();
    }
}
