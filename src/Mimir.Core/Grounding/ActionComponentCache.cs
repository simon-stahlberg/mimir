namespace Mimir.Core.Grounding;

using Mimir.Core.Schemas;
using System.Runtime.CompilerServices;

internal sealed class ActionComponentCache
{
    private readonly Dictionary<
        ProjectedBindingKey,
        WeakReference<ActionPreconditions>>? _preconditions;
    private readonly Dictionary<
        ProjectedBindingKey,
        WeakReference<ActionEffects>>? _effects;
    private readonly ProjectedBindingKeyComparer? _preconditionComparer;
    private readonly ProjectedBindingKeyComparer? _effectComparer;
    private readonly List<ProjectedBindingKey> _deadPreconditions = new();
    private readonly List<ProjectedBindingKey> _deadEffects = new();

    public bool HasCaches => _preconditions is not null || _effects is not null;

    public ActionComponentCache(
        bool[] preconditionRelevantParameters,
        bool[] effectRelevantParameters)
    {
        ArgumentNullException.ThrowIfNull(preconditionRelevantParameters);
        ArgumentNullException.ThrowIfNull(effectRelevantParameters);

        if (CanShare(preconditionRelevantParameters))
        {
            _preconditionComparer =
                new ProjectedBindingKeyComparer(preconditionRelevantParameters);
            _preconditions = new Dictionary<
                ProjectedBindingKey,
                WeakReference<ActionPreconditions>>(_preconditionComparer);
        }

        if (CanShare(effectRelevantParameters))
        {
            _effectComparer =
                new ProjectedBindingKeyComparer(effectRelevantParameters);
            _effects = new Dictionary<
                ProjectedBindingKey,
                WeakReference<ActionEffects>>(_effectComparer);
        }
    }

    public bool TryGetPreconditions(
        ReadOnlySpan<Constant> binding,
        out ActionPreconditions? preconditions)
    {
        if (_preconditions is null)
        {
            preconditions = null;
            return false;
        }

        Dictionary<ProjectedBindingKey, WeakReference<ActionPreconditions>>
            .AlternateLookup<ReadOnlySpan<Constant>> lookup =
                _preconditions.GetAlternateLookup<ReadOnlySpan<Constant>>();
        if (lookup.TryGetValue(binding, out WeakReference<ActionPreconditions>? weak)
            && weak.TryGetTarget(out preconditions))
        {
            return true;
        }

        preconditions = null;
        return false;
    }

    public void StorePreconditions(
        ReadOnlySpan<Constant> binding,
        ActionPreconditions preconditions)
    {
        ArgumentNullException.ThrowIfNull(preconditions);
        if (_preconditions is null)
            return;

        Dictionary<ProjectedBindingKey, WeakReference<ActionPreconditions>>
            .AlternateLookup<ReadOnlySpan<Constant>> lookup =
                _preconditions.GetAlternateLookup<ReadOnlySpan<Constant>>();
        if (lookup.TryGetValue(binding, out WeakReference<ActionPreconditions>? weak))
        {
            weak.SetTarget(preconditions);
            return;
        }

        _preconditions.Add(
            _preconditionComparer!.Create(binding),
            new WeakReference<ActionPreconditions>(preconditions));
    }

    public bool TryGetEffects(
        ReadOnlySpan<Constant> binding,
        out ActionEffects? effects)
    {
        if (_effects is null)
        {
            effects = null;
            return false;
        }

        Dictionary<ProjectedBindingKey, WeakReference<ActionEffects>>
            .AlternateLookup<ReadOnlySpan<Constant>> lookup =
                _effects.GetAlternateLookup<ReadOnlySpan<Constant>>();
        if (lookup.TryGetValue(binding, out WeakReference<ActionEffects>? weak)
            && weak.TryGetTarget(out effects))
        {
            return true;
        }

        effects = null;
        return false;
    }

    public void StoreEffects(
        ReadOnlySpan<Constant> binding,
        ActionEffects effects)
    {
        ArgumentNullException.ThrowIfNull(effects);
        if (_effects is null)
            return;

        Dictionary<ProjectedBindingKey, WeakReference<ActionEffects>>
            .AlternateLookup<ReadOnlySpan<Constant>> lookup =
                _effects.GetAlternateLookup<ReadOnlySpan<Constant>>();
        if (lookup.TryGetValue(binding, out WeakReference<ActionEffects>? weak))
        {
            weak.SetTarget(effects);
            return;
        }

        _effects.Add(
            _effectComparer!.Create(binding),
            new WeakReference<ActionEffects>(effects));
    }

    public void Scavenge()
    {
        if (_preconditions is not null)
        {
            foreach ((ProjectedBindingKey key,
                     WeakReference<ActionPreconditions> weak) in _preconditions)
            {
                if (!weak.TryGetTarget(out _))
                    _deadPreconditions.Add(key);
            }

            foreach (ProjectedBindingKey key in _deadPreconditions)
                _preconditions.Remove(key);

            _deadPreconditions.Clear();
        }

        if (_effects is not null)
        {
            foreach ((ProjectedBindingKey key,
                     WeakReference<ActionEffects> weak) in _effects)
            {
                if (!weak.TryGetTarget(out _))
                    _deadEffects.Add(key);
            }

            foreach (ProjectedBindingKey key in _deadEffects)
                _effects.Remove(key);

            _deadEffects.Clear();
        }
    }

    private static bool CanShare(bool[] relevantParameters)
        => relevantParameters.Length > 0
            && relevantParameters.Any(relevant => !relevant);

    private readonly struct ProjectedBindingKey :
        IEquatable<ProjectedBindingKey>
    {
        private readonly Constant? _arg1;
        private readonly Constant? _arg2;
        private readonly Constant? _arg3;
        private readonly Constant? _arg4;
        private readonly Constant[]? _extraArgs;
        private readonly int _count;
        private readonly int _hashCode;

        public ProjectedBindingKey(
            ReadOnlySpan<Constant> binding,
            int[] relevantIndices)
        {
            _count = relevantIndices.Length;
            _arg1 = _count > 0 ? binding[relevantIndices[0]] : null;
            _arg2 = _count > 1 ? binding[relevantIndices[1]] : null;
            _arg3 = _count > 2 ? binding[relevantIndices[2]] : null;
            _arg4 = _count > 3 ? binding[relevantIndices[3]] : null;
            _extraArgs = _count > 4
                ? new Constant[_count - 4]
                : null;
            if (_extraArgs is not null)
            {
                for (int index = 4; index < _count; index++)
                {
                    _extraArgs[index - 4] =
                        binding[relevantIndices[index]];
                }
            }

            _hashCode = GetHashCode(binding, relevantIndices);
        }

        public bool Equals(ProjectedBindingKey other)
        {
            if (_count != other._count || _hashCode != other._hashCode)
                return false;

            for (int index = 0; index < _count; index++)
            {
                if (!ReferenceEquals(GetArgument(index), other.GetArgument(index)))
                    return false;
            }

            return true;
        }

        public bool Equals(
            ReadOnlySpan<Constant> binding,
            int[] relevantIndices)
        {
            if (_count != relevantIndices.Length)
                return false;

            for (int index = 0; index < _count; index++)
            {
                if (!ReferenceEquals(
                        GetArgument(index),
                        binding[relevantIndices[index]]))
                {
                    return false;
                }
            }

            return true;
        }

        public override bool Equals(object? obj)
            => obj is ProjectedBindingKey other && Equals(other);

        public override int GetHashCode() => _hashCode;

        public static int GetHashCode(
            ReadOnlySpan<Constant> binding,
            int[] relevantIndices)
        {
            var hash = new HashCode();
            hash.Add(relevantIndices.Length);
            for (int index = 0; index < relevantIndices.Length; index++)
            {
                hash.Add(RuntimeHelpers.GetHashCode(
                    binding[relevantIndices[index]]));
            }

            return hash.ToHashCode();
        }

        private Constant? GetArgument(int index)
            => index switch
            {
                0 => _arg1,
                1 => _arg2,
                2 => _arg3,
                3 => _arg4,
                _ => _extraArgs![index - 4],
            };
    }

    private sealed class ProjectedBindingKeyComparer :
        IEqualityComparer<ProjectedBindingKey>,
        IAlternateEqualityComparer<ReadOnlySpan<Constant>, ProjectedBindingKey>
    {
        private readonly int[] _relevantIndices;

        public ProjectedBindingKeyComparer(bool[] relevantParameters)
        {
            _relevantIndices = relevantParameters
                .Select((relevant, index) => (relevant, index))
                .Where(item => item.relevant)
                .Select(item => item.index)
                .ToArray();
        }

        public bool Equals(ProjectedBindingKey left, ProjectedBindingKey right)
            => left.Equals(right);

        public int GetHashCode(ProjectedBindingKey key) => key.GetHashCode();

        public bool Equals(
            ReadOnlySpan<Constant> binding,
            ProjectedBindingKey key)
            => key.Equals(binding, _relevantIndices);

        public int GetHashCode(ReadOnlySpan<Constant> binding)
            => ProjectedBindingKey.GetHashCode(binding, _relevantIndices);

        public ProjectedBindingKey Create(ReadOnlySpan<Constant> binding)
            => new(binding, _relevantIndices);
    }
}
