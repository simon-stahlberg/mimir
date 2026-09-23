using System;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    // ====== Lifted literals: Literal<Atom<T>> ======

    // 0 = unknown, 1 = static, 2 = fluent, 3 = derived
    [UnmanagedCallersOnly(EntryPoint = "mimir_literal_get_predicate_type")]
    public static int LiteralGetPredicateType(int handle)
    {
        var lit = ObjectRegistry.GetRaw(handle);
        if (lit is Literal<Atom<Static>>) return 1;
        if (lit is Literal<Atom<Fluent>>) return 2;
        if (lit is Literal<Atom<Derived>>) return 3;
        return 0;
    }

    // 0 = positive, 1 = negative
    [UnmanagedCallersOnly(EntryPoint = "mimir_literal_get_polarity")]
    public static byte LiteralGetPolarity(int handle)
    {
        var lit = ObjectRegistry.GetRaw(handle);
        Polarity polarity = lit switch
        {
            Literal<Atom<Static>> l => l.Polarity,
            Literal<Atom<Fluent>> l => l.Polarity,
            Literal<Atom<Derived>> l => l.Polarity,
            _ => Polarity.Positive
        };
        return polarity == Polarity.Positive ? (byte)0 : (byte)1;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_literal_get_atom")]
    public static int LiteralGetAtom(int handle)
    {
        var lit = ObjectRegistry.GetRaw(handle);
        return lit switch
        {
            Literal<Atom<Static>> l => ObjectRegistry.Store(l.Value),
            Literal<Atom<Fluent>> l => ObjectRegistry.Store(l.Value),
            Literal<Atom<Derived>> l => ObjectRegistry.Store(l.Value),
            _ => 0
        };
    }

    // ====== Ground literals: Literal<Fact<T>> or Literal<Fact> ======

    // 0 = unknown, 1 = static, 2 = fluent, 3 = derived
    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_literal_get_predicate_type")]
    public static int GroundLiteralGetPredicateType(int handle)
    {
        var lit = ObjectRegistry.GetRaw(handle);
        if (lit is Literal<Fact<Static>>) return 1;
        if (lit is Literal<Fact<Fluent>>) return 2;
        if (lit is Literal<Fact<Derived>>) return 3;
        if (lit is Literal<Fact> lf)
        {
            return lf.Value switch
            {
                Fact<Static> => 1,
                Fact<Fluent> => 2,
                Fact<Derived> => 3,
                _ => 0
            };
        }
        return 0;
    }

    // 0 = positive, 1 = negative
    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_literal_get_polarity")]
    public static byte GroundLiteralGetPolarity(int handle)
    {
        var lit = ObjectRegistry.GetRaw(handle);
        Polarity polarity = lit switch
        {
            Literal<Fact<Static>> l => l.Polarity,
            Literal<Fact<Fluent>> l => l.Polarity,
            Literal<Fact<Derived>> l => l.Polarity,
            Literal<Fact> l => l.Polarity,
            _ => Polarity.Positive
        };
        return polarity == Polarity.Positive ? (byte)0 : (byte)1;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_literal_get_atom")]
    public static int GroundLiteralGetAtom(int handle)
    {
        var lit = ObjectRegistry.GetRaw(handle);
        return lit switch
        {
            Literal<Fact<Static>> l => ObjectRegistry.Store(l.Value),
            Literal<Fact<Fluent>> l => ObjectRegistry.Store(l.Value),
            Literal<Fact<Derived>> l => ObjectRegistry.Store(l.Value),
            Literal<Fact> l => ObjectRegistry.Store(l.Value),
            _ => 0
        };
    }
}
