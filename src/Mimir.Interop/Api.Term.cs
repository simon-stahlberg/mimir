using System;
using System.Runtime.InteropServices;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    // -- Constants / objects --

    [UnmanagedCallersOnly(EntryPoint = "mimir_object_get_name")]
    public static IntPtr ObjectGetName(int handle)
    {
        var c = ObjectRegistry.Get<Constant>(handle);
        return AllocUtf8(c?.Name);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_object_get_type")]
    public static IntPtr ObjectGetType(int handle)
    {
        var c = ObjectRegistry.Get<Constant>(handle);
        return AllocUtf8(c?.Type);
    }

    // -- Variables --

    [UnmanagedCallersOnly(EntryPoint = "mimir_variable_get_name")]
    public static IntPtr VariableGetName(int handle)
    {
        var v = ObjectRegistry.Get<Variable>(handle);
        return AllocUtf8(v?.Name);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_variable_get_type")]
    public static IntPtr VariableGetType(int handle)
    {
        var v = ObjectRegistry.Get<Variable>(handle);
        return AllocUtf8(v?.Type);
    }

    // -- Generic ITerm helpers (used when iterating Atom.Arguments which is IReadOnlyList<ITerm>) --

    // 0 = unknown, 1 = constant (Object), 2 = variable
    [UnmanagedCallersOnly(EntryPoint = "mimir_term_get_kind")]
    public static int TermGetKind(int handle)
    {
        var term = ObjectRegistry.GetRaw(handle);
        if (term is Constant) return 1;
        if (term is Variable) return 2;
        return 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_term_get_name")]
    public static IntPtr TermGetName(int handle)
    {
        var term = ObjectRegistry.GetRaw(handle) as ITerm;
        return AllocUtf8(term?.Name);
    }
}
