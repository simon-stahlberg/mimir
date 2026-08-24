using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    internal static ExtendedState? ReadExtendedState(int handle)
        => ObjectRegistry.Get<ExtendedState>(handle);

    internal static int StoreExtendedState(ExtendedState extendedState)
    {
        ArgumentNullException.ThrowIfNull(extendedState);
        return ObjectRegistry.Store(extendedState);
    }

    internal static int ExpandAndStoreState(State state)
    {
        ArgumentNullException.ThrowIfNull(state);
        return StoreExtendedState(state.Expand());
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_get_fluent_atom_count")]
    public static int StateGetFluentAtomCount(int handle)
        => ReadValue(handle, -1, (ExtendedState state) => BitboardOps.PopCount(state.State.Bitboard));

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_get_fluent_atom")]
    public static int StateGetFluentAtom(int handle, int index)
    {
        ExtendedState? extendedState = ReadExtendedState(handle);
        State? s = extendedState?.State;
        if (s == null) return 0;
        int localId = NthSetBit(s.Bitboard, index);
        var fluentIndex = new FluentIndex(localId);
        if (!s.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(s.Context.GetFact(fluentIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_get_derived_atom_count")]
    public static int StateGetDerivedAtomCount(int handle)
        => ReadValue(
            handle,
            -1,
            (ExtendedState state) => state.GetTrueDerivedFacts().Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_get_derived_atom")]
    public static int StateGetDerivedAtom(int handle, int index)
    {
        ExtendedState? state = ReadExtendedState(handle);
        if (state is null) return 0;
        IReadOnlyList<Fact<Derived>> facts = state.GetTrueDerivedFacts();
        if (index < 0 || index >= facts.Count) return 0;
        return ObjectRegistry.Store(facts[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_contains_fluent")]
    public static int StateContainsFluent(int stateHandle, int factHandle)
    {
        ExtendedState? s = ReadExtendedState(stateHandle);
        var f = ObjectRegistry.Get<Fact<Fluent>>(factHandle);
        if (s == null || f == null) return InvalidBoolean;
        return EvaluateRelationship(() => s.IsTrue(f));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_contains_static")]
    public static int StateContainsStatic(int stateHandle, int factHandle)
    {
        ExtendedState? s = ReadExtendedState(stateHandle);
        var f = ObjectRegistry.Get<Fact<Static>>(factHandle);
        if (s == null || f == null) return InvalidBoolean;
        return EvaluateRelationship(() => s.IsTrue(f));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_contains_derived")]
    public static int StateContainsDerived(int stateHandle, int factHandle)
    {
        ExtendedState? s = ReadExtendedState(stateHandle);
        var f = ObjectRegistry.Get<Fact<Derived>>(factHandle);
        if (s == null || f == null) return InvalidBoolean;
        return EvaluateRelationship(() => s.IsTrue(f));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_to_string")]
    public static IntPtr StateToString(int handle)
    {
        return ReadValue(handle, IntPtr.Zero, (ExtendedState state) =>
        {
            IEnumerable<string> staticFacts = state.State.Context.Problem
                ._initialStaticFacts
                .Select(fact => fact.ToString());
            IEnumerable<string> fluentFacts = state.State
                .GetTrueFacts()
                .Select(fact => fact.ToString());
            IEnumerable<string> derivedFacts = state
                .GetTrueDerivedFacts()
                .Select(fact => fact.ToString());
            IEnumerable<string> facts = staticFacts
                .Concat(fluentFacts)
                .Concat(derivedFacts);
            return AllocUtf8("[" + string.Join(", ", facts) + "]");
        });
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_generate_applicable_actions")]
    public static int StateGenerateApplicableActions(int stateHandle, int problemHandle)
    {
        ExtendedState? s = ReadExtendedState(stateHandle);
        var p = ObjectRegistry.Get<Problem>(problemHandle);
        if (s == null || p == null) return 0;
        return CreateHandle(() =>
            new List<GroundAction>(p.GetApplicableActionGenerator(s.State).GetApplicableActions(s)));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_list_count")]
    public static int ActionListCount(int handle)
        => ReadValue(handle, -1, (List<GroundAction> actions) => actions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_list_get")]
    public static int ActionListGet(int handle, int index)
    {
        var list = ObjectRegistry.Get<List<GroundAction>>(handle);
        if (list == null || index < 0 || index >= list.Count) return 0;
        return ObjectRegistry.Store(list[index]);
    }
}
