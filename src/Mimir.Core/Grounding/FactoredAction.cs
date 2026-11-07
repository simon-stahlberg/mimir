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

    public ActionSchema Schema => _plan.Schema;
    public int ParameterCount => _candidates.Length;
    public IReadOnlyList<FactoredExclusion> Exclusions { get; }

    internal FactoredAction(
        ActionGroundingPlan plan,
        GroundActionCache actions,
        Constant[][] candidates,
        IReadOnlyList<FactoredExclusion> exclusions)
    {
        _plan = plan;
        _actions = actions;
        _candidates = candidates;
        Exclusions = exclusions;
    }

    /// <summary>The objects this parameter may take. The span is a view over the group and cannot alias it.</summary>
    public ReadOnlySpan<Constant> GetCandidates(int parameterIndex) => _candidates[parameterIndex];

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
        // One scratch row per enumeration keeps exclusion tests allocation-free without sharing state between them.
        var probe = new Constant[ProbeLength()];
        while (true)
        {
            for (int i = 0; i < parameterCount; i++)
                binding[i] = _candidates[i][cursor[i]];

            if (IsAllowed(binding, probe))
                yield return _actions.GetOrBuild(binding);

            int position = parameterCount - 1;
            while (position >= 0 && ++cursor[position] == _candidates[position].Length)
                cursor[position--] = 0;
            if (position < 0) yield break;
        }
    }

    /// <summary>
    /// One member per distinct combination of <paramref name="parameterIndices"/>, with every other parameter
    /// taking an arbitrary admissible value. A consumer that cannot tell two members apart unless they differ on
    /// those parameters can use this instead of <see cref="Expand"/> and never build the rest of the product.
    /// </summary>
    public IEnumerable<Action> ExpandProjection(IReadOnlyList<int> parameterIndices)
    {
        ArgumentNullException.ThrowIfNull(parameterIndices);
        foreach (int parameter in parameterIndices)
        {
            if (parameter < 0 || parameter >= _candidates.Length)
                throw new ArgumentOutOfRangeException(nameof(parameterIndices), parameter, null);
        }

        int parameterCount = _candidates.Length;
        foreach (Constant[] candidates in _candidates)
        {
            if (candidates.Length == 0) yield break;
        }

        var projected = parameterIndices.Distinct().Order().ToArray();
        if (projected.Length == parameterCount)
        {
            foreach (Action action in Expand()) yield return action;
            yield break;
        }

        var partial = new Constant?[parameterCount];
        var cursor = new int[projected.Length];
        while (true)
        {
            Array.Clear(partial);
            for (int i = 0; i < projected.Length; i++)
                partial[projected[i]] = _candidates[projected[i]][cursor[i]];

            if (TryCompleteMember(partial) is { } action)
                yield return action;

            int position = projected.Length - 1;
            while (position >= 0 && ++cursor[position] == _candidates[projected[position]].Length)
                cursor[position--] = 0;
            if (position < 0) yield break;
        }
    }

    /// <summary>
    /// Fills the unassigned slots of <paramref name="partialBinding"/> with objects this group admits, or returns
    /// null when no completion satisfies the exclusions. A null entry is a slot to fill; a non-null one is held
    /// fixed and must be one of that parameter's candidates.
    /// </summary>
    public Action? TryCompleteMember(ReadOnlySpan<Constant?> partialBinding)
    {
        if (partialBinding.Length != _candidates.Length)
            throw new ArgumentException("The partial binding must have one entry per parameter.", nameof(partialBinding));

        var binding = new Constant[_candidates.Length];
        var free = new List<int>();
        for (int parameter = 0; parameter < _candidates.Length; parameter++)
        {
            if (partialBinding[parameter] is { } value)
            {
                if (Array.IndexOf(_candidates[parameter], value) < 0) return null;
                binding[parameter] = value;
                continue;
            }

            if (_candidates[parameter].Length == 0) return null;
            free.Add(parameter);
        }

        var probe = new Constant[ProbeLength()];
        if (free.Count == 0)
            return IsAllowed(binding, probe) ? _actions.GetOrBuild(binding) : null;

        // Exclusions can rule out the obvious completion, so walk the free sub-product until one is admissible.
        var cursor = new int[free.Count];
        while (true)
        {
            for (int i = 0; i < free.Count; i++)
                binding[free[i]] = _candidates[free[i]][cursor[i]];

            if (IsAllowed(binding, probe)) return _actions.GetOrBuild(binding);

            int position = free.Count - 1;
            while (position >= 0 && ++cursor[position] == _candidates[free[position]].Length)
                cursor[position--] = 0;
            if (position < 0) return null;
        }
    }

    private int ProbeLength()
    {
        int length = 0;
        for (int i = 0; i < Exclusions.Count; i++)
            length = Math.Max(length, Exclusions[i].ParameterIndices.Length);
        return length;
    }

    private bool IsAllowed(Constant[] binding, Constant[] probe)
    {
        for (int i = 0; i < Exclusions.Count; i++)
        {
            if (!Exclusions[i].IsSatisfiedBy(binding, probe))
                return false;
        }

        return true;
    }

    public override string ToString()
    {
        IEnumerable<string> parameters = _candidates.Select(Describe);
        return $"({Schema.Name} {string.Join(" ", parameters)})";

        // A group can hold hundreds of thousands of objects, which no debugger view wants spelled out.
        static string Describe(Constant[] candidates)
        {
            if (candidates.Length == 1) return candidates[0].Name;
            IEnumerable<string> names = candidates.Take(4).Select(candidate => candidate.Name);
            string shown = string.Join(",", names);
            return candidates.Length <= 4
                ? $"{{{shown}}}"
                : $"{{{shown},... {candidates.Length} total}}";
        }
    }
}

/// <summary>
/// A precondition a Cartesian product cannot express on its own. Carrying it alongside the candidate sets lets a
/// negative precondition rule out a few combinations instead of coupling two otherwise independent parameters.
/// </summary>
public sealed class FactoredExclusion
{
    private readonly int[] _parameterIndices;
    private readonly Constant[]? _forbidden;
    private readonly HashSet<Constant[]>? _lookup;
    private IReadOnlyList<IReadOnlyList<Constant>>? _forbiddenView;

    /// <summary>The parameters this exclusion constrains, as indices into the schema's parameter list.</summary>
    public ReadOnlySpan<int> ParameterIndices => _parameterIndices;

    /// <summary>
    /// True when the exclusion only requires its two parameters to differ, as `(not (= ?x ?y))` does. The
    /// combinations it rules out are the diagonal, so <see cref="ForbiddenTuples"/> is empty.
    /// </summary>
    public bool IsDistinctness => _forbidden is null;

    /// <summary>The number of combinations ruled out, or the size of the diagonal for a distinctness exclusion.</summary>
    public int ForbiddenTupleCount
        => _forbidden is null ? 0 : _forbidden.Length / _parameterIndices.Length;

    /// <summary>
    /// The combinations ruled out, each aligned with <see cref="ParameterIndices"/>. Empty for a distinctness
    /// exclusion. Materialized on first access, so reading it is only worth it to interpret the group.
    /// </summary>
    public IReadOnlyList<IReadOnlyList<Constant>> ForbiddenTuples
        => _forbiddenView ??= BuildForbiddenView();

    private FactoredExclusion(int[] parameterIndices, Constant[]? forbidden)
    {
        _parameterIndices = parameterIndices;
        _forbidden = forbidden;
        if (forbidden is null) return;

        _lookup = new HashSet<Constant[]>(TupleComparer.Instance);
        for (int offset = 0; offset < forbidden.Length; offset += parameterIndices.Length)
            _lookup.Add(forbidden[offset..(offset + parameterIndices.Length)]);
    }

    internal static FactoredExclusion Distinctness(int left, int right)
        => new([left, right], null);

    /// <summary>`forbidden` holds the ruled-out combinations row-major, one row per `parameterIndices.Length`.</summary>
    internal static FactoredExclusion ForbiddenCombinations(int[] parameterIndices, Constant[] forbidden)
        => new(parameterIndices, forbidden);

    internal bool IsSatisfiedBy(Constant[] binding, Constant[] probe)
    {
        if (_lookup is null)
            return !ReferenceEquals(binding[_parameterIndices[0]], binding[_parameterIndices[1]]);

        for (int i = 0; i < _parameterIndices.Length; i++)
            probe[i] = binding[_parameterIndices[i]];

        return !_lookup.Contains(probe);
    }

    private IReadOnlyList<IReadOnlyList<Constant>> BuildForbiddenView()
    {
        if (_forbidden is null) return Array.Empty<IReadOnlyList<Constant>>();

        int arity = _parameterIndices.Length;
        var rows = new IReadOnlyList<Constant>[_forbidden.Length / arity];
        for (int row = 0; row < rows.Length; row++)
            rows[row] = Array.AsReadOnly(_forbidden[(row * arity)..((row + 1) * arity)]);

        return Array.AsReadOnly(rows);
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
