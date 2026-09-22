"""ctypes signatures for numeric Mimir.Interop exports (``Api.Numeric.cs``)."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind

_int = ctypes.c_int
_ptr = ctypes.c_void_p
_double = ctypes.c_double
_string = ctypes.c_char_p

_bind("mimir_domain_get_function_count", [_int], _int)
_bind("mimir_domain_get_function", [_int, _int], _int)
_bind("mimir_function_get_name", [_int], _ptr)
_bind("mimir_function_get_parameter_count", [_int], _int)
_bind("mimir_function_get_parameter", [_int, _int], _int)
_bind("mimir_problem_function_call", [_int, _string, _ptr, _int], _int)
_bind("mimir_problem_lifted_function_call", [_int, _int, _ptr, _int], _int)
_bind("mimir_numeric_constant", [_double], _int)
_bind("mimir_numeric_binary", [_int, _int, _int], _int)
_bind("mimir_numeric_kind", [_int], _int)
_bind("mimir_numeric_is_ground", [_int], _int)
_bind("mimir_numeric_constant_value", [_int], _double)
_bind("mimir_numeric_operator", [_int], _int)
_bind("mimir_numeric_operand", [_int, _int], _int)
_bind("mimir_numeric_function", [_int], _int)
_bind("mimir_numeric_argument_count", [_int], _int)
_bind("mimir_numeric_argument", [_int, _int], _int)
_bind("mimir_action_cost_expression", [_int], _int)
_bind("mimir_ground_action_cost_expression", [_int], _int)
_bind("mimir_state_numeric_value", [_int, _int], _double)
_bind("mimir_state_holds_comparison", [_int, _int], _int)

_bind("mimir_numeric_comparison_count", [_int], _int)
_bind("mimir_numeric_comparison_get", [_int, _int], _int)
_bind("mimir_numeric_comparison_operand", [_int, _int], _int)
_bind("mimir_numeric_comparison_operator", [_int], _int)
_bind("mimir_numeric_comparison_create", [_int, _int, _int], _int)
_bind("mimir_numeric_update_count", [_int], _int)
_bind("mimir_numeric_update_get", [_int, _int], _int)
_bind("mimir_numeric_update_operand", [_int, _int], _int)
_bind("mimir_numeric_update_operator", [_int], _int)
