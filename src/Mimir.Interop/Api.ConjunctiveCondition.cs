using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_conjunctive_condition_ground")]
    public static int ConjunctiveConditionGround(int conditionHandle, int stateHandle, int maxResults)
    {
        var condition = ObjectRegistry.Get<ConjunctiveCondition>(conditionHandle);
        ExtendedState? state = ReadExtendedState(stateHandle);
        if (condition == null || state?.State.Context.Problem == null) return 0;

        return CreateHandle(() =>
        {
            var gen = new ConjunctiveConditionBindingGenerator();
            var compiled = gen.Compile(condition, state.State.Context.Problem);
            int limit = maxResults > 0 ? maxResults : int.MaxValue;
            var results = new List<IReadOnlyList<Constant>>();
            gen.EnumerateBindings(compiled, state, limit, binding =>
            {
                results.Add(binding.ToArray());
                return true;
            });
            return results;
        });
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_binding_list_count")]
    public static int BindingListCount(int handle)
        => ReadValue(handle, -1, (List<IReadOnlyList<Constant>> bindings) => bindings.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_binding_list_get_binding_size")]
    public static int BindingListGetBindingSize(int handle, int bindingIndex)
    {
        var list = ObjectRegistry.Get<List<IReadOnlyList<Constant>>>(handle);
        if (list == null || bindingIndex < 0 || bindingIndex >= list.Count) return 0;
        return list[bindingIndex].Count;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_binding_list_get_object")]
    public static int BindingListGetObject(int handle, int bindingIndex, int objIndex)
    {
        var list = ObjectRegistry.Get<List<IReadOnlyList<Constant>>>(handle);
        if (list == null || bindingIndex < 0 || bindingIndex >= list.Count) return 0;
        var binding = list[bindingIndex];
        if (objIndex < 0 || objIndex >= binding.Count) return 0;
        return ObjectRegistry.Store(binding[objIndex]);
    }

    // -------- ConjunctiveCondition inspection (mirrors action precondition accessors) --------

    [UnmanagedCallersOnly(EntryPoint = "mimir_conjunctive_condition_get_parameter_count")]
    public static int CcGetParameterCount(int handle)
        => ReadValue(handle, -1, (ConjunctiveCondition condition) => condition.Parameters.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_conjunctive_condition_get_parameter")]
    public static int CcGetParameter(int handle, int index)
    {
        var cc = ObjectRegistry.Get<ConjunctiveCondition>(handle);
        if (cc == null || index < 0 || index >= cc.Parameters.Count) return 0;
        return ObjectRegistry.Store(cc.Parameters[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_conjunctive_condition_get_literal_count")]
    public static int CcGetLiteralCount(int handle)
    {
        var cc = ObjectRegistry.Get<ConjunctiveCondition>(handle);
        if (cc == null) return 0;
        return cc.StaticLiterals.Count + cc.FluentLiterals.Count + cc.DerivedLiterals.Count;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_conjunctive_condition_get_literal")]
    public static int CcGetLiteral(int handle, int index)
    {
        var cc = ObjectRegistry.Get<ConjunctiveCondition>(handle);
        if (cc == null || index < 0) return 0;
        int i = index;
        if (i < cc.StaticLiterals.Count) return ObjectRegistry.Store(cc.StaticLiterals[i]);
        i -= cc.StaticLiterals.Count;
        if (i < cc.FluentLiterals.Count) return ObjectRegistry.Store(cc.FluentLiterals[i]);
        i -= cc.FluentLiterals.Count;
        if (i < cc.DerivedLiterals.Count) return ObjectRegistry.Store(cc.DerivedLiterals[i]);
        return 0;
    }

}
