from __future__ import annotations

import ctypes
from ._native import lib, native_errcheck

_int = ctypes.c_int
_ptr = ctypes.c_void_p
_double = ctypes.c_double
_string = ctypes.c_char_p

def _bind(name: str, arguments: list[type[object]], result: type[object]) -> None:
    function = getattr(lib, name)
    function.argtypes = arguments
    function.restype = result
    function.errcheck = native_errcheck

_bind("mimir_domain_get_function_count", [_int], _int)
_bind("mimir_domain_get_function", [_int, _int], _int)
_bind("mimir_function_get_name", [_int], _ptr)
_bind("mimir_function_get_parameter_count", [_int], _int)
_bind("mimir_function_get_parameter", [_int, _int], _int)
_bind("mimir_problem_function_call", [_int, _string, _ptr, _int], _int)
_bind("mimir_numeric_constant", [_double], _int)
_bind("mimir_numeric_binary", [_int, _int, _int], _int)
_bind("mimir_numeric_kind", [_int], _int)
_bind("mimir_numeric_constant_value", [_int], _double)
_bind("mimir_numeric_operator", [_int], _int)
_bind("mimir_numeric_operand", [_int, _int], _int)
_bind("mimir_numeric_function", [_int], _int)
_bind("mimir_numeric_argument_count", [_int], _int)
_bind("mimir_numeric_argument", [_int, _int], _int)
_bind("mimir_action_cost_expression", [_int], _int)
_bind("mimir_ground_action_cost_expression", [_int], _int)
_bind("mimir_state_numeric_value", [_int, _int], _double)
_bind("mimir_numeric_spec_comparison", [_int, _int, _int], _int)

_bind("mimir_initial_state_set_value", [_int, _int, _double], ctypes.c_byte)

_bind("mimir_problem_lifted_function_call", [_int, _int, _ptr, _int], _int)
