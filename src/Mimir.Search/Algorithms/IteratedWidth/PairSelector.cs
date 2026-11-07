using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Algorithms.IteratedWidth;

/// <summary>
/// Emits, for one factored group, the members IW(2) would keep.
/// <para>
/// Members of an add-class — one combination of the positive effects' parameters — add the same atoms. Their
/// successors differ only in what they delete, so the pairs they can claim split into a part shared by the whole
/// class and a part whose partner some sibling deletes. One member claims the shared part; every remaining pair
/// is claimed by a member that spares its partner, and sparing an atom means giving one parameter a different
/// value. The members that can claim anything are therefore bounded by the number of delete literals rather than
/// by the size of the product.
/// </para>
/// </summary>
internal sealed class PairSelector
{
    private readonly InstanceContext _context;
    private readonly FactoredAction _group;
    private readonly SchemaEffectModel _model;
    private readonly NoveltyTracker _tracker;
    private readonly int[] _parentAtoms;
    private readonly FluentIndex[] _unsatisfiedGoals;
    private readonly FluentIndex[] _goalAtoms;

    private readonly Constant?[] _binding;
    private readonly Constant?[] _scratch;
    private readonly List<FluentIndex> _added = new();

    // An added atom with no registered fact has never been true, so it and every pair containing it are new.
    private bool _addsUnregisteredAtom;

    /// <summary>Atoms some member of the current class deletes, mapped to the parameter whose value produced it.</summary>
    private readonly Dictionary<FluentIndex, int> _deletable = new();

    // Reused across classes and members; a group can have thousands of each.
    private readonly HashSet<FluentIndex> _wanted = new();
    private readonly HashSet<FluentIndex> _deleted = new();

    internal PairSelector(
        InstanceContext context,
        FactoredAction group,
        SchemaEffectModel model,
        NoveltyTracker tracker,
        int[] parentAtoms,
        FluentIndex[] unsatisfiedGoals,
        FluentIndex[] goalAtoms)
    {
        _context = context;
        _group = group;
        _model = model;
        _tracker = tracker;
        _parentAtoms = parentAtoms;
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

        var cursor = new int[addParameters.Length];
        foreach (int parameter in addParameters)
        {
            if (_group.GetCandidates(parameter).Length == 0) yield break;
        }

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
        CollectDeletableAtoms();

        // A brand new atom makes every pair containing it new too, which saves scanning the shared pairs.
        bool fresh = HasUnseenAddedAtom();

        HashSet<FluentIndex> wanted = _wanted;
        wanted.Clear();
        foreach (FluentIndex partner in _deletable.Keys)
        {
            if (fresh || HasUnseenPairWith(partner)) wanted.Add(partner);
        }

        // Losing a goal atom the parent holds would hide a goal state, so a member sparing each must be emitted.
        if (ReachesGoal())
        {
            foreach (FluentIndex goalAtom in _goalAtoms)
            {
                if (_deletable.ContainsKey(goalAtom)) wanted.Add(goalAtom);
            }
        }

        if (!fresh && wanted.Count == 0 && !HasUnseenSharedPair()) yield break;

        Action? member = _group.TryCompleteMember(_binding);
        if (member is null) yield break;
        yield return member;

        // A member claims every wanted partner it does not delete, so only what it deleted can remain.
        wanted.IntersectWith(DeletedBy(member));
        while (wanted.Count > 0)
        {
            FluentIndex target = wanted.First();
            Action? sparing = BuildSparingMember(target);
            if (sparing is null)
            {
                wanted.Remove(target);
                continue;
            }

            yield return sparing;
            wanted.IntersectWith(DeletedBy(sparing));
        }
    }

    /// <summary>A member of this class that does not delete <paramref name="target"/>, or null if none exists.</summary>
    private Action? BuildSparingMember(FluentIndex target)
    {
        if (!_deletable.TryGetValue(target, out int parameter) || parameter < 0) return null;

        _binding.CopyTo(_scratch, 0);
        foreach (Constant candidate in _group.GetCandidates(parameter))
        {
            _scratch[parameter] = candidate;
            if (DeletesTarget(parameter, target)) continue;
            if (_group.TryCompleteMember(_scratch) is { } member) return member;
        }

        return null;
    }

    // Only literals mentioning the varied parameter can change, and the class fixes the rest.
    private bool DeletesTarget(int parameter, FluentIndex target)
    {
        foreach (EffectLiteral delete in _model.Deletes)
        {
            if (!delete.Parameters.Contains(parameter)) continue;
            if (!IsFullyBound(delete, _scratch)) continue;
            if (delete.Resolve(_context, _scratch) == target) return true;
        }

        return false;
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

    /// <summary>
    /// A delete literal has at most one parameter outside the add parameters, so enumerating the atoms the class
    /// can delete costs that parameter's candidate set rather than the product.
    /// </summary>
    private void CollectDeletableAtoms()
    {
        _deletable.Clear();
        foreach (EffectLiteral delete in _model.Deletes)
        {
            int free = -1;
            foreach (int parameter in delete.Parameters)
            {
                if (_binding[parameter] is null) free = parameter;
            }

            if (free < 0)
            {
                if (delete.Resolve(_context, _binding) is { } fixedAtom) _deletable[fixedAtom] = -1;
                continue;
            }

            _binding.CopyTo(_scratch, 0);
            foreach (Constant candidate in _group.GetCandidates(free))
            {
                _scratch[free] = candidate;
                if (!IsFullyBound(delete, _scratch)) continue;
                if (delete.Resolve(_context, _scratch) is { } atom) _deletable[atom] = free;
            }
        }
    }

    private bool HasUnseenAddedAtom()
    {
        if (_addsUnregisteredAtom) return true;
        foreach (FluentIndex atom in _added)
        {
            if (!_tracker.IsAtomSeen(atom)) return true;
        }

        return false;
    }

    private bool HasUnseenPairWith(FluentIndex partner)
    {
        foreach (FluentIndex atom in _added)
        {
            if (!_tracker.IsPairSeen(atom, partner)) return true;
        }

        return false;
    }

    /// <summary>Pairs every member of the class shares: added atoms against each other and against what no one deletes.</summary>
    private bool HasUnseenSharedPair()
    {
        for (int i = 0; i < _added.Count; i++)
        {
            for (int j = i + 1; j < _added.Count; j++)
            {
                if (!_tracker.IsPairSeen(_added[i], _added[j])) return true;
            }
        }

        foreach (int atom in _parentAtoms)
        {
            var partner = new FluentIndex(atom);
            if (_deletable.ContainsKey(partner)) continue;
            if (HasUnseenPairWith(partner)) return true;
        }

        return false;
    }

    private bool ReachesGoal()
    {
        if (_unsatisfiedGoals.Length == 0) return false;
        // An unregistered added atom cannot be a goal atom, which is registered when the goal is built.
        foreach (FluentIndex goalAtom in _unsatisfiedGoals)
        {
            if (!_added.Contains(goalAtom)) return false;
        }

        return true;
    }

    private HashSet<FluentIndex> DeletedBy(Action action)
    {
        for (int parameter = 0; parameter < _scratch.Length; parameter++)
            _scratch[parameter] = action.Arguments[parameter];

        _deleted.Clear();
        foreach (EffectLiteral delete in _model.Deletes)
        {
            if (delete.Resolve(_context, _scratch) is { } atom) _deleted.Add(atom);
        }

        return _deleted;
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
