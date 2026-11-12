"""ctypes signatures for the native :mod:`pymimir.plan_space` surface."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind


_int = ctypes.c_int
_ptr = ctypes.c_void_p


_bind("mimir_serial_plan_empty", [_int], _int)
_bind("mimir_serial_plan_empty_with_goal", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_get_goal_count", [_int], _int)
_bind("mimir_serial_plan_get_goal_literal", [_int, _int], _int)
_bind("mimir_serial_plan_with_goal", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_prune_to_achieved_goal", [_int], _int)
_bind("mimir_serial_plan_clone", [_int], _int)
_bind("mimir_serial_plan_is_solution", [_int], _int)
_bind("mimir_serial_plan_get_step_value_count", [_int], _int)
_bind("mimir_serial_plan_copy_steps", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_get_parameter_option_count", [_int, _int, _int], _int)
_bind("mimir_serial_plan_copy_parameter_options", [_int, _int, _int, _ptr, _int], _int)
_bind("mimir_serial_plan_diagnose", [_int], _int)
_bind("mimir_serial_plan_apply", [_int, _ptr, _int], _int)
_bind("mimir_serial_plan_get_step_precondition_count", [_int, _int], _int)
_bind("mimir_serial_plan_get_step_precondition", [_int, _int, _int], _int)
_bind("mimir_serial_plan_get_step_effect_count", [_int, _int], _int)
_bind("mimir_serial_plan_get_step_effect", [_int, _int, _int], _int)
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
_bind("mimir_serial_diagnosis_get_final_fact_count", [_int], _int)
_bind("mimir_serial_diagnosis_get_final_fact", [_int, _int], _int)


__all__: list[str] = []
