using System;
using System.Runtime.InteropServices;
using Mimir.Core.Schemas;
using Predicate = Mimir.Core.Schemas.Predicate;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_get_name")]
    public static IntPtr PredicateGetName(int handle)
    {
        var pred = ObjectRegistry.Get<Predicate>(handle);
        return AllocUtf8(pred?.Name);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_get_arity")]
    public static int PredicateGetArity(int handle)
        => ReadValue(handle, -1, (Predicate predicate) => predicate.Parameters.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_get_parameter_count")]
    public static int PredicateGetParameterCount(int handle)
        => ReadValue(handle, -1, (Predicate predicate) => predicate.Parameters.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_get_parameter")]
    public static int PredicateGetParameter(int handle, int index)
    {
        var pred = ObjectRegistry.Get<Predicate>(handle);
        if (pred == null || index < 0 || index >= pred.Parameters.Count) return 0;
        return ObjectRegistry.Store(pred.Parameters[index]);
    }

    // 0 = unknown, 1 = static, 2 = fluent, 3 = derived
    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_get_predicate_type")]
    public static int PredicateGetPredicateType(int handle)
    {
        var pred = ObjectRegistry.GetRaw(handle);
        if (pred is StaticPredicate) return 1;
        if (pred is FluentPredicate) return 2;
        if (pred is DerivedPredicate) return 3;
        return 0;
    }
}
