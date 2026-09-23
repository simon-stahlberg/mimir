using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;
using System.Runtime.CompilerServices;

namespace Mimir.Core.Engines;

public class CliqueApplicableActionGenerator : IApplicableActionGenerator
{
    private readonly struct BindingKey : IEquatable<BindingKey>
    {
        private readonly Constant[] _arguments;

        public BindingKey(ReadOnlySpan<Constant> binding)
        {
            _arguments = binding.ToArray();
        }

        public BindingKey(Constant[] arguments)
        {
            _arguments = arguments;
        }

        public Constant[] Arguments => _arguments;

        public bool Equals(BindingKey other) => Equals(other._arguments);

        public bool Equals(ReadOnlySpan<Constant> binding)
        {
            if (_arguments.Length != binding.Length)
                return false;

            for (int i = 0; i < _arguments.Length; i++)
            {
                if (!ReferenceEquals(_arguments[i], binding[i]))
                    return false;
            }

            return true;
        }

        public override bool Equals(object? obj) => obj is BindingKey other && Equals(other);

        public override int GetHashCode() => GetHashCode(_arguments);

        public static int GetHashCode(ReadOnlySpan<Constant> binding)
        {
            var hash = new HashCode();
            hash.Add(binding.Length);
            foreach (Constant argument in binding)
                hash.Add(RuntimeHelpers.GetHashCode(argument));

            return hash.ToHashCode();
        }
    }

    private sealed class BindingKeyComparer :
        IEqualityComparer<BindingKey>,
        IAlternateEqualityComparer<ReadOnlySpan<Constant>, BindingKey>
    {
        public static BindingKeyComparer Instance { get; } = new();

        private BindingKeyComparer()
        {
        }

        public bool Equals(BindingKey left, BindingKey right) => left.Equals(right);
        public int GetHashCode(BindingKey key) => key.GetHashCode();
        public bool Equals(ReadOnlySpan<Constant> binding, BindingKey key) => key.Equals(binding);
        public int GetHashCode(ReadOnlySpan<Constant> binding) => BindingKey.GetHashCode(binding);
        public BindingKey Create(ReadOnlySpan<Constant> binding) => new(binding);
    }

    private sealed class ActionSchemaCache
    {
        public ActionGroundingPlan GroundingPlan = null!;
        public WeakReference<GroundAction>? NullaryAction;
        public CompiledConjunctiveCondition Preconditions = null!;
        public bool[] TransitionRelevantParameters = null!;
        public bool HasExistentialWitnessParameters;

        public Dictionary<BindingKey, CachedGroundAction> GroundActionCache = null!;
        public List<BindingKey> StaleBindings = new();
    }

    private sealed class CachedGroundAction
    {
        public Constant[] Arguments { get; }
        public WeakReference<GroundAction> Reference { get; }

        public CachedGroundAction(Constant[] arguments, GroundAction action)
        {
            Arguments = arguments;
            Reference = new WeakReference<GroundAction>(action);
        }
    }

    private sealed class ApplicableActionCollector
    {
        private readonly CliqueApplicableActionGenerator _generator;
        private ActionSchemaCache _schemaCache = null!;
        private ExtendedState _state = null!;
        private List<GroundAction> _actions = null!;
        private int _maxActions;

        public BindingCallback Callback { get; }

        public ApplicableActionCollector(CliqueApplicableActionGenerator generator)
        {
            _generator = generator;
            Callback = Collect;
        }

        public void Prepare(
            ActionSchemaCache schemaCache,
            ExtendedState state,
            List<GroundAction> actions,
            int maxActions)
        {
            _schemaCache = schemaCache;
            _state = state;
            _actions = actions;
            _maxActions = maxActions;
        }

        public void Release()
        {
            _schemaCache = null!;
            _state = null!;
            _actions = null!;
            _maxActions = 0;
        }

        private bool Collect(ReadOnlySpan<Constant> binding)
        {
            GroundAction action = _generator.GetOrBuildAction(_schemaCache, binding);
            if (!action.IsTransitionDefined(_state))
                return true;
            _actions.Add(action);
            return _actions.Count < _maxActions;
        }
    }

    public Problem Problem { get; }
    private readonly ConjunctiveConditionBindingGenerator _bindingGenerator = new();
    private readonly ActionSchemaCache[] _schemaCaches;
    private readonly ApplicableActionCollector _actionCollector;
    private int _observedGen0Collections;

    internal CliqueApplicableActionGenerator(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        Problem = problem ?? throw new ArgumentNullException(nameof(problem));
        // One reusable reference callback avoids cold generic-specialization allocations
        // while keeping repeated applicable-action collection allocation-free.
        _actionCollector = new ApplicableActionCollector(this);
        var schemaCaches = new List<ActionSchemaCache>();

        foreach (ActionSchema schema in Problem.Domain.Actions)
        {
            ActionGroundingPlan groundingPlan =
                Problem.Context.GetActionGroundingPlan(schema);
            var schemaCache = new ActionSchemaCache
            {
                GroundingPlan = groundingPlan,
            };
            if (schema.Parameters.Count == 0)
            {
                GroundAction action = ActionBuilder.BuildAction(
                    groundingPlan,
                    Array.Empty<Constant>(),
                    Problem);
                schemaCache.NullaryAction = new WeakReference<GroundAction>(action);
                schemaCaches.Add(schemaCache);
                continue;
            }

            schemaCache.TransitionRelevantParameters = groundingPlan.TransitionRelevantParameters;
            schemaCache.Preconditions = _bindingGenerator.Compile(
                Problem,
                schema.Parameters,
                schema.FluentPreconditions,
                schema.StaticPreconditions,
                schema.DerivedPreconditions,
                schema.NumericPreconditions);
            schemaCache.HasExistentialWitnessParameters =
                schemaCache.TransitionRelevantParameters.Any(isRelevant => !isRelevant);
            schemaCache.GroundActionCache = new Dictionary<BindingKey, CachedGroundAction>(
                BindingKeyComparer.Instance);

            schemaCaches.Add(schemaCache);
        }

        _schemaCaches = schemaCaches.ToArray();
        _ = Problem.InitialState;
        _observedGen0Collections = GC.CollectionCount(0);
    }

    internal void CollectApplicableActions(
        ExtendedState state,
        List<GroundAction> destination)
    {
        ValidateState(state);
        ArgumentNullException.ThrowIfNull(destination);
        ScavengeCollectedActions();

        destination.Clear();
        CollectApplicableActions(state, int.MaxValue, destination);
    }

    private void CollectApplicableActions(
        ExtendedState state,
        int maxActions,
        List<GroundAction> actions)
    {
        foreach (ActionSchemaCache schemaCache in _schemaCaches)
        {
            if (schemaCache.NullaryAction is not null)
            {
                // Keep weak-cache rebuilding out of this hot method; inlining it
                // measurably regresses tiered-JIT code quality on lifted searches.
                GroundAction nullaryAction = GetOrBuildNullaryAction(schemaCache);
                if (!nullaryAction.IsApplicable(state))
                    continue;

                actions.Add(nullaryAction);
                if (actions.Count >= maxActions)
                    break;

                continue;
            }

            ApplicableActionCollector collector = _actionCollector;
            collector.Prepare(schemaCache, state, actions, maxActions);
            try
            {
                if (schemaCache.HasExistentialWitnessParameters)
                {
                    _bindingGenerator.EnumerateRepresentativeBindings(
                        schemaCache.Preconditions,
                        state,
                        schemaCache.TransitionRelevantParameters,
                        collector.Callback);
                }
                else
                {
                    _bindingGenerator.EnumerateBindings(
                        schemaCache.Preconditions,
                        state,
                        collector.Callback);
                }
            }
            finally
            {
                collector.Release();
            }

            if (actions.Count >= maxActions)
                break;
        }
    }

    public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state)
        => GetApplicableActions(state, int.MaxValue);

    public IEnumerable<GroundAction> GetApplicableActions(ExtendedState state, int maxActions)
    {
        ValidateState(state);
        if (maxActions < 0)
            throw new ArgumentOutOfRangeException(nameof(maxActions));
        if (maxActions == 0)
            return Array.Empty<GroundAction>();
        ScavengeCollectedActions();

        var actions = new List<GroundAction>();
        CollectApplicableActions(state, maxActions, actions);
        return actions;
    }

    private GroundAction GetOrBuildAction(
        ActionSchemaCache schemaCache,
        ReadOnlySpan<Constant> binding)
    {
        Dictionary<BindingKey, CachedGroundAction>.AlternateLookup<ReadOnlySpan<Constant>> lookup =
            schemaCache.GroundActionCache.GetAlternateLookup<ReadOnlySpan<Constant>>();
        if (lookup.TryGetValue(binding, out CachedGroundAction? cachedActionEntry))
        {
            if (cachedActionEntry.Reference.TryGetTarget(out GroundAction? cachedAction))
                return cachedAction;

            lookup.Remove(binding);
            GroundAction rebuiltAction = ActionBuilder.BuildActionWithOwnedArguments(
                schemaCache.GroundingPlan,
                cachedActionEntry.Arguments,
                Problem);
            cachedActionEntry.Reference.SetTarget(rebuiltAction);
            schemaCache.GroundActionCache.Add(
                new BindingKey(cachedActionEntry.Arguments),
                cachedActionEntry);
            return rebuiltAction;
        }

        var key = new BindingKey(binding);
        GroundAction action = ActionBuilder.BuildActionWithOwnedArguments(
            schemaCache.GroundingPlan,
            key.Arguments,
            Problem);
        schemaCache.GroundActionCache[key] = new CachedGroundAction(key.Arguments, action);
        return action;
    }

    private GroundAction GetOrBuildNullaryAction(ActionSchemaCache schemaCache)
    {
        if (schemaCache.NullaryAction!.TryGetTarget(out GroundAction? action))
            return action;

        action = ActionBuilder.BuildAction(
            schemaCache.GroundingPlan,
            Array.Empty<Constant>(),
            Problem);
        schemaCache.NullaryAction.SetTarget(action);
        return action;
    }

    private void ScavengeCollectedActions()
    {
        Problem.Context.ScavengeActionComponentCaches();
        int gen0Collections = GC.CollectionCount(0);
        if (gen0Collections == _observedGen0Collections)
            return;

        _observedGen0Collections = gen0Collections;
        foreach (ActionSchemaCache schemaCache in _schemaCaches)
        {
            if (schemaCache.NullaryAction is not null)
                continue;

            foreach ((BindingKey binding, CachedGroundAction action) in schemaCache.GroundActionCache)
            {
                if (!action.Reference.TryGetTarget(out _))
                    schemaCache.StaleBindings.Add(binding);
            }

            foreach (BindingKey binding in schemaCache.StaleBindings)
                schemaCache.GroundActionCache.Remove(binding);

            schemaCache.StaleBindings.Clear();
        }
    }

    private void ValidateState(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.State.Context, Problem.Context))
            throw new InvalidOperationException("State and applicable-action generator belong to different problem contexts.");
    }
}
