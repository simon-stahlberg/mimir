using System;
using System.Runtime.InteropServices;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_load_domain")]
    public static int LoadDomain(IntPtr domainPathPtr)
    {
        string? domainPath = ReadUtf8(domainPathPtr);
        if (domainPath == null) return 0;
        return CreateHandle(() => Domain.FromFile(domainPath));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_load_domain_from_string")]
    public static int LoadDomainFromString(IntPtr domainTextPtr)
    {
        string? domainText = ReadUtf8(domainTextPtr);
        if (domainText == null) return 0;
        return CreateHandle(() => Domain.FromText(domainText));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_name")]
    public static IntPtr DomainGetName(int handle)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        return AllocUtf8(domain?.Name);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_requirement_count")]
    public static int DomainGetRequirementCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Requirements.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_requirement")]
    public static IntPtr DomainGetRequirement(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.Requirements.Count) return IntPtr.Zero;
        return AllocUtf8(domain.Requirements[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_expanded_requirement_count")]
    public static int DomainGetExpandedRequirementCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.ExpandedRequirements.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_expanded_requirement")]
    public static IntPtr DomainGetExpandedRequirement(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.ExpandedRequirements.Count) return IntPtr.Zero;
        return AllocUtf8(domain.ExpandedRequirements[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_uses_typing")]
    public static int DomainUsesTyping(int handle)
        => ReadValue(handle, InvalidBoolean, (Domain domain) => domain.UsesTyping ? 1 : 0);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_uses_equality")]
    public static int DomainUsesEquality(int handle)
        => ReadValue(handle, InvalidBoolean, (Domain domain) => domain.UsesEquality ? 1 : 0);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_uses_conditional_effects")]
    public static int DomainUsesConditionalEffects(int handle)
        => ReadValue(handle, InvalidBoolean, (Domain domain) => domain.UsesConditionalEffects ? 1 : 0);

    // -- Predicates --

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_static_predicate_count")]
    public static int DomainGetStaticPredicateCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Statics.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_static_predicate")]
    public static int DomainGetStaticPredicate(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.Statics.Count) return 0;
        return ObjectRegistry.Store(domain.Statics[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_fluent_predicate_count")]
    public static int DomainGetFluentPredicateCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Fluents.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_fluent_predicate")]
    public static int DomainGetFluentPredicate(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.Fluents.Count) return 0;
        return ObjectRegistry.Store(domain.Fluents[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_derived_predicate_count")]
    public static int DomainGetDerivedPredicateCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Derived.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_derived_predicate")]
    public static int DomainGetDerivedPredicate(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.Derived.Count) return 0;
        return ObjectRegistry.Store(domain.Derived[index]);
    }

    // -- Actions --

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_action_count")]
    public static int DomainGetActionCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Actions.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_action")]
    public static int DomainGetAction(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.Actions.Count) return 0;
        return ObjectRegistry.Store(domain.Actions[index]);
    }

    // -- Constants --

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_constant_count")]
    public static int DomainGetConstantCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.Constants.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_constant")]
    public static int DomainGetConstant(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index >= domain.Constants.Count) return 0;
        return ObjectRegistry.Store(domain.Constants[index]);
    }

    // -- Type hierarchy --

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_type_count")]
    public static int DomainGetTypeCount(int handle)
        => ReadValue(handle, -1, (Domain domain) => domain.TypeHierarchy.Count + 1);

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_type_name")]
    public static IntPtr DomainGetTypeName(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index > domain.TypeHierarchy.Count) return IntPtr.Zero;
        return AllocUtf8(index == 0 ? "object" : domain.TypeHierarchy.Keys.ElementAt(index - 1));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_get_type_parent")]
    public static IntPtr DomainGetTypeParent(int handle, int index)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null || index < 0 || index > domain.TypeHierarchy.Count) return IntPtr.Zero;
        if (index == 0) return IntPtr.Zero;
        return AllocUtf8(domain.TypeHierarchy.Values.ElementAt(index - 1));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_is_compatible")]
    public static int DomainIsCompatible(int handle, IntPtr childPtr, IntPtr parentPtr)
    {
        var domain = ObjectRegistry.Get<Domain>(handle);
        if (domain == null) return InvalidBoolean;
        string? child = ReadUtf8(childPtr);
        string? parent = ReadUtf8(parentPtr);
        if (child == null || parent == null) return InvalidBoolean;
        return domain.IsCompatible(child, parent) ? 1 : 0;
    }
}
