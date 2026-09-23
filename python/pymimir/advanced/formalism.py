"""ctypes signatures for formalism-related Mimir.Interop exports.

Importing this module configures ``argtypes`` / ``restype`` for every
``mimir_*`` function declared in ``Api.Domain.cs``, ``Api.Problem.cs``,
``Api.Predicate.cs``, ``Api.Term.cs``, ``Api.Atom.cs``, ``Api.Literal.cs``,
``Api.GroundAtom.cs``, ``Api.ActionSchema.cs``, ``Api.GroundAction.cs``,
``Api.GroundConditionalEffect.cs`` and ``Api.State.cs``.

Handle return values are :class:`int` (0 == null). Strings returned from the
native side are raw UTF-8 ``c_void_p`` pointers; callers must hand them to
``pymimir.advanced.take_string`` to decode and free in one step.
"""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind

# Common shorthands.
_int = ctypes.c_int
_byte = ctypes.c_byte
_dbl = ctypes.c_double
_ptr = ctypes.c_void_p
_cstr = ctypes.c_char_p

# ----- Domain -----

_bind("mimir_load_domain", [_cstr], _int)
_bind("mimir_load_domain_from_string", [_cstr], _int)
_bind("mimir_domain_get_name", [_int], _ptr)
_bind("mimir_domain_get_requirement_count", [_int], _int)
_bind("mimir_domain_get_requirement", [_int, _int], _ptr)
_bind("mimir_domain_get_expanded_requirement_count", [_int], _int)
_bind("mimir_domain_get_expanded_requirement", [_int, _int], _ptr)
_bind("mimir_domain_get_type_count", [_int], _int)
_bind("mimir_domain_get_type_name", [_int, _int], _ptr)
_bind("mimir_domain_get_type_parent", [_int, _int], _ptr)
_bind("mimir_domain_uses_typing", [_int], _int)
_bind("mimir_domain_uses_equality", [_int], _int)
_bind("mimir_domain_uses_conditional_effects", [_int], _int)
_bind("mimir_domain_get_static_predicate_count", [_int], _int)
_bind("mimir_domain_get_static_predicate", [_int, _int], _int)
_bind("mimir_domain_get_fluent_predicate_count", [_int], _int)
_bind("mimir_domain_get_fluent_predicate", [_int, _int], _int)
_bind("mimir_domain_get_derived_predicate_count", [_int], _int)
_bind("mimir_domain_get_derived_predicate", [_int, _int], _int)
_bind("mimir_domain_get_action_count", [_int], _int)
_bind("mimir_domain_get_action", [_int, _int], _int)
_bind("mimir_domain_get_constant_count", [_int], _int)
_bind("mimir_domain_get_constant", [_int, _int], _int)
_bind("mimir_domain_is_compatible", [_int, _cstr, _cstr], _int)


# ----- Problem -----

_bind("mimir_load_problem", [_int, _cstr, _cstr], _int)
_bind("mimir_load_problem_from_string", [_int, _cstr, _cstr], _int)
_bind("mimir_problem_get_name", [_int], _ptr)
_bind("mimir_problem_get_domain", [_int], _int)
_bind("mimir_problem_get_generator", [_int], _ptr)
_bind("mimir_problem_get_requirement_count", [_int], _int)
_bind("mimir_problem_get_requirement", [_int, _int], _ptr)
_bind("mimir_problem_get_declared_object_count", [_int], _int)
_bind("mimir_problem_get_declared_object", [_int, _int], _int)
_bind("mimir_problem_get_all_object_count", [_int], _int)
_bind("mimir_problem_get_all_object", [_int, _int], _int)
_bind("mimir_problem_get_object_by_name", [_int, _cstr], _int)
_bind("mimir_problem_get_initial_state", [_int], _int)
_bind("mimir_problem_get_static_initial_atom_count", [_int], _int)
_bind("mimir_problem_get_static_initial_atom", [_int, _int], _int)
_bind("mimir_problem_get_fluent_initial_atom_count", [_int], _int)
_bind("mimir_problem_get_fluent_initial_atom", [_int, _int], _int)
_bind("mimir_problem_get_goal_literal_count", [_int], _int)
_bind("mimir_problem_get_goal_literal", [_int, _int], _int)
_bind("mimir_problem_register_fact", [_int, _int, _ptr, _int], _int)
_bind("mimir_problem_new_variable", [_int, _cstr, _cstr], _int)
_bind("mimir_problem_new_atom", [_int, _int, _ptr, _int], _int)
_bind("mimir_problem_new_literal", [_int, _int, _byte], _int)
_bind("mimir_problem_new_ground_literal", [_int, _int, _byte], _int)
_bind("mimir_problem_new_conjunctive_condition", [_int, _ptr, _int, _ptr, _int, _ptr, _int], _int)
_bind("mimir_problem_new_ground_action", [_int, _int, _ptr, _int], _int)
_bind("mimir_problem_new_state", [_int, _ptr, _int, _ptr, _ptr, _int], _int)
_bind("mimir_goal_create", [_int, _ptr, _int, _ptr, _int], _int)
_bind("mimir_goal_is_satisfied", [_int, _int], _int)
_bind("mimir_goal_literal_count", [_int], _int)
_bind("mimir_goal_get_literal", [_int, _int], _int)


# ----- ConjunctiveCondition -----

_bind("mimir_conjunctive_condition_get_parameter_count", [_int], _int)
_bind("mimir_conjunctive_condition_get_parameter", [_int, _int], _int)
_bind("mimir_conjunctive_condition_get_literal_count", [_int], _int)
_bind("mimir_conjunctive_condition_get_literal", [_int, _int], _int)
_bind("mimir_conjunctive_condition_ground", [_int, _int, _int], _int)
_bind("mimir_binding_list_count", [_int], _int)
_bind("mimir_binding_list_get_binding_size", [_int, _int], _int)
_bind("mimir_binding_list_get_object", [_int, _int, _int], _int)


# ----- Predicate -----

_bind("mimir_predicate_get_name", [_int], _ptr)
_bind("mimir_predicate_get_arity", [_int], _int)
_bind("mimir_predicate_get_parameter_count", [_int], _int)
_bind("mimir_predicate_get_parameter", [_int, _int], _int)
_bind("mimir_predicate_get_predicate_type", [_int], _int)


# ----- Terms (Object/Constant + Variable) -----

_bind("mimir_object_get_name", [_int], _ptr)
_bind("mimir_object_get_type", [_int], _ptr)

_bind("mimir_variable_get_name", [_int], _ptr)
_bind("mimir_variable_get_type", [_int], _ptr)

_bind("mimir_term_get_kind", [_int], _int)
_bind("mimir_term_get_name", [_int], _ptr)


# ----- Atom -----

_bind("mimir_atom_get_predicate_type", [_int], _int)
_bind("mimir_atom_get_predicate", [_int], _int)
_bind("mimir_atom_get_argument_count", [_int], _int)
_bind("mimir_atom_get_argument", [_int, _int], _int)
_bind("mimir_atom_to_string", [_int], _ptr)


# ----- Ground atom (Fact) -----

_bind("mimir_ground_atom_get_predicate_type", [_int], _int)
_bind("mimir_ground_atom_get_predicate", [_int], _int)
_bind("mimir_ground_atom_get_argument_count", [_int], _int)
_bind("mimir_ground_atom_get_argument", [_int, _int], _int)
_bind("mimir_ground_atom_to_string", [_int], _ptr)


# ----- Literal (lifted Literal<Atom<T>>) -----

_bind("mimir_literal_get_predicate_type", [_int], _int)
_bind("mimir_literal_get_polarity", [_int], _byte)
_bind("mimir_literal_get_atom", [_int], _int)


# ----- Ground literal (Literal<Fact<T>>) -----

_bind("mimir_ground_literal_get_predicate_type", [_int], _int)
_bind("mimir_ground_literal_get_polarity", [_int], _byte)
_bind("mimir_ground_literal_get_atom", [_int], _int)


# ----- Action schema -----

_bind("mimir_action_get_name", [_int], _ptr)
_bind("mimir_action_get_arity", [_int], _int)
_bind("mimir_action_get_parameter_count", [_int], _int)
_bind("mimir_action_get_parameter", [_int, _int], _int)
_bind("mimir_action_get_fluent_precondition_count", [_int], _int)
_bind("mimir_action_get_fluent_precondition", [_int, _int], _int)
_bind("mimir_action_get_static_precondition_count", [_int], _int)
_bind("mimir_action_get_static_precondition", [_int, _int], _int)
_bind("mimir_action_get_derived_precondition_count", [_int], _int)
_bind("mimir_action_get_derived_precondition", [_int, _int], _int)
_bind("mimir_action_get_effect_literal_count", [_int], _int)
_bind("mimir_action_get_effect_literal", [_int, _int], _int)
_bind("mimir_action_get_conditional_effect_count", [_int], _int)
_bind("mimir_action_get_conditional_effect", [_int, _int], _int)
_bind("mimir_action_get_conditional_numeric_effect_count", [_int], _int)
_bind("mimir_action_get_conditional_numeric_effect", [_int, _int], _int)
_bind("mimir_action_to_string", [_int], _ptr)


# ----- Conditional effect (lifted) -----

_bind("mimir_conditional_effect_get_effect", [_int], _int)
_bind("mimir_conditional_effect_get_quantified_count", [_int], _int)
_bind("mimir_conditional_effect_get_quantified", [_int, _int], _int)
_bind("mimir_conditional_effect_get_fluent_condition_count", [_int], _int)
_bind("mimir_conditional_effect_get_fluent_condition", [_int, _int], _int)
_bind("mimir_conditional_effect_get_static_condition_count", [_int], _int)
_bind("mimir_conditional_effect_get_static_condition", [_int, _int], _int)
_bind("mimir_conditional_effect_get_derived_condition_count", [_int], _int)
_bind("mimir_conditional_effect_get_derived_condition", [_int, _int], _int)


# ----- Ground action -----

_bind("mimir_ground_action_get_schema", [_int], _int)
_bind("mimir_ground_action_get_cost", [_int], _dbl)
_bind("mimir_ground_action_get_argument_count", [_int], _int)
_bind("mimir_ground_action_get_argument", [_int, _int], _int)
_bind("mimir_ground_action_to_string", [_int], _ptr)
_bind("mimir_ground_action_is_applicable", [_int, _int], _int)
_bind("mimir_ground_action_apply", [_int, _int], _int)
_bind("mimir_ground_action_get_add_effect_count", [_int], _int)
_bind("mimir_ground_action_get_add_effect", [_int, _int], _int)
_bind("mimir_ground_action_get_delete_effect_count", [_int], _int)
_bind("mimir_ground_action_get_delete_effect", [_int, _int], _int)
_bind("mimir_ground_action_get_positive_fluent_precond_count", [_int], _int)
_bind("mimir_ground_action_get_positive_fluent_precond", [_int, _int], _int)
_bind("mimir_ground_action_get_negative_fluent_precond_count", [_int], _int)
_bind("mimir_ground_action_get_negative_fluent_precond", [_int, _int], _int)
_bind("mimir_ground_action_get_positive_static_precond_count", [_int], _int)
_bind("mimir_ground_action_get_positive_static_precond", [_int, _int], _int)
_bind("mimir_ground_action_get_negative_static_precond_count", [_int], _int)
_bind("mimir_ground_action_get_negative_static_precond", [_int, _int], _int)
_bind("mimir_ground_action_get_derived_precondition_count", [_int], _int)
_bind("mimir_ground_action_get_derived_precondition", [_int, _int], _int)
_bind("mimir_ground_action_get_conditional_effect_count", [_int], _int)
_bind("mimir_ground_action_get_conditional_effect", [_int, _int], _int)
_bind("mimir_ground_action_get_conditional_numeric_effect_count", [_int], _int)
_bind("mimir_ground_action_get_conditional_numeric_effect", [_int, _int], _int)


# ----- Ground conditional effect -----

_bind("mimir_ground_conditional_effect_get_effect", [_int], _int)
_bind("mimir_ground_conditional_effect_get_positive_fluent_count", [_int], _int)
_bind("mimir_ground_conditional_effect_get_positive_fluent", [_int, _int], _int)
_bind("mimir_ground_conditional_effect_get_negative_fluent_count", [_int], _int)
_bind("mimir_ground_conditional_effect_get_negative_fluent", [_int, _int], _int)
_bind("mimir_ground_conditional_effect_get_positive_static_count", [_int], _int)
_bind("mimir_ground_conditional_effect_get_positive_static", [_int, _int], _int)
_bind("mimir_ground_conditional_effect_get_negative_static_count", [_int], _int)
_bind("mimir_ground_conditional_effect_get_negative_static", [_int, _int], _int)
_bind("mimir_ground_conditional_effect_get_derived_count", [_int], _int)
_bind("mimir_ground_conditional_effect_get_derived", [_int, _int], _int)
_bind("mimir_ground_conditional_effect_is_satisfied", [_int, _int], _int)


# ----- State -----

_bind("mimir_state_get_fluent_atom_count", [_int], _int)
_bind("mimir_state_get_fluent_atom", [_int, _int], _int)
_bind("mimir_state_get_derived_atom_count", [_int], _int)
_bind("mimir_state_get_derived_atom", [_int, _int], _int)
_bind("mimir_state_contains_fluent", [_int, _int], _int)
_bind("mimir_state_contains_static", [_int, _int], _int)
_bind("mimir_state_contains_derived", [_int, _int], _int)
_bind("mimir_state_to_string", [_int], _ptr)
_bind("mimir_state_generate_applicable_actions", [_int, _int], _int)
_bind("mimir_state_generate_successors", [_int, _int, _ptr, _int], _int)
_bind("mimir_action_list_count", [_int], _int)
_bind("mimir_action_list_get", [_int, _int], _int)
