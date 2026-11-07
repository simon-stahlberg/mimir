using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.IteratedWidth;

/// <summary>
/// Emits, for one factored group, the members IW(2) would keep.
/// <para>
/// Members of an add-class — one combination of the positive effects' parameters — add the same atoms, so their
/// successors differ only in what they delete. One member claims every pair except those whose partner it
/// deletes, and a delete list is short, so what remains after it is a handful of partners rather than a share of
/// the product. Each is claimed by a member that spares it, which means giving one of the parameters that
/// produced it a different value.
/// </para>
/// <para>
/// Nothing here enumerates the atoms the class could delete. The partners still worth claiming after a member is
/// emitted are exactly the ones that member deleted, read straight off its own effects, so a delete literal may
/// mention any number of parameters.
/// </para>
/// </summary>
internal sealed class PairSelector
{
    private readonly InstanceContext _context;
    private readonly FactoredAction _group;
    private readonly SchemaEffectModel _model;
    private readonly NoveltyTracker _tracker;
    private readonly State _parent;
    private readonly FluentIndex[] _unsatisfiedGoals;
    private readonly FluentIndex[] _goalAtoms;

    private readonly Constant?[] _binding;
    private readonly Constant?[] _scratch;
    private readonly List<FluentIndex> _added = new();
    private readonly List<Deletion> _deletions = new();
    private readonly HashSet<FluentIndex> _spared = new();

    // An added atom with no registered fact has never been true, so it and every pair containing it are new.
    private bool _addsUnregisteredAtom;

    private readonly record struct Deletion(FluentIndex Atom, EffectLiteral Literal);

    internal PairSelector(
        InstanceContext context,
        FactoredAction group,
        SchemaEffectModel model,
        NoveltyTracker tracker,
        State parent,
        FluentIndex[] unsatisfiedGoals,
        FluentIndex[] goalAtoms)
    {
        _context = context;
        _group = group;
        _model = model;
        _tracker = tracker;
        _parent = parent;
        _unsatisfiedGoals = unsatisfiedGoals;
        _goalAtoms = goalAtoms;
        _binding = new Constant?[group.ParameterCount];
        _scratch = new Constant?[group.ParameterCount];
    }

    internal IEnumerable<Action> Select()
    {
        int[] addParameters = _model.AddParameters;
        if (addParameters.Length == 0)
        {
            Array.Clear(_binding);
            foreach (Action action in SelectClass()) yield return action;
            yield break;
        }

        foreach (int parameter in addParameters)
        {
            if (_group.GetCandidates(parameter).Length == 0) yield break;
        }

        var cursor = new int[addParameters.Length];
        while (true)
        {
            Array.Clear(_binding);
            for (int i = 0; i < addParameters.Length; i++)
                _binding[addParameters[i]] = _group.GetCandidates(addParameters[i])[cursor[i]];

            foreach (Action action in SelectClass()) yield return action;

            int position = addParameters.Length - 1;
            while (position >= 0
                   && ++cursor[position] == _group.GetCandidates(addParameters[position]).Length)
            {
                cursor[position--] = 0;
            }

            if (position < 0) yield break;
        }
    }

    /// <summary>Emits the members of the add-class currently fixed in <see cref="_binding"/>.</summary>
    private IEnumerable<Action> SelectClass()
    {
        ResolveAddedAtoms();
        if (!CanClaimAnything()) yield break;

        Action? member = _group.TryCompleteMember(_binding);
        if (member is null) yield break;
        yield return member;

        // Deleting a partner is the only way a member can leave its pair unclaimed, so the next member to try is
        // one that spares it. A goal atom is tracked the same way: losing it would hide a goal state.
        bool wantsGoalAtoms = ReachesGoal();
        _spared.Clear();
        while (true)
        {
            ResolveDeletions(member);
            Deletion? target = FindTargetToSpare(wantsGoalAtoms);
            if (target is null) yield break;

            _spared.Add(target.Value.Atom);
            Action? sparing = BuildSparingMember(target.Value);
            if (sparing is null) yield break;

            yield return sparing;
            member = sparing;
        }
    }

    /// <summary>
    /// Whether any member of the class could introduce a tuple. Deleting an atom only removes partners, so if
    /// nothing pairs anew with the parent then nothing pairs anew with any successor either.
    /// </summary>
    private bool CanClaimAnything()
    {
        if (_addsUnregisteredAtom) return true;
        foreach (FluentIndex atom in _added)
        {
            if (!_tracker.IsAtomSeen(atom)) return true;
            if (_tracker.HasUnseenPairWith(atom, _parent)) return true;
        }

        return false;
    }

    /// <summary>The first atom the last member deleted whose pair is still worth claiming, if any.</summary>
    private Deletion? FindTargetToSpare(bool wantsGoalAtoms)
    {
        foreach (Deletion deletion in _deletions)
        {
            // Sparing the same atom twice would loop; one member sparing it has already claimed its pairs.
            if (_spared.Contains(deletion.Atom)) continue;
            if (wantsGoalAtoms && Array.IndexOf(_goalAtoms, deletion.Atom) >= 0) return deletion;
            if (_addsUnregisteredAtom) return deletion;

            foreach (FluentIndex atom in _added)
            {
                if (!_tracker.IsPairSeen(atom, deletion.Atom)) return deletion;
            }
        }

        return null;
    }

    /// <summary>
    /// A member of this class that does not delete the target. Only the literal that produced it can produce it
    /// again, so it is enough to give one of that literal's free parameters a different value.
    /// </summary>
    private Action? BuildSparingMember(Deletion target)
    {
        foreach (int parameter in target.Literal.Parameters)
        {
            if (_binding[parameter] is not null) continue;

            _binding.CopyTo(_scratch, 0);
            foreach (Constant candidate in _group.GetCandidates(parameter))
            {
                _scratch[parameter] = candidate;
                if (!IsFullyBound(target.Literal, _scratch)) continue;
                if (target.Literal.Resolve(_context, _scratch) == target.Atom) continue;
                if (_group.TryCompleteMember(_scratch) is { } member) return member;
            }
        }

        return null;
    }

    private void ResolveAddedAtoms()
    {
        _added.Clear();
        _addsUnregisteredAtom = false;
        foreach (EffectLiteral add in _model.Adds)
        {
            if (add.Resolve(_context, _binding) is { } atom) _added.Add(atom);
            else _addsUnregisteredAtom = true;
        }
    }

    private void ResolveDeletions(Action member)
    {
        for (int parameter = 0; parameter < _scratch.Length; parameter++)
            _scratch[parameter] = member.Arguments[parameter];

        _deletions.Clear();
        foreach (EffectLiteral delete in _model.Deletes)
        {
            if (delete.Resolve(_context, _scratch) is { } atom) _deletions.Add(new Deletion(atom, delete));
        }
    }

    private bool ReachesGoal()
    {
        if (_unsatisfiedGoals.Length == 0) return false;
        foreach (FluentIndex goalAtom in _unsatisfiedGoals)
        {
            if (!_added.Contains(goalAtom)) return false;
        }

        return true;
    }

    private static bool IsFullyBound(EffectLiteral literal, Constant?[] binding)
    {
        foreach (int parameter in literal.Parameters)
        {
            if (binding[parameter] is null) return false;
        }

        return true;
    }
}
