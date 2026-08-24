using System.Runtime.InteropServices;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_name")]
    public static IntPtr ActionGetName(int handle)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        return AllocUtf8(a?.Name);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_arity")]
    public static int ActionGetArity(int handle)
        => ReadValue(handle, -1, (ActionSchema action) => action.Parameters.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_parameter_count")]
    public static int ActionGetParameterCount(int handle)
        => ReadValue(handle, -1, (ActionSchema action) => action.Parameters.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_parameter")]
    public static int ActionGetParameter(int handle, int index)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        if (a == null || index < 0 || index >= a.Parameters.Count) return 0;
        return ObjectRegistry.Store(a.Parameters[index]);
    }

    // -- Preconditions (lifted) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_fluent_precondition_count")]
    public static int ActionGetFluentPreconditionCount(int handle)
        => ReadValue(handle, -1, (ActionSchema action) => action.FluentPreconditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_fluent_precondition")]
    public static int ActionGetFluentPrecondition(int handle, int index)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        if (a == null || index < 0 || index >= a.FluentPreconditions.Count) return 0;
        return ObjectRegistry.Store(a.FluentPreconditions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_static_precondition_count")]
    public static int ActionGetStaticPreconditionCount(int handle)
        => ReadValue(handle, -1, (ActionSchema action) => action.StaticPreconditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_static_precondition")]
    public static int ActionGetStaticPrecondition(int handle, int index)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        if (a == null || index < 0 || index >= a.StaticPreconditions.Count) return 0;
        return ObjectRegistry.Store(a.StaticPreconditions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_derived_precondition_count")]
    public static int ActionGetDerivedPreconditionCount(int handle)
        => ReadValue(handle, -1, (ActionSchema action) => action.DerivedPreconditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_derived_precondition")]
    public static int ActionGetDerivedPrecondition(int handle, int index)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        if (a == null || index < 0 || index >= a.DerivedPreconditions.Count) return 0;
        return ObjectRegistry.Store(a.DerivedPreconditions[index]);
    }

    // -- Effects (conditional) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_effect_count")]
    public static int ActionGetEffectCount(int handle)
        => ReadValue(handle, -1, (ActionSchema action) => action.Effects.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_get_effect")]
    public static int ActionGetEffect(int handle, int index)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        if (a == null || index < 0 || index >= a.Effects.Count) return 0;
        return ObjectRegistry.Store(a.Effects[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_to_string")]
    public static IntPtr ActionToString(int handle)
    {
        var a = ObjectRegistry.Get<ActionSchema>(handle);
        return AllocUtf8(a?.ToString());
    }

    // ====== ConditionalEffect (lifted) ======

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_effect")]
    public static int ConditionalEffectGetEffect(int handle)
    {
        var ce = ObjectRegistry.Get<ConditionalEffect>(handle);
        if (ce == null) return 0;
        return ObjectRegistry.Store(ce.Effect);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_quantified_count")]
    public static int ConditionalEffectGetQuantifiedCount(int handle)
        => ReadValue(handle, -1, (ConditionalEffect effect) => effect.QuantifiedVariables.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_quantified")]
    public static int ConditionalEffectGetQuantified(int handle, int index)
    {
        var ce = ObjectRegistry.Get<ConditionalEffect>(handle);
        if (ce == null || index < 0 || index >= ce.QuantifiedVariables.Count) return 0;
        return ObjectRegistry.Store(ce.QuantifiedVariables[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_fluent_condition_count")]
    public static int ConditionalEffectGetFluentConditionCount(int handle)
        => ReadValue(handle, -1, (ConditionalEffect effect) => effect.FluentConditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_fluent_condition")]
    public static int ConditionalEffectGetFluentCondition(int handle, int index)
    {
        var ce = ObjectRegistry.Get<ConditionalEffect>(handle);
        if (ce == null || index < 0 || index >= ce.FluentConditions.Count) return 0;
        return ObjectRegistry.Store(ce.FluentConditions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_static_condition_count")]
    public static int ConditionalEffectGetStaticConditionCount(int handle)
        => ReadValue(handle, -1, (ConditionalEffect effect) => effect.StaticConditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_static_condition")]
    public static int ConditionalEffectGetStaticCondition(int handle, int index)
    {
        var ce = ObjectRegistry.Get<ConditionalEffect>(handle);
        if (ce == null || index < 0 || index >= ce.StaticConditions.Count) return 0;
        return ObjectRegistry.Store(ce.StaticConditions[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_derived_condition_count")]
    public static int ConditionalEffectGetDerivedConditionCount(int handle)
        => ReadValue(handle, -1, (ConditionalEffect effect) => effect.DerivedConditions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_get_derived_condition")]
    public static int ConditionalEffectGetDerivedCondition(int handle, int index)
    {
        var ce = ObjectRegistry.Get<ConditionalEffect>(handle);
        if (ce == null || index < 0 || index >= ce.DerivedConditions.Count) return 0;
        return ObjectRegistry.Store(ce.DerivedConditions[index]);
    }
}
