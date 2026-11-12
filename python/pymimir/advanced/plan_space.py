"""ctypes signatures for the native :mod:`pymimir.plan_space` surface."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind


_int = ctypes.c_int
_ptr = ctypes.c_void_p
_cstr = ctypes.c_char_p


_bind("mimir_serial_plan_empty", [_int], _int)
_bind("mimir_serial_plan_empty_with_goal", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_get_goal_count", [_int], _int)
_bind("mimir_serial_plan_get_goal_literal", [_int, _int], _int)
_bind("mimir_serial_plan_with_goal", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_prune_to_achieved_goal", [_int], _int)
_bind("mimir_serial_plan_clone", [_int], _int)
_bind("mimir_serial_plan_get_round", [_int], _int)
_bind("mimir_serial_plan_is_solution", [_int], _int)
_bind("mimir_serial_plan_get_step_value_count", [_int], _int)
_bind("mimir_serial_plan_copy_steps", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_get_step_purpose", [_int, _int], _int)
_bind("mimir_serial_plan_diagnose", [_int], _int)
_bind("mimir_serial_plan_apply", [_int, _int, _ptr, _int, _ptr, _int], _int)
_bind("mimir_serial_plan_to_actions", [_int], _int)
_bind("mimir_serial_action_list_get_count", [_int], _int)
_bind("mimir_serial_action_list_get_action", [_int, _int], _int)

_bind("mimir_serial_diagnosis_get_error_value_count", [_int], _int)
_bind("mimir_serial_diagnosis_copy_errors", [_int, _ptr, _int], _int)
_bind("mimir_serial_diagnosis_get_error_literal", [_int, _int], _int)
_bind("mimir_serial_diagnosis_get_candidate_value_count", [_int], _int)
_bind("mimir_serial_diagnosis_copy_candidates", [_int, _ptr, _int], _int)
_bind("mimir_serial_diagnosis_get_support_value_count", [_int], _int)
_bind("mimir_serial_diagnosis_copy_supports", [_int, _ptr, _int], _int)
_bind("mimir_serial_diagnosis_get_support_literal", [_int, _int], _int)
_bind("mimir_serial_diagnosis_get_unused_step_count", [_int], _int)
_bind("mimir_serial_diagnosis_copy_unused_steps", [_int, _ptr, _int], _int)

_bind("mimir_serial_edit_result_get_value_count", [_int], _int)
_bind("mimir_serial_edit_result_copy_values", [_int, _ptr, _int], _int)

_bind("mimir_serial_search", [_int, _int, ctypes.c_double], _int)

_bind("mimir_serial_oracle_targets", [_int, _int, _int], _int)
_bind("mimir_serial_oracle_targets_get_value_count", [_int], _int)
_bind("mimir_serial_oracle_targets_copy_values", [_int, _ptr, _int], _int)

_bind("mimir_serial_encode_plan", [_int, _int, _int, _cstr], _int)
_bind("mimir_serial_layout_get_value_count", [_int], _int)
_bind("mimir_serial_layout_copy_values", [_int, _ptr, _int], _int)

_bind("mimir_serial_relations", [_int, _cstr], _ptr)


__all__: list[str] = []
