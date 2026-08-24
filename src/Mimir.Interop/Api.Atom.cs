using System;
using System.Runtime.InteropServices;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    // Atom<T> is a generic; we use the polymorphic accessors below.
    // 0 = unknown, 1 = static, 2 = fluent, 3 = derived
    [UnmanagedCallersOnly(EntryPoint = "mimir_atom_get_predicate_type")]
    public static int AtomGetPredicateType(int handle)
    {
        var atom = ObjectRegistry.GetRaw(handle);
        if (atom is Atom<Static>) return 1;
        if (atom is Atom<Fluent>) return 2;
        if (atom is Atom<Derived>) return 3;
        return 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_atom_get_predicate")]
    public static int AtomGetPredicate(int handle)
    {
        var atom = ObjectRegistry.GetRaw(handle);
        return atom switch
        {
            Atom<Static> a => ObjectRegistry.Store(a.Predicate),
            Atom<Fluent> a => ObjectRegistry.Store(a.Predicate),
            Atom<Derived> a => ObjectRegistry.Store(a.Predicate),
            _ => 0
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_atom_get_argument_count")]
    public static int AtomGetArgumentCount(int handle)
    {
        var atom = ObjectRegistry.GetRaw(handle);
        return atom switch
        {
            Atom<Static> a => a.Arguments.Count,
            Atom<Fluent> a => a.Arguments.Count,
            Atom<Derived> a => a.Arguments.Count,
            _ => 0
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_atom_get_argument")]
    public static int AtomGetArgument(int handle, int index)
    {
        var atom = ObjectRegistry.GetRaw(handle);
        System.Collections.Generic.IReadOnlyList<ITerm>? args = atom switch
        {
            Atom<Static> a => a.Arguments,
            Atom<Fluent> a => a.Arguments,
            Atom<Derived> a => a.Arguments,
            _ => null
        };
        if (args == null || index < 0 || index >= args.Count) return 0;
        return ObjectRegistry.Store(args[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_atom_to_string")]
    public static IntPtr AtomToString(int handle)
    {
        var atom = ObjectRegistry.GetRaw(handle);
        return AllocUtf8(atom?.ToString());
    }
}
