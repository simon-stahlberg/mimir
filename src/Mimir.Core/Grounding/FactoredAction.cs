using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

/// <summary>
/// A set of applicable ground actions of one schema, stored as a candidate set per parameter.
/// Every tuple of the Cartesian product is applicable except those ruled out by <see cref="Exclusions"/>.
/// </summary>
public sealed class FactoredAction
{
    private readonly ActionGroundingPlan _plan;
    private readonly GroundActionCache _actions;
    private readonly Constant[][] _candidates;
    private readonly bool[] _witnessParameters;

    public ActionSchema Schema => _plan.Schema;
    public int ParameterCount => _candidates.Length;
    public IReadOnlyList<FactoredExclusion> Exclusions { get; }

    internal FactoredAction(
        ActionGroundingPlan plan,
        GroundActionCache actions,
        Constant[][] candidates,
        bool[] witnessParameters,
        IReadOnlyList<FactoredExclusion> exclusions)
    {
        _plan = plan;
        _actions = actions;
        _candidates = candidates;
        _witnessParameters = witnessParameters;
        Exclusions = exclusions;
    }

    public IReadOnlyList<Constant> GetCandidates(int parameterIndex) => _candidates[parameterIndex];

    public bool IsPinned(int parameterIndex) => _candidates[parameterIndex].Length == 1;

    /// <summary>
    /// True when the parameter occurs in no effect, effect condition or cost, so the group carries a single
    /// representative rather than every value that satisfies the precondition.
    /// </summary>
    public bool IsWitness(int parameterIndex) => _witnessParameters[parameterIndex];

    /// <summary>
    /// The product of the candidate set sizes. Exact when <see cref="Exclusions"/> is empty, an upper bound otherwise.
    /// Saturates at <see cref="long.MaxValue"/>.
    /// </summary>
    public long MemberCountUpperBound
    {
        get
        {
            long count = 1;
            foreach (Constant[] candidates in _candidates)
            {
                if (candidates.Length == 0) return 0;
                if (count > long.MaxValue / candidates.Length) return long.MaxValue;
                count *= candidates.Length;
            }

            return count;
        }
    }

    public IEnumerable<Action> Expand()
    {
        int parameterCount = _candidates.Length;
        if (parameterCount == 0)
        {
            yield return _actions.GetOrBuild(ReadOnlySpan<Constant>.Empty);
            yield break;
        }

        foreach (Constant[] candidates in _candidates)
        {
            if (candidates.Length == 0) yield break;
        }

        var cursor = new int[parameterCount];
        var binding = new Constant[parameterCount];
        while (true)
        {
            for (int i = 0; i < parameterCount; i++)
                binding[i] = _candidates[i][cursor[i]];

            if (IsAllowed(binding))
                yield return _actions.GetOrBuild(binding);

            int position = parameterCount - 1;
            while (position >= 0 && ++cursor[position] == _candidates[position].Length)
                cursor[position--] = 0;
            if (position < 0) yield break;
        }
    }

    private bool IsAllowed(Constant[] binding)
    {
        for (int i = 0; i < Exclusions.Count; i++)
        {
            if (!Exclusions[i].IsSatisfiedBy(binding))
                return false;
        }

        return true;
    }

    public override string ToString()
    {
        IEnumerable<string> parameters = _candidates.Select(candidates => candidates.Length == 1
            ? candidates[0].Name
            : $"{{{string.Join(",", candidates.Select(candidate => candidate.Name))}}}");
        return $"({Schema.Name} {string.Join(" ", parameters)})";
    }
}

/// <summary>
/// A constraint that the clique-free product representation cannot express, carried alongside the candidate sets
/// so that a negative precondition does not have to couple two otherwise independent parameters.
/// </summary>
public sealed class FactoredExclusion
{
    private readonly int[] _parameterIndices;
    private readonly HashSet<Constant[]>? _forbiddenTuples;

    public IReadOnlyList<int> ParameterIndices => _parameterIndices;

    /// <summary>Requires two parameters to take different objects, as `(not (= ?x ?y))` does.</summary>
    public bool IsDistinctness => _forbiddenTuples is null;

    private FactoredExclusion(int[] parameterIndices, Constant[][]? forbiddenTuples)
    {
        _parameterIndices = parameterIndices;
        _forbiddenTuples = forbiddenTuples is null
            ? null
            : new HashSet<Constant[]>(forbiddenTuples, TupleComparer.Instance);
    }

    internal static FactoredExclusion Distinctness(int left, int right)
        => new([left, right], null);

    internal static FactoredExclusion ForbiddenTuples(int[] parameterIndices, Constant[][] forbiddenTuples)
        => new(parameterIndices, forbiddenTuples);

    internal bool IsSatisfiedBy(Constant[] binding)
    {
        if (_forbiddenTuples is null)
            return !ReferenceEquals(binding[_parameterIndices[0]], binding[_parameterIndices[1]]);

        var probe = new Constant[_parameterIndices.Length];
        for (int i = 0; i < probe.Length; i++)
            probe[i] = binding[_parameterIndices[i]];

        return !_forbiddenTuples.Contains(probe);
    }

    private sealed class TupleComparer : IEqualityComparer<Constant[]>
    {
        public static TupleComparer Instance { get; } = new();

        public bool Equals(Constant[]? left, Constant[]? right)
        {
            if (left is null || right is null) return ReferenceEquals(left, right);
            if (left.Length != right.Length) return false;
            for (int i = 0; i < left.Length; i++)
            {
                if (!ReferenceEquals(left[i], right[i])) return false;
            }

            return true;
        }

        public int GetHashCode(Constant[] tuple)
        {
            var hash = new HashCode();
            foreach (Constant constant in tuple)
                hash.Add(System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(constant));
            return hash.ToHashCode();
        }
    }
}
