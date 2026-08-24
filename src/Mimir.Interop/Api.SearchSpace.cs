using System;
using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Space;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_create")]
    public static int StateSpaceCreate(int problemHandle, int goalHandle, int maxStates)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        var goal = goalHandle == 0
            ? problem == null ? null : GoalCondition.FromProblem(problem)
            : ObjectRegistry.Get<GoalCondition>(goalHandle);
        if (problem == null || goal == null || maxStates < 0) return 0;
        return CreateHandle(() =>
        {
            IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
            var builder = new SearchSpaceBuilder()
                .WithInitialState(problem.InitialState)
                .WithGoal(goal)
                .WithActionGenerator(generator);
            if (maxStates > 0) builder.WithMaxStates(maxStates);
            return builder.Build();
        });
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_total_states")]
    public static int StateSpaceTotalStates(int handle)
        => ReadValue(handle, -1, (SearchSpace space) => space.TotalStates);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_goal_state_count")]
    public static int StateSpaceGoalStateCount(int handle)
        => ReadValue(handle, -1, (SearchSpace space) => space.GoalStateCount);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_dead_end_state_count")]
    public static int StateSpaceDeadEndStateCount(int handle)
        => ReadValue(handle, -1, (SearchSpace space) => space.DeadEndStateCount);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_max_depth")]
    public static int StateSpaceMaxDepth(int handle)
        => ReadValue(handle, -1, (SearchSpace space) => space.MaxDepth);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_max_distance_to_goal")]
    public static int StateSpaceMaxDistanceToGoal(int handle)
        => ReadValue(handle, -1, (SearchSpace space) => space.MaxDistanceToGoal);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_total_transitions")]
    public static int StateSpaceTotalTransitions(int handle)
        => ReadValue(handle, -1, (SearchSpace space) => space.TotalTransitions);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_average_branching")]
    public static double StateSpaceAverageBranching(int handle)
        => ReadValue(handle, double.NaN, (SearchSpace space) => space.AverageBranchingFactor);

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_get_state")]
    public static int StateSpaceGetState(int handle, int index)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        if (space == null || index < 0 || index >= space.AllStates.Count) return 0;
        return ExpandAndStoreState(space.AllStates[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_get_initial_state")]
    public static int StateSpaceGetInitialState(int handle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        if (space == null) return 0;
        return ExpandAndStoreState(space.InitialState);
    }

    // -- Per-state queries --

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_is_goal")]
    public static int StateSpaceIsGoal(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return InvalidBoolean;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return InvalidBoolean;
        return space.Query.IsGoalState(state) ? 1 : 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_is_dead_end")]
    public static int StateSpaceIsDeadEnd(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return InvalidBoolean;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return InvalidBoolean;
        return space.Query.IsDeadEnd(state) ? 1 : 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_distance_to_goal")]
    public static int StateSpaceDistanceToGoal(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return -1;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return -1;
        return space.Query.GetDistanceToGoal(state);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_cost_to_goal")]
    public static double StateSpaceCostToGoal(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return double.PositiveInfinity;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return double.PositiveInfinity;
        return space.Query.GetCostToGoal(state);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_get_depth")]
    public static int StateSpaceGetDepth(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return -1;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return -1;
        return space.Query.GetDepth(state);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_successor_count")]
    public static int StateSpaceSuccessorCount(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return 0;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return 0;
        return space.Query.GetSuccessors(state).Count;
    }

    /// <summary>
    /// Returns a list-of-tuples object containing the successors for the given state.
    /// Use mimir_transition_list_* to read it.
    /// </summary>
    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_get_successors")]
    public static int StateSpaceGetSuccessors(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return 0;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return 0;
        var successors = space.Query.GetSuccessors(state);
        return ObjectRegistry.Store(successors);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_state_space_get_predecessors")]
    public static int StateSpaceGetPredecessors(int handle, int stateHandle)
    {
        var space = ObjectRegistry.Get<SearchSpace>(handle);
        State? state = ReadExtendedState(stateHandle)?.State;
        if (space == null || state == null) return 0;
        if (!ReferenceEquals(space.InitialState.Context, state.Context)) return 0;
        var predecessors = space.Query.GetPredecessors(state);
        return ObjectRegistry.Store(predecessors);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_transition_list_count")]
    public static int TransitionListCount(int handle)
    {
        if (ObjectRegistry.GetRaw(handle) is System.Collections.Generic.IReadOnlyList<(State, GroundAction)> list)
            return list.Count;
        return 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_transition_list_get_action")]
    public static int TransitionListGetAction(int handle, int index)
    {
        var obj = ObjectRegistry.GetRaw(handle);
        if (obj is System.Collections.Generic.IReadOnlyList<(State, GroundAction)> list1)
        {
            if (index < 0 || index >= list1.Count) return 0;
            return ObjectRegistry.Store(list1[index].Item2);
        }
        return 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_transition_list_get_state")]
    public static int TransitionListGetState(int handle, int index)
    {
        var obj = ObjectRegistry.GetRaw(handle);
        if (obj is System.Collections.Generic.IReadOnlyList<(State, GroundAction)> list1)
        {
            if (index < 0 || index >= list1.Count) return 0;
            return ExpandAndStoreState(list1[index].Item1);
        }
        return 0;
    }
}
