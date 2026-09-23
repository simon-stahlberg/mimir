using System;
using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Planning;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_is_success")]
    public static byte PlanResultIsSuccess(int resultHandle)
        => ReadValue<PlanResult, byte>(resultHandle, 0, result => result.IsSuccess ? (byte)1 : (byte)0);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_status")]
    public static int PlanResultGetStatus(int resultHandle)
        => ReadValue(resultHandle, -1, (PlanResult result) => (int)result.Status);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_time_ms")]
    public static double PlanResultGetTimeMs(int resultHandle)
        => ReadValue(resultHandle, double.NaN, (PlanResult result) => result.ElapsedTime.TotalMilliseconds);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_setup_time_ms")]
    public static double PlanResultGetSetupTimeMs(int resultHandle)
        => ReadValue(resultHandle, double.NaN, (PlanResult result) => result.SetupTime.TotalMilliseconds);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_search_time_ms")]
    public static double PlanResultGetSearchTimeMs(int resultHandle)
        => ReadValue(resultHandle, double.NaN, (PlanResult result) => result.SearchTime.TotalMilliseconds);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_nodes_expanded")]
    public static int PlanResultGetNodesExpanded(int resultHandle)
        => ReadValue(resultHandle, -1, (PlanResult result) => result.Statistics.NodesExpanded);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_nodes_generated")]
    public static int PlanResultGetNodesGenerated(int resultHandle)
        => ReadValue(resultHandle, -1, (PlanResult result) => result.Statistics.NodesGenerated);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_max_depth")]
    public static int PlanResultGetMaxDepth(int resultHandle)
        => ReadValue(resultHandle, -1, (PlanResult result) => result.Statistics.MaxDepth);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_plan_length")]
    public static int PlanResultGetPlanLength(int resultHandle)
        => ReadValue(resultHandle, -1, (PlanResult result) => result.PlanLength);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_plan_cost")]
    public static double PlanResultGetPlanCost(int resultHandle)
        => ReadValue(resultHandle, double.NaN, (PlanResult result) => result.PlanCost);

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_action")]
    public static IntPtr PlanResultGetAction(int resultHandle, int index)
    {
        var result = ObjectRegistry.Get<PlanResult>(resultHandle);
        if (result == null || index < 0 || index >= result.Plan.Count) return IntPtr.Zero;
        return AllocUtf8(result.Plan[index].ToString());
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_plan_result_get_ground_action")]
    public static int PlanResultGetGroundAction(int resultHandle, int index)
    {
        var result = ObjectRegistry.Get<PlanResult>(resultHandle);
        if (result == null || index < 0 || index >= result.Plan.Count) return 0;
        return ObjectRegistry.Store(result.Plan[index]);
    }

}
