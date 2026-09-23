using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using System.Runtime.CompilerServices;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Heuristics;

public sealed class LiftedFfHeuristic : IHeuristic
{
    private sealed class CompiledGoalData
    {
        public GoalCondition Goal { get; }
        public Fact<Fluent>[] GoalFacts { get; }

        public CompiledGoalData(GoalCondition goal, Fact<Fluent>[] goalFacts)
        {
            Goal = goal;
            GoalFacts = goalFacts;
        }
    }

    private sealed class ExtractedRelaxedPlan
    {
        public double PlanCost { get; }
        public HashSet<PreferredActionKey> PreferredActions { get; }
        public bool HasPreferredActionCandidates { get; }

        public ExtractedRelaxedPlan(
            double planCost,
            HashSet<PreferredActionKey> preferredActions,
            bool hasPreferredActionCandidates)
        {
            PlanCost = planCost;
            PreferredActions = preferredActions;
            HasPreferredActionCandidates = hasPreferredActionCandidates;
        }
    }

    private sealed class CompiledRelaxedAction
    {
        public required ActionSchema Schema { get; init; }
        public required CompiledConjunctiveCondition Precondition { get; init; }
        public required CompiledFluentAtom[] PositiveFluentPreconditions { get; init; }
        public required CompiledFluentAtom[] PositiveEffects { get; init; }
        public required CompiledNumericExpression CostExpression { get; init; }
        public required Constant?[] CostBindings { get; init; }

        public Fact<Fluent>[] ResolveDistinctPositivePreconditions(Problem problem, ReadOnlySpan<Constant> binding)
        {
            if (PositiveFluentPreconditions.Length == 0)
                return Array.Empty<Fact<Fluent>>();

            var facts = new List<Fact<Fluent>>(PositiveFluentPreconditions.Length);
            var seen = new HashSet<Fact<Fluent>>();
            foreach (var precondition in PositiveFluentPreconditions)
            {
                Fact<Fluent> fact = precondition.ResolveFact(problem, binding);
                if (seen.Add(fact))
                    facts.Add(fact);
            }

            return facts.ToArray();
        }
    }

    private sealed class CompiledFluentAtom
    {
        private readonly Core.Schemas.Predicate<Fluent> _predicate;
        private readonly int[] _variableSlots;
        private readonly Constant?[] _constantArguments;
        private readonly Constant[]? _scratchArgs;

        public CompiledFluentAtom(
            Core.Schemas.Predicate<Fluent> predicate,
            IReadOnlyList<ITerm> arguments,
            IReadOnlyDictionary<Variable, int> parameterSlots)
        {
            _predicate = predicate;
            _variableSlots = new int[arguments.Count];
            _constantArguments = new Constant?[arguments.Count];
            if (arguments.Count > 0)
                _scratchArgs = new Constant[arguments.Count];

            for (int i = 0; i < arguments.Count; i++)
            {
                _variableSlots[i] = -1;
                if (arguments[i] is Variable variable)
                {
                    _variableSlots[i] = parameterSlots[variable];
                }
                else
                {
                    _constantArguments[i] = (Constant)arguments[i];
                }
            }
        }

        public Fact<Fluent> ResolveOrRegisterFact(Problem problem, ReadOnlySpan<Constant> binding)
        {
            switch (_variableSlots.Length)
            {
                case 0:
                    return problem.Context.RegisterFact(_predicate, Array.Empty<Constant>());
                case 1:
                    return problem.Context.RegisterFact(_predicate, new[] { ResolveArgument(0, binding) });
                case 2:
                    return problem.Context.RegisterFact(
                        _predicate,
                        new[] { ResolveArgument(0, binding), ResolveArgument(1, binding) });
                case 3:
                    return problem.Context.RegisterFact(
                        _predicate,
                        new[]
                        {
                            ResolveArgument(0, binding),
                            ResolveArgument(1, binding),
                            ResolveArgument(2, binding)
                        });
                default:
                {
                    var scratchArgs = _scratchArgs!;
                    for (int i = 0; i < _variableSlots.Length; i++)
                        scratchArgs[i] = ResolveArgument(i, binding);
                    return problem.Context.RegisterFact(_predicate, scratchArgs);
                }
            }
        }

        public Fact<Fluent> ResolveFact(Problem problem, ReadOnlySpan<Constant> binding)
        {
            Fact<Fluent>? fact = FindFact(problem, binding);
            if (fact == null)
                throw new InvalidOperationException($"Expected fluent fact '{_predicate.Name}' to be preregistered for the current binding.");

            return fact;
        }

        private Fact<Fluent>? FindFact(Problem problem, ReadOnlySpan<Constant> binding)
        {
            switch (_variableSlots.Length)
            {
                case 0:
                    return problem.Context.FindFact(_predicate, Array.Empty<Constant>());
                case 1:
                    return problem.Context.FindFact(_predicate, new[] { ResolveArgument(0, binding) });
                case 2:
                    return problem.Context.FindFact(_predicate, new[] { ResolveArgument(0, binding), ResolveArgument(1, binding) });
                case 3:
                    return problem.Context.FindFact(_predicate, new[] { ResolveArgument(0, binding), ResolveArgument(1, binding), ResolveArgument(2, binding) });
                default:
                {
                    var scratchArgs = _scratchArgs!;
                    for (int i = 0; i < _variableSlots.Length; i++)
                        scratchArgs[i] = ResolveArgument(i, binding);
                    return problem.Context.FindFact(_predicate, scratchArgs);
                }
            }
        }

        private Constant ResolveArgument(int argumentIndex, ReadOnlySpan<Constant> binding)
        {
            int variableSlot = _variableSlots[argumentIndex];
            return variableSlot >= 0 ? binding[variableSlot] : _constantArguments[argumentIndex]!;
        }
    }

    private sealed class FirstAchiever
    {
        public required int ActionIndex { get; init; }
        public required Constant[] Binding { get; init; }
        public required double Cost { get; init; }
    }

    private readonly struct RelaxedActionKey : IEquatable<RelaxedActionKey>
    {
        private readonly int _actionIndex;
        private readonly int _arity;
        private readonly Constant? _arg1;
        private readonly Constant? _arg2;
        private readonly Constant? _arg3;
        private readonly Constant[]? _extraArgs;

        public RelaxedActionKey(int actionIndex, ReadOnlySpan<Constant> binding)
        {
            _actionIndex = actionIndex;
            _arity = binding.Length;
            _arg1 = binding.Length > 0 ? binding[0] : null;
            _arg2 = binding.Length > 1 ? binding[1] : null;
            _arg3 = binding.Length > 2 ? binding[2] : null;

            if (binding.Length > 3)
            {
                _extraArgs = new Constant[binding.Length - 3];
                for (int i = 3; i < binding.Length; i++)
                    _extraArgs[i - 3] = binding[i];
            }
            else
            {
                _extraArgs = null;
            }
        }

        public bool Equals(RelaxedActionKey other)
        {
            if (_actionIndex != other._actionIndex
                || _arity != other._arity
                || !ReferenceEquals(_arg1, other._arg1)
                || !ReferenceEquals(_arg2, other._arg2)
                || !ReferenceEquals(_arg3, other._arg3))
                return false;

            if (_extraArgs == null)
                return other._extraArgs == null;

            if (other._extraArgs == null || _extraArgs.Length != other._extraArgs.Length)
                return false;

            for (int i = 0; i < _extraArgs.Length; i++)
            {
                if (!ReferenceEquals(_extraArgs[i], other._extraArgs[i]))
                    return false;
            }

            return true;
        }

        public override bool Equals(object? obj) => obj is RelaxedActionKey other && Equals(other);

        public override int GetHashCode()
        {
            var hash = new HashCode();
            hash.Add(_actionIndex);
            hash.Add(_arity);
            hash.Add(_arg1 == null ? 0 : RuntimeHelpers.GetHashCode(_arg1));
            hash.Add(_arg2 == null ? 0 : RuntimeHelpers.GetHashCode(_arg2));
            hash.Add(_arg3 == null ? 0 : RuntimeHelpers.GetHashCode(_arg3));
            if (_extraArgs != null)
            {
                foreach (Constant arg in _extraArgs)
                    hash.Add(RuntimeHelpers.GetHashCode(arg));
            }

            return hash.ToHashCode();
        }
    }

    private readonly struct PreferredActionKey : IEquatable<PreferredActionKey>
    {
        private readonly ActionSchema _schema;
        private readonly int _arity;
        private readonly Constant? _arg1;
        private readonly Constant? _arg2;
        private readonly Constant? _arg3;
        private readonly Constant[]? _extraArgs;

        public PreferredActionKey(ActionSchema schema, ReadOnlySpan<Constant> binding)
        {
            _schema = schema;
            _arity = binding.Length;
            _arg1 = binding.Length > 0 ? binding[0] : null;
            _arg2 = binding.Length > 1 ? binding[1] : null;
            _arg3 = binding.Length > 2 ? binding[2] : null;

            if (binding.Length > 3)
            {
                _extraArgs = new Constant[binding.Length - 3];
                for (int i = 3; i < binding.Length; i++)
                    _extraArgs[i - 3] = binding[i];
            }
            else
            {
                _extraArgs = null;
            }
        }

        public PreferredActionKey(Action action)
        {
            _schema = action.Schema;
            _arity = action.Arguments.Count;
            _arg1 = action.Arguments.Count > 0 ? action.Arguments[0] : null;
            _arg2 = action.Arguments.Count > 1 ? action.Arguments[1] : null;
            _arg3 = action.Arguments.Count > 2 ? action.Arguments[2] : null;

            if (action.Arguments.Count > 3)
            {
                _extraArgs = new Constant[action.Arguments.Count - 3];
                for (int i = 3; i < action.Arguments.Count; i++)
                    _extraArgs[i - 3] = action.Arguments[i];
            }
            else
            {
                _extraArgs = null;
            }
        }

        public bool Equals(PreferredActionKey other)
        {
            if (!ReferenceEquals(_schema, other._schema)
                || _arity != other._arity
                || !ReferenceEquals(_arg1, other._arg1)
                || !ReferenceEquals(_arg2, other._arg2)
                || !ReferenceEquals(_arg3, other._arg3))
                return false;

            if (_extraArgs == null)
                return other._extraArgs == null;

            if (other._extraArgs == null || _extraArgs.Length != other._extraArgs.Length)
                return false;

            for (int i = 0; i < _extraArgs.Length; i++)
            {
                if (!ReferenceEquals(_extraArgs[i], other._extraArgs[i]))
                    return false;
            }

            return true;
        }

        public override bool Equals(object? obj) => obj is PreferredActionKey other && Equals(other);

        public override int GetHashCode()
        {
            var hash = new HashCode();
            hash.Add(RuntimeHelpers.GetHashCode(_schema));
            hash.Add(_arity);
            hash.Add(_arg1 == null ? 0 : RuntimeHelpers.GetHashCode(_arg1));
            hash.Add(_arg2 == null ? 0 : RuntimeHelpers.GetHashCode(_arg2));
            hash.Add(_arg3 == null ? 0 : RuntimeHelpers.GetHashCode(_arg3));
            if (_extraArgs != null)
            {
                foreach (Constant arg in _extraArgs)
                    hash.Add(RuntimeHelpers.GetHashCode(arg));
            }

            return hash.ToHashCode();
        }
    }

    private readonly Problem _problem;
    private readonly ConjunctiveConditionBindingGenerator _bindingGenerator;
    private readonly CompiledRelaxedAction[] _actions;
    private CompiledGoalData _currentGoal;

    public LiftedFfHeuristic(Problem problem, GoalCondition? goal = null)
    {
        _problem = problem ?? throw new ArgumentNullException(nameof(problem));
        problem.RequirePropositionalPlanning("Lifted FF");
        _bindingGenerator = new ConjunctiveConditionBindingGenerator();
        _actions = CompileActions(problem);
        _currentGoal = CompileGoal(goal ?? GoalCondition.FromProblem(problem), nameof(goal));
    }

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (state.State.Context.Problem != _problem)
            throw new ArgumentException("State belongs to a different problem instance than this heuristic.", nameof(state));

        CompiledGoalData compiledGoal = ResolveGoal(goal);
        HeuristicEvaluation evaluation = Evaluate(state, compiledGoal);
        if (goal is not null)
            _currentGoal = compiledGoal;

        return evaluation;
    }

    private HeuristicEvaluation Evaluate(ExtendedState state, CompiledGoalData compiledGoal)
    {
        if (compiledGoal.GoalFacts.Length == 0)
            return new(0);

        State initialState = state.State;
        State relaxedState = initialState;
        if (AreGoalsSatisfied(relaxedState, compiledGoal.GoalFacts))
            return new(0);

        var firstAchievers = new List<FirstAchiever?>(_problem.Context.FluentCount);
        var factCosts = new List<double>(_problem.Context.FluentCount);
        for (int i = 0; i < _problem.Context.FluentCount; i++)
        {
            firstAchievers.Add(null);
            factCosts.Add(initialState.IsTrue(new FluentIndex(i)) ? 0d : double.PositiveInfinity);
        }

        while (true)
        {
            var additionalFacts = new List<Fact<Fluent>>();
            var additionalFactSet = new HashSet<Fact<Fluent>>(ReferenceEqualityComparer.Instance);
            bool improvedAnyFact = false;

            for (int actionIndex = 0; actionIndex < _actions.Length; actionIndex++)
            {
                var action = _actions[actionIndex];
                _bindingGenerator.EnumerateBindingsUsingFluentState(action.Precondition, relaxedState, binding =>
                {
                    double supportCost = EvaluateActionCost(action, binding);
                    // An undefined cost makes the binding inapplicable.
                    if (double.IsNaN(supportCost))
                        return true;
                    foreach (Fact<Fluent> precondition in action.ResolveDistinctPositivePreconditions(_problem, binding))
                    {
                        double preconditionCost = factCosts[precondition.LocalIndex];
                        if (double.IsPositiveInfinity(preconditionCost))
                            return true;

                        supportCost += preconditionCost;
                    }

                    foreach (var effect in action.PositiveEffects)
                    {
                        Fact<Fluent> fact = effect.ResolveOrRegisterFact(_problem, binding);
                        EnsureTrackedFact(fact, initialState, firstAchievers, factCosts);
                        int factIndex = fact.LocalIndex;
                        if (supportCost >= factCosts[factIndex])
                            continue;

                        factCosts[factIndex] = supportCost;
                        improvedAnyFact = true;

                        if (!relaxedState.IsTrue(fact) && additionalFactSet.Add(fact))
                        {
                            additionalFacts.Add(fact);
                        }

                        firstAchievers[factIndex] = new FirstAchiever
                        {
                            ActionIndex = actionIndex,
                            Binding = binding.ToArray(),
                            Cost = supportCost
                        };
                    }

                    return true;
                });
            }

            if (!improvedAnyFact)
            {
                if (!AreGoalsSatisfied(relaxedState, compiledGoal.GoalFacts))
                    return new(double.PositiveInfinity);

                var extractedPlan = ExtractRelaxedPlan(state, firstAchievers, compiledGoal.GoalFacts);
                Func<Action, bool>? isPreferredAction = !extractedPlan.HasPreferredActionCandidates
                    ? null
                    : action => ReferenceEquals(action.Context, initialState.Context)
                        && extractedPlan.PreferredActions.Contains(new PreferredActionKey(action));
                return new(extractedPlan.PlanCost, isPreferredAction);
            }

            relaxedState = relaxedState.WithAdditionalFluentFacts(additionalFacts);
        }
    }

    private CompiledGoalData ResolveGoal(GoalCondition? goal)
    {
        if (goal is null)
            return _currentGoal;

        EnsureGoalBelongsToProblem(goal, nameof(goal));
        return _currentGoal.Goal.Equals(goal)
            ? _currentGoal
            : new CompiledGoalData(goal, CompileGoals(goal));
    }

    private CompiledGoalData CompileGoal(GoalCondition goal, string parameterName)
    {
        EnsureGoalBelongsToProblem(goal, parameterName);
        return new CompiledGoalData(goal, CompileGoals(goal));
    }

    private void EnsureGoalBelongsToProblem(GoalCondition goal, string parameterName)
    {
        if (!ReferenceEquals(goal.Problem, _problem))
            throw new ArgumentException("Goal belongs to a different problem instance than this heuristic.", parameterName);
    }

    private CompiledRelaxedAction[] CompileActions(Problem problem)
    {
        var compiledActions = new List<CompiledRelaxedAction>();

        foreach (var schema in problem.Domain.Actions)
        {
            ValidateSupportedEffects(schema);

            var parameterSlots = new Dictionary<Variable, int>(schema.Parameters.Count, ReferenceEqualityComparer.Instance);
            for (int i = 0; i < schema.Parameters.Count; i++)
                parameterSlots[schema.Parameters[i]] = i;

            var positiveFluentPreconditions = schema.FluentPreconditions
                .Where(literal => literal.Polarity == Polarity.Positive)
                .ToArray();
            var staticPreconditions = schema.StaticPreconditions.ToArray();

            var compiledAction = new CompiledRelaxedAction
            {
                Schema = schema,
                Precondition = _bindingGenerator.Compile(
                    problem,
                    schema.Parameters,
                    positiveFluentPreconditions,
                    staticPreconditions,
                    Array.Empty<Literal<Atom<Derived>>>(),
                    schema.NumericPreconditions),
                PositiveFluentPreconditions = positiveFluentPreconditions
                    .Select(literal => new CompiledFluentAtom(literal.Value.Predicate, literal.Value.Arguments, parameterSlots))
                    .ToArray(),
                PositiveEffects = schema.Effects
                    .Where(effect => effect.Effect.Polarity == Polarity.Positive)
                    .Select(effect => new CompiledFluentAtom(effect.Effect.Value.Predicate, effect.Effect.Value.Arguments, parameterSlots))
                    .ToArray(),
                CostExpression = CompiledNumericExpression.Compile(schema.CostExpression, parameterSlots),
                CostBindings = new Constant?[schema.Parameters.Count]
            };

            if (compiledAction.PositiveEffects.Length == 0)
                continue;

            compiledActions.Add(compiledAction);
        }

        return compiledActions.ToArray();
    }

    private static Fact<Fluent>[] CompileGoals(GoalCondition goal)
    {
        if (goal.NumericConditions.Count > 0) throw new NotSupportedException("Lifted FF does not support numeric goals.");
        var goalLiterals = goal.GoalLiterals;
        if (goalLiterals == null)
            throw new NotSupportedException("Lifted FF v1 only supports positive conjunctive fluent goals.");

        if (goalLiterals.Count == 0)
            return Array.Empty<Fact<Fluent>>();

        var goalFacts = new List<Fact<Fluent>>();
        var seen = new HashSet<Fact<Fluent>>(ReferenceEqualityComparer.Instance);
        foreach (var goalLiteral in goalLiterals)
        {
            if (goalLiteral.Polarity != Polarity.Positive || goalLiteral.Value is not Fact<Fluent> fluentGoal)
                throw new NotSupportedException("Lifted FF v1 only supports positive conjunctive fluent goals.");

            if (seen.Add(fluentGoal))
                goalFacts.Add(fluentGoal);
        }

        return goalFacts.ToArray();
    }

    private static void ValidateSupportedEffects(ActionSchema schema)
    {
        foreach (var effect in schema.Effects)
        {
            if (effect.QuantifiedVariables.Count > 0)
                throw new NotSupportedException($"Lifted FF v1 does not support quantified effects (action '{schema.Name}').");

            if (effect.FluentConditions.Count > 0 || effect.StaticConditions.Count > 0 || effect.DerivedConditions.Count > 0)
                throw new NotSupportedException($"Lifted FF v1 does not support conditional effects (action '{schema.Name}').");
        }
    }

    private static bool AreGoalsSatisfied(State state, Fact<Fluent>[] goalFacts)
    {
        foreach (Fact<Fluent> goalFact in goalFacts)
        {
            if (!state.IsTrue(goalFact))
                return false;
        }

        return true;
    }

    private ExtractedRelaxedPlan ExtractRelaxedPlan(
        ExtendedState state,
        IReadOnlyList<FirstAchiever?> firstAchievers,
        Fact<Fluent>[] goalFacts)
    {
        State initialState = state.State;
        var pendingFacts = new Stack<Fact<Fluent>>();
        var seenFacts = new HashSet<Fact<Fluent>>(ReferenceEqualityComparer.Instance);
        foreach (Fact<Fluent> goalFact in goalFacts)
        {
            if (!initialState.IsTrue(goalFact))
            {
                pendingFacts.Push(goalFact);
                seenFacts.Add(goalFact);
            }
        }

        var selectedActions = new HashSet<RelaxedActionKey>();
        var preferredActions = new HashSet<PreferredActionKey>();
        bool hasPreferredActionCandidates = false;
        double totalCost = 0d;
        while (pendingFacts.Count > 0)
        {
            Fact<Fluent> fact = pendingFacts.Pop();
            if (initialState.IsTrue(fact))
                continue;

            var achiever = firstAchievers[fact.LocalIndex];
            if (achiever == null)
                throw new InvalidOperationException("A relaxed goal fact was marked reachable without a recorded achiever.");

            var actionKey = new RelaxedActionKey(achiever.ActionIndex, achiever.Binding);
            if (!selectedActions.Add(actionKey))
                continue;

            var action = _actions[achiever.ActionIndex];
            totalCost += EvaluateActionCost(action, achiever.Binding);
            bool isPreferredAction = true;
            foreach (Fact<Fluent> precondition in action.ResolveDistinctPositivePreconditions(_problem, achiever.Binding))
            {
                if (initialState.IsTrue(precondition))
                    continue;

                isPreferredAction = false;
                if (!seenFacts.Add(precondition))
                    continue;

                pendingFacts.Push(precondition);
            }

            if (isPreferredAction)
            {
                hasPreferredActionCandidates = true;
                Action preferredAction = ActionBuilder.BuildAction(
                    action.Schema,
                    achiever.Binding,
                    _problem);
                if (preferredAction.IsApplicable(state))
                    preferredActions.Add(new PreferredActionKey(action.Schema, achiever.Binding));
            }
        }

        return new ExtractedRelaxedPlan(
            totalCost,
            preferredActions,
            hasPreferredActionCandidates);
    }

    private static void EnsureTrackedFact(
        Fact<Fluent> fact,
        State initialState,
        List<FirstAchiever?> firstAchievers,
        List<double> factCosts)
    {
        while (factCosts.Count <= fact.LocalIndex)
        {
            int localIndex = factCosts.Count;
            firstAchievers.Add(null);
            factCosts.Add(initialState.IsTrue(new FluentIndex(localIndex))
                ? 0d
                : double.PositiveInfinity);
        }

        if (firstAchievers.Count != factCosts.Count)
            throw new InvalidOperationException("Lifted heuristic fact tracking is inconsistent.");
    }

    private double EvaluateActionCost(CompiledRelaxedAction action, ReadOnlySpan<Constant> binding)
    {
        Constant?[] bindings = action.CostBindings;
        for (int i = 0; i < binding.Length; i++)
            bindings[i] = binding[i];

        double cost = action.CostExpression.Evaluate(_problem.Context, bindings, state: null);
        if (cost < 0d)
            throw new InvalidOperationException($"Lifted action '{action.Schema.Name}' has a negative cost ({cost}).");

        return cost;
    }
}
