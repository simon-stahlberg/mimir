using System;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_effect")]
    public static int GroundConditionalEffectGetEffect(int handle)
    {
        var ce = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        if (ce == null) return 0;
        return ObjectRegistry.Store(ce.EffectLiteral);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_positive_fluent_count")]
    public static int GceGetPositiveFluentCount(int handle)
        => ReadValue(handle, -1, (GroundConditionalEffect effect) => PopCount(effect.PositiveFluentConditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_positive_fluent")]
    public static int GceGetPositiveFluent(int handle, int index)
    {
        var ce = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        if (ce == null) return 0;
        int localId = NthSetBit(ce.PositiveFluentConditions, index);
        var fluentIndex = new FluentIndex(localId);
        if (!ce.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(ce.Context.GetFact(fluentIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_negative_fluent_count")]
    public static int GceGetNegativeFluentCount(int handle)
        => ReadValue(handle, -1, (GroundConditionalEffect effect) => PopCount(effect.NegativeFluentConditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_negative_fluent")]
    public static int GceGetNegativeFluent(int handle, int index)
    {
        var ce = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        if (ce == null) return 0;
        int localId = NthSetBit(ce.NegativeFluentConditions, index);
        var fluentIndex = new FluentIndex(localId);
        if (!ce.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(ce.Context.GetFact(fluentIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_positive_static_count")]
    public static int GceGetPositiveStaticCount(int handle)
        => ReadValue(handle, -1, (GroundConditionalEffect effect) => PopCount(effect.PositiveStaticConditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_positive_static")]
    public static int GceGetPositiveStatic(int handle, int index)
    {
        var ce = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        if (ce == null) return 0;
        int localId = NthSetBit(ce.PositiveStaticConditions, index);
        var staticIndex = new StaticIndex(localId);
        if (!ce.Context.IsValid(staticIndex)) return 0;
        return ObjectRegistry.Store(ce.Context.GetFact(staticIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_negative_static_count")]
    public static int GceGetNegativeStaticCount(int handle)
        => ReadValue(handle, -1, (GroundConditionalEffect effect) => PopCount(effect.NegativeStaticConditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_negative_static")]
    public static int GceGetNegativeStatic(int handle, int index)
    {
        var ce = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        if (ce == null) return 0;
        int localId = NthSetBit(ce.NegativeStaticConditions, index);
        var staticIndex = new StaticIndex(localId);
        if (!ce.Context.IsValid(staticIndex)) return 0;
        return ObjectRegistry.Store(ce.Context.GetFact(staticIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_derived_count")]
    public static int GceGetDerivedCount(int handle)
        => ReadValue(handle, -1, (GroundConditionalEffect effect) => effect.DerivedConditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_get_derived")]
    public static int GceGetDerived(int handle, int index)
    {
        var ce = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        if (ce == null || index < 0 || index >= ce.DerivedConditions.Count) return 0;
        return ObjectRegistry.Store(ce.DerivedConditions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_conditional_effect_is_satisfied")]
    public static int GceIsSatisfied(int handle, int stateHandle)
    {
        var effect = ObjectRegistry.Get<GroundConditionalEffect>(handle);
        ExtendedState? state = ReadExtendedState(stateHandle);
        if (effect == null || state == null) return InvalidBoolean;
        return EvaluateRelationship(() => effect.IsSatisfied(state));
    }
}
