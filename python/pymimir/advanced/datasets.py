"""ctypes signatures for state-space (SearchSpace) related Mimir.Interop exports."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind

_int = ctypes.c_int
_ptr = ctypes.c_void_p
_byte = ctypes.c_byte
_dbl = ctypes.c_double

_bind("mimir_state_space_create", [_int, _int, _int], _int)
_bind("mimir_state_space_total_states", [_int], _int)
_bind("mimir_state_space_goal_state_count", [_int], _int)
_bind("mimir_state_space_dead_end_state_count", [_int], _int)
_bind("mimir_state_space_max_depth", [_int], _int)
_bind("mimir_state_space_max_distance_to_goal", [_int], _int)
_bind("mimir_state_space_total_transitions", [_int], _int)
_bind("mimir_state_space_average_branching", [_int], _dbl)
_bind("mimir_state_space_get_state", [_int, _int], _int)
_bind("mimir_state_space_get_initial_state", [_int], _int)

_bind("mimir_state_space_is_goal", [_int, _int], _int)
_bind("mimir_state_space_is_dead_end", [_int, _int], _int)
_bind("mimir_state_space_distance_to_goal", [_int, _int], _int)
_bind("mimir_state_space_cost_to_goal", [_int, _int], _dbl)
_bind("mimir_state_space_get_depth", [_int, _int], _int)
_bind("mimir_state_space_successor_count", [_int, _int], _int)
_bind("mimir_state_space_get_successors", [_int, _int], _int)
_bind("mimir_state_space_get_predecessors", [_int, _int], _int)

_bind("mimir_transition_list_count", [_int], _int)
_bind("mimir_transition_list_copy_handles", [_int, _ptr, _ptr, _int], _int)
_bind("mimir_transition_list_get_action", [_int, _int], _int)
_bind("mimir_transition_list_get_state", [_int, _int], _int)
