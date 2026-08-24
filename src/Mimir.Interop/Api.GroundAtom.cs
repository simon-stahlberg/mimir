using System;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    // 0 = unknown, 1 = static, 2 = fluent, 3 = derived
    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_atom_get_predicate_type")]
    public static int GroundAtomGetPredicateType(int handle)
    {
        var fact = ObjectRegistry.GetRaw(handle);
        if (fact is Fact<Static>) return 1;
        if (fact is Fact<Fluent>) return 2;
        if (fact is Fact<Derived>) return 3;
        return 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_atom_get_predicate")]
    public static int GroundAtomGetPredicate(int handle)
    {
        var fact = ObjectRegistry.Get<Fact>(handle);
        if (fact == null) return 0;
        return ObjectRegistry.Store(fact.Predicate);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_atom_get_argument_count")]
    public static int GroundAtomGetArgumentCount(int handle)
        => ReadValue(handle, -1, (Fact fact) => fact.Arguments.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_atom_get_argument")]
    public static int GroundAtomGetArgument(int handle, int index)
    {
        var fact = ObjectRegistry.Get<Fact>(handle);
        if (fact == null || index < 0 || index >= fact.Arguments.Count) return 0;
        return ObjectRegistry.Store(fact.Arguments[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_atom_to_string")]
    public static IntPtr GroundAtomToString(int handle)
    {
        var fact = ObjectRegistry.Get<Fact>(handle);
        return AllocUtf8(fact?.ToString());
    }
}
