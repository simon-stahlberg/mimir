using System.Runtime.CompilerServices;
using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

/// <summary>
/// Shares the ground actions of one schema across states, keyed by the binding. Entries are weak, so an action
/// no consumer still holds is rebuilt on demand; <see cref="Scavenge"/> drops the keys those leave behind.
/// </summary>
internal sealed class GroundActionCache
{
    private readonly ActionGroundingPlan _plan;
    private readonly Problem _problem;
    private readonly Dictionary<BindingKey, Entry> _entries = new(BindingKeyComparer.Instance);
    private readonly List<BindingKey> _staleBindings = new();

    public GroundActionCache(ActionGroundingPlan plan, Problem problem)
    {
        _plan = plan;
        _problem = problem;
    }

    public Action GetOrBuild(ReadOnlySpan<Constant> binding)
    {
        Dictionary<BindingKey, Entry>.AlternateLookup<ReadOnlySpan<Constant>> lookup =
            _entries.GetAlternateLookup<ReadOnlySpan<Constant>>();
        if (lookup.TryGetValue(binding, out Entry? entry))
        {
            if (entry.Reference.TryGetTarget(out Action? cached))
                return cached;

            lookup.Remove(binding);
            Action rebuilt = ActionBuilder.BuildActionWithOwnedArguments(_plan, entry.Arguments, _problem);
            entry.Reference.SetTarget(rebuilt);
            _entries.Add(new BindingKey(entry.Arguments), entry);
            return rebuilt;
        }

        var key = new BindingKey(binding);
        Action action = ActionBuilder.BuildActionWithOwnedArguments(_plan, key.Arguments, _problem);
        _entries[key] = new Entry(key.Arguments, action);
        return action;
    }

    public void Scavenge()
    {
        foreach ((BindingKey binding, Entry entry) in _entries)
        {
            if (!entry.Reference.TryGetTarget(out _)) _staleBindings.Add(binding);
        }

        foreach (BindingKey binding in _staleBindings)
            _entries.Remove(binding);

        _staleBindings.Clear();
    }

    private sealed class Entry(Constant[] arguments, Action action)
    {
        public Constant[] Arguments { get; } = arguments;
        public WeakReference<Action> Reference { get; } = new(action);
    }

    private readonly struct BindingKey : IEquatable<BindingKey>
    {
        private readonly Constant[] _arguments;

        public BindingKey(ReadOnlySpan<Constant> binding) => _arguments = binding.ToArray();

        public BindingKey(Constant[] arguments) => _arguments = arguments;

        public Constant[] Arguments => _arguments;

        public bool Equals(BindingKey other) => Equals(other._arguments);

        public bool Equals(ReadOnlySpan<Constant> binding)
        {
            if (_arguments.Length != binding.Length) return false;
            for (int i = 0; i < _arguments.Length; i++)
            {
                if (!ReferenceEquals(_arguments[i], binding[i])) return false;
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
}
