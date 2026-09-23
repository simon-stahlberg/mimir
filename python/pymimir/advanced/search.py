"""ctypes signatures for search-related Mimir.Interop exports."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind

_int = ctypes.c_int
_byte = ctypes.c_byte
_dbl = ctypes.c_double
_ptr = ctypes.c_void_p
_cstr = ctypes.c_char_p

# ----- Plan-result entry-points -----

_bind("mimir_plan_result_is_success", [_int], _byte)
_bind("mimir_plan_result_get_status", [_int], _int)
_bind("mimir_plan_result_get_time_ms", [_int], _dbl)
_bind("mimir_plan_result_get_setup_time_ms", [_int], _dbl)
_bind("mimir_plan_result_get_search_time_ms", [_int], _dbl)
_bind("mimir_plan_result_get_nodes_expanded", [_int], _int)
_bind("mimir_plan_result_get_nodes_generated", [_int], _int)
_bind("mimir_plan_result_get_plan_length", [_int], _int)
_bind("mimir_plan_result_get_plan_cost", [_int], _dbl)
_bind("mimir_plan_result_get_action", [_int, _int], _ptr)
_bind("mimir_plan_result_get_ground_action", [_int, _int], _int)


# Unified search entry point.
_bind(
    "mimir_search",
    [_cstr, _int, _int, _int, _int, _dbl, _int,
     _ptr, _ptr, _ptr, _ptr, _ptr, _ptr, _ptr, _int],
    _int,
)


# ----- ctypes callback types -----
STATE_CALLBACK = ctypes.CFUNCTYPE(_byte, _int)
TRANSITION_CALLBACK = ctypes.CFUNCTYPE(_byte, _int, _int, _int)
DOUBLE_CALLBACK = ctypes.CFUNCTYPE(_byte, _dbl)
STOP_CALLBACK = ctypes.CFUNCTYPE(_byte, _int)

_bind("mimir_qgbfs_search", [_int, _int, _int, _ptr, _int, _byte, _dbl, _int, _ptr, _int], _int)
_bind("mimir_plan_result_get_visited_states", [_int], _int)
_bind("mimir_plan_result_get_generated_transitions", [_int], _int)
_bind("mimir_plan_result_get_evaluated_candidates", [_int], _int)
_bind("mimir_plan_result_get_partial_plan_length", [_int], _int)
_bind("mimir_plan_result_copy_scored_path", [_int, _ptr, _ptr, _ptr, _int], _int)

_bind("mimir_qbeam_search", [_int, _int, _int, _ptr, _int, _int, _byte, _dbl, _int, _ptr, _int], _int)
_bind("mimir_beam_search", [_int, _int, _int, _int, _int, _int, _dbl, _int, _ptr, _int], _int)

_bind("mimir_dead_end_detector_h2", [_int], _int)
_bind("mimir_dead_end_detector_disjunction", [_ptr, _int], _int)
_bind("mimir_dead_end_detector_evaluate", [_int, _int, _int], _int)
