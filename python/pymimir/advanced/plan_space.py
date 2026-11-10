"""ctypes signatures for the native :mod:`pymimir.plan_space` surface."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind


_int = ctypes.c_int
_ptr = ctypes.c_void_p
_cstr = ctypes.c_char_p


_bind("mimir_plan_space_plan_empty", [_int, _int, _int], _int)
_bind("mimir_plan_space_plan_from_sequential_plan", [_int, _ptr, _int, _int, _int], _int)
_bind("mimir_plan_space_plan_empty_with_goal", [_int, _ptr, _int, _int, _int], _int)
_bind("mimir_plan_space_plan_from_sequential_plan_with_goal", [_int, _ptr, _int, _ptr, _int, _int, _int], _int)
_bind("mimir_plan_space_plan_get_goal_count", [_int], _int)
_bind("mimir_plan_space_plan_get_goal_atom", [_int, _int], _int)
_bind("mimir_plan_space_plan_with_goal", [_int, _ptr, _int], _int)
_bind("mimir_plan_space_plan_prune_to_achieved_goal", [_int], _int)
_bind("mimir_plan_space_plan_clone", [_int], _int)
_bind("mimir_plan_space_plan_get_round", [_int], _int)
_bind("mimir_plan_space_plan_is_before", [_int, _int, _int], _int)
_bind("mimir_plan_space_plan_is_solution", [_int], _int)
_bind("mimir_plan_space_plan_get_step_value_count", [_int], _int)
_bind("mimir_plan_space_plan_copy_steps", [_int, _ptr, _int], _int)
_bind("mimir_plan_space_plan_get_link_value_count", [_int], _int)
_bind("mimir_plan_space_plan_copy_links", [_int, _ptr, _int], _int)
_bind("mimir_plan_space_plan_get_link_atom", [_int, _int], _int)
_bind("mimir_plan_space_plan_get_tombstone_value_count", [_int], _int)
_bind("mimir_plan_space_plan_copy_tombstones", [_int, _ptr, _int], _int)
_bind("mimir_plan_space_plan_get_ordering_value_count", [_int], _int)
_bind("mimir_plan_space_plan_copy_ordering", [_int, _ptr, _int], _int)
_bind("mimir_plan_space_plan_compute_flaws", [_int], _int)
_bind("mimir_plan_space_plan_apply", [_int, _int, _ptr, _int, _ptr, _int], _int)
_bind("mimir_plan_space_plan_linearize", [_int], _int)
_bind("mimir_plan_space_action_list_get_count", [_int], _int)
_bind("mimir_plan_space_action_list_get_action", [_int, _int], _int)

_bind("mimir_plan_space_flaws_get_value_count", [_int], _int)
_bind("mimir_plan_space_flaws_copy_flaws", [_int, _ptr, _int], _int)
_bind("mimir_plan_space_flaws_get_flaw_atom", [_int, _int], _int)
_bind("mimir_plan_space_flaws_get_ghost_value_count", [_int], _int)
_bind("mimir_plan_space_flaws_copy_ghosts", [_int, _ptr, _int], _int)

_bind("mimir_plan_space_edit_result_get_value_count", [_int], _int)
_bind("mimir_plan_space_edit_result_copy_values", [_int, _ptr, _int], _int)

_bind("mimir_plan_space_oracle_targets", [_int, _int, _int], _int)
_bind("mimir_plan_space_oracle_targets_get_value_count", [_int], _int)
_bind("mimir_plan_space_oracle_targets_copy_values", [_int, _ptr, _int], _int)

_bind("mimir_plan_space_encode_partial_plan", [_int, _int, _int, _cstr], _int)
_bind("mimir_plan_space_layout_get_value_count", [_int], _int)
_bind("mimir_plan_space_layout_copy_values", [_int, _ptr, _int], _int)

_bind("mimir_plan_space_relations", [_int, _cstr], _ptr)


__all__: list[str] = []
