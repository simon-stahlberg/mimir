using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

/// <summary>
/// A set of applicable ground actions of one schema, stored as a candidate set per parameter.
/// Every tuple of the Cartesian product is applicable except those ruled out by <see cref="Exclusions"/>.
/// </summary>
public sealed class FactoredAction
{
    private readonly ActionGroundingPlan _plan;
    private readonly Problem _problem;
    private readonly Constant[][] _candidates;
    private readonly bool[] _witnessParameters;

    public ActionSchema Schema => _plan.Schema;
    public int ParameterCount => _candidates.Length;
    public IReadOnlyList<FactoredExclusion> Exclusions { get; }

    internal FactoredAction(
        ActionGroundingPlan plan,
        Problem problem,
        Constant[][] candidates,
        bool[] witnessParameters,
        IReadOnlyList<FactoredExclusion> exclusions)
    {
        _plan = plan;
        _problem = problem;
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
            yield return ActionBuilder.BuildAction(_plan, Array.Empty<Constant>(), _problem);
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
                yield return ActionBuilder.BuildAction(_plan, binding, _problem);

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
    private readonly Constant[][]? _forbiddenTuples;

    public IReadOnlyList<int> ParameterIndices => _parameterIndices;

    /// <summary>Requires two parameters to take different objects, as `(not (= ?x ?y))` does.</summary>
    public bool IsDistinctness => _forbiddenTuples is null;

    private FactoredExclusion(int[] parameterIndices, Constant[][]? forbiddenTuples)
    {
        _parameterIndices = parameterIndices;
        _forbiddenTuples = forbiddenTuples;
    }

    internal static FactoredExclusion Distinctness(int left, int right)
        => new([left, right], null);

    internal static FactoredExclusion ForbiddenTuples(int[] parameterIndices, Constant[][] forbiddenTuples)
        => new(parameterIndices, forbiddenTuples);

    internal bool IsSatisfiedBy(Constant[] binding)
    {
        if (_forbiddenTuples is null)
            return !ReferenceEquals(binding[_parameterIndices[0]], binding[_parameterIndices[1]]);

        foreach (Constant[] forbidden in _forbiddenTuples)
        {
            bool matches = true;
            for (int i = 0; i < _parameterIndices.Length; i++)
            {
                if (!ReferenceEquals(binding[_parameterIndices[i]], forbidden[i]))
                {
                    matches = false;
                    break;
                }
            }

            if (matches) return false;
        }

        return true;
    }
}
