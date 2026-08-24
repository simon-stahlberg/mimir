using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_schema")]
    public static int GroundActionGetSchema(int handle)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        return ObjectRegistry.Store(a.Schema);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_cost")]
    public static double GroundActionGetCost(int handle)
        => ReadValue(handle, double.NaN, (GroundAction action) => action.Cost);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_argument_count")]
    public static int GroundActionGetArgumentCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => action.Arguments.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_argument")]
    public static int GroundActionGetArgument(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null || index < 0 || index >= a.Arguments.Count) return 0;
        return ObjectRegistry.Store(a.Arguments[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_to_string")]
    public static IntPtr GroundActionToString(int handle)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        return AllocUtf8(a?.ToString());
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_is_applicable")]
    public static int GroundActionIsApplicable(int actionHandle, int stateHandle)
    {
        var action = ObjectRegistry.Get<GroundAction>(actionHandle);
        ExtendedState? state = ReadExtendedState(stateHandle);
        if (action == null || state == null) return InvalidBoolean;
        return EvaluateRelationship(() => action.IsApplicable(state));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_apply")]
    public static int GroundActionApply(int actionHandle, int stateHandle)
    {
        var action = ObjectRegistry.Get<GroundAction>(actionHandle);
        ExtendedState? state = ReadExtendedState(stateHandle);
        if (action == null || state == null) return 0;
        return CreateHandle(() => state.Apply(action).Expand());
    }

    // -- Add / delete effects (decoded from bitboards) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_add_effect_count")]
    public static int GroundActionGetAddEffectCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => PopCount(action.AddEffects));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_add_effect")]
    public static int GroundActionGetAddEffect(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        int localId = NthSetBit(a.AddEffects, index);
        var fluentIndex = new FluentIndex(localId);
        if (!a.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(a.Context.GetFact(fluentIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_delete_effect_count")]
    public static int GroundActionGetDeleteEffectCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => PopCount(action.DeleteEffects));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_delete_effect")]
    public static int GroundActionGetDeleteEffect(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        int localId = NthSetBit(a.DeleteEffects, index);
        var fluentIndex = new FluentIndex(localId);
        if (!a.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(a.Context.GetFact(fluentIndex));
    }

    // -- Preconditions (decoded from bitboards on the action) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_positive_fluent_precond_count")]
    public static int GroundActionGetPositiveFluentPrecondCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => PopCount(action.PositiveFluentPreconditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_positive_fluent_precond")]
    public static int GroundActionGetPositiveFluentPrecond(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        int localId = NthSetBit(a.PositiveFluentPreconditions, index);
        var fluentIndex = new FluentIndex(localId);
        if (!a.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(a.Context.GetFact(fluentIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_negative_fluent_precond_count")]
    public static int GroundActionGetNegativeFluentPrecondCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => PopCount(action.NegativeFluentPreconditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_negative_fluent_precond")]
    public static int GroundActionGetNegativeFluentPrecond(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        int localId = NthSetBit(a.NegativeFluentPreconditions, index);
        var fluentIndex = new FluentIndex(localId);
        if (!a.Context.IsValid(fluentIndex)) return 0;
        return ObjectRegistry.Store(a.Context.GetFact(fluentIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_positive_static_precond_count")]
    public static int GroundActionGetPositiveStaticPrecondCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => PopCount(action.PositiveStaticPreconditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_positive_static_precond")]
    public static int GroundActionGetPositiveStaticPrecond(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        int localId = NthSetBit(a.PositiveStaticPreconditions, index);
        var staticIndex = new StaticIndex(localId);
        if (!a.Context.IsValid(staticIndex)) return 0;
        return ObjectRegistry.Store(a.Context.GetFact(staticIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_negative_static_precond_count")]
    public static int GroundActionGetNegativeStaticPrecondCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => PopCount(action.NegativeStaticPreconditions));

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_negative_static_precond")]
    public static int GroundActionGetNegativeStaticPrecond(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null) return 0;
        int localId = NthSetBit(a.NegativeStaticPreconditions, index);
        var staticIndex = new StaticIndex(localId);
        if (!a.Context.IsValid(staticIndex)) return 0;
        return ObjectRegistry.Store(a.Context.GetFact(staticIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_derived_precondition_count")]
    public static int GroundActionGetDerivedPreconditionCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => action.DerivedPreconditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_derived_precondition")]
    public static int GroundActionGetDerivedPrecondition(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null || index < 0 || index >= a.DerivedPreconditions.Count) return 0;
        return ObjectRegistry.Store(a.DerivedPreconditions[index]);
    }

    // -- Conditional effects --

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_conditional_effect_count")]
    public static int GroundActionGetConditionalEffectCount(int handle)
        => ReadValue(handle, -1, (GroundAction action) => action.ConditionalEffects.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_ground_action_get_conditional_effect")]
    public static int GroundActionGetConditionalEffect(int handle, int index)
    {
        var a = ObjectRegistry.Get<GroundAction>(handle);
        if (a == null || index < 0 || index >= a.ConditionalEffects.Count) return 0;
        return ObjectRegistry.Store(a.ConditionalEffects[index]);
    }

    // PopCount / NthSetBit live in Mimir.Core.Grounding.BitboardOps. Forwarders
    // keep this file's call-sites short.
    private static int PopCount(ulong[] bits) => BitboardOps.PopCount(bits);
    private static int PopCount(OffsetBitboard bits) => BitboardOps.PopCount(bits);
    private static int NthSetBit(ReadOnlySpan<ulong> bits, int index) => BitboardOps.NthSetBitIndex(bits, index);
    private static int NthSetBit(OffsetBitboard bits, int index) => BitboardOps.NthSetBitIndex(bits, index);
}
