"""ctypes signatures for the programmatic model builders."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind


_int = ctypes.c_int
_byte = ctypes.c_byte
_double = ctypes.c_double
_pointer = ctypes.c_void_p
_string = ctypes.c_char_p


# Domain and domain sections.
_bind("mimir_domain_builder_create", [_string], _int)
_bind("mimir_domain_builder_requirements", [_int], _int)
_bind("mimir_domain_builder_types", [_int], _int)
_bind("mimir_domain_builder_constants", [_int], _int)
_bind("mimir_domain_builder_predicates", [_int], _int)
_bind("mimir_domain_builder_functions", [_int], _int)
_bind("mimir_domain_builder_actions", [_int], _int)
_bind("mimir_domain_builder_derived_predicates", [_int], _int)
_bind("mimir_domain_builder_build", [_int], _int)

_bind("mimir_requirement_list_builder_add", [_int, _string], _byte)
_bind("mimir_requirement_list_builder_close", [_int], _byte)
_bind("mimir_type_list_builder_add", [_int, _string, _string], _byte)
_bind("mimir_type_list_builder_close", [_int], _byte)
_bind("mimir_constant_list_builder_add", [_int, _string, _string], _byte)
_bind("mimir_constant_list_builder_close", [_int], _byte)
_bind(
    "mimir_predicate_list_builder_add",
    [_int, _string, _pointer, _pointer, _int],
    _byte,
)
_bind("mimir_predicate_list_builder_close", [_int], _byte)
_bind(
    "mimir_numeric_function_list_builder_add",
    [_int, _string, _pointer, _pointer, _int],
    _byte,
)
_bind("mimir_numeric_function_list_builder_close", [_int], _byte)
_bind(
    "mimir_derived_predicate_list_builder_define",
    [_int, _string, _int],
    _byte,
)
_bind("mimir_derived_predicate_list_builder_close", [_int], _byte)


# Action schemas and conditional effects.
_bind("mimir_action_list_builder_add", [_int, _string], _int)
_bind("mimir_action_list_builder_close", [_int], _byte)
_bind(
    "mimir_action_schema_builder_add_parameter",
    [_int, _string, _string],
    _byte,
)
_bind(
    "mimir_action_schema_builder_add_precondition",
    [_int, _string, _byte, _pointer, _int],
    _byte,
)
_bind(
    "mimir_action_schema_builder_add_effect",
    [_int, _string, _byte, _pointer, _int],
    _byte,
)
_bind("mimir_action_schema_builder_add_conditional_effect", [_int], _int)
_bind("mimir_action_schema_builder_with_constant_cost", [_int, _double], _byte)
_bind("mimir_action_schema_builder_with_cost", [_int, _int], _byte)
_bind("mimir_action_schema_builder_close", [_int], _byte)

_bind(
    "mimir_conditional_effect_builder_add_parameter",
    [_int, _string, _string],
    _byte,
)
_bind(
    "mimir_conditional_effect_builder_add_condition",
    [_int, _string, _byte, _pointer, _int],
    _byte,
)
_bind(
    "mimir_conditional_effect_builder_add_effect",
    [_int, _string, _byte, _pointer, _int],
    _byte,
)
_bind("mimir_conditional_effect_builder_close", [_int], _byte)


# Logical-expression specifications.
_bind("mimir_logic_true", [], _int)
_bind("mimir_logic_false", [], _int)
_bind("mimir_logic_atom", [_string, _pointer, _int], _int)
_bind("mimir_logic_equal", [_string, _string], _int)
_bind("mimir_logic_not", [_int], _int)
_bind("mimir_logic_and", [_pointer, _int], _int)
_bind("mimir_logic_or", [_pointer, _int], _int)
_bind("mimir_logic_imply", [_int, _int], _int)
_bind("mimir_logic_exists", [_pointer, _pointer, _int, _int], _int)
_bind("mimir_logic_forall", [_pointer, _pointer, _int, _int], _int)


# Action-cost specifications.
_bind("mimir_action_cost_constant", [_double], _int)
_bind("mimir_action_cost_function", [_string, _pointer, _int], _int)
_bind("mimir_action_cost_add", [_int, _int], _int)
_bind("mimir_action_cost_subtract", [_int, _int], _int)
_bind("mimir_action_cost_multiply", [_int, _int], _int)
_bind("mimir_action_cost_divide", [_int, _int], _int)


# Problem and problem sections.
_bind("mimir_problem_builder_create", [_int, _string, _string], _int)
_bind("mimir_problem_builder_objects", [_int], _int)
_bind("mimir_problem_builder_initial_state", [_int], _int)
_bind("mimir_problem_builder_goal", [_int], _int)
_bind("mimir_problem_builder_build", [_int], _int)

_bind(
    "mimir_problem_object_list_builder_add",
    [_int, _string, _string],
    _byte,
)
_bind("mimir_problem_object_list_builder_close", [_int], _byte)
_bind(
    "mimir_initial_state_builder_add_fact",
    [_int, _string, _pointer, _int],
    _byte,
)
_bind(
    "mimir_initial_state_builder_add_numeric_initialization",
    [_int, _string, _double, _pointer, _int],
    _byte,
)
_bind("mimir_initial_state_builder_close", [_int], _byte)
_bind(
    "mimir_goal_builder_add",
    [_int, _string, _byte, _pointer, _int],
    _byte,
)
_bind("mimir_goal_builder_close", [_int], _byte)
