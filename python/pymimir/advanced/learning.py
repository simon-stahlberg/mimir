"""ctypes signatures for the native :mod:`pymimir.learning` surface."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind


_int = ctypes.c_int
_byte = ctypes.c_byte
_ptr = ctypes.c_void_p
_cstr = ctypes.c_char_p


_bind("mimir_learning_encoding_context_create", [], _int)
_bind("mimir_learning_encoding_context_begin_instance", [_int, _int], _byte)
_bind("mimir_learning_encoding_context_end_instance", [_int], _byte)
_bind("mimir_learning_encoding_context_get_batch_count", [_int], _int)
_bind("mimir_learning_encoding_context_get_node_count", [_int], _int)
_bind("mimir_learning_encoding_context_copy_node_sizes", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_object_sizes", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_object_indices", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_action_sizes", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_action_indices", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_virtual_sizes", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_virtual_indices", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_auxiliary_sizes", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_auxiliary_indices", [_int, _ptr, _int], _int)

_bind("mimir_learning_encoding_context_get_current_object_count", [_int], _int)
_bind("mimir_learning_encoding_context_get_current_node_offset", [_int], _int)
_bind("mimir_learning_encoding_context_get_current_node_count", [_int], _int)
_bind("mimir_learning_encoding_context_get_current_action_count", [_int], _int)
_bind("mimir_learning_encoding_context_get_current_virtual_count", [_int], _int)
_bind("mimir_learning_encoding_context_get_current_auxiliary_count", [_int], _int)
_bind("mimir_learning_encoding_context_copy_current_object_ids", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_current_action_ids", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_current_virtual_ids", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_copy_current_auxiliary_ids", [_int, _ptr, _int], _int)
_bind("mimir_learning_encoding_context_get_object_id", [_int, _int], _int)
_bind("mimir_learning_encoding_context_new_action_id", [_int], _int)
_bind("mimir_learning_encoding_context_new_virtual_id", [_int], _int)
_bind("mimir_learning_encoding_context_new_or_existing_virtual_id", [_int], _int)
_bind("mimir_learning_encoding_context_new_auxiliary_id", [_int], _int)
_bind(
    "mimir_learning_encoding_context_new_or_existing_object_pair_id",
    [_int, _int, _int],
    _int,
)
_bind(
    "mimir_learning_encoding_context_try_get_object_pair_id",
    [_int, _int, _int],
    _int,
)

_bind("mimir_learning_encoding_context_get_relation_count", [_int], _int)
_bind("mimir_learning_encoding_context_get_relation_name", [_int, _int], _ptr)
_bind("mimir_learning_encoding_context_get_relation_value_count", [_int, _int], _int)
_bind(
    "mimir_learning_encoding_context_copy_relation_values",
    [_int, _int, _ptr, _int],
    _int,
)
_bind(
    "mimir_learning_encoding_context_copy_all_relation_values",
    [_int, _ptr, _int],
    _int,
)

_bind("mimir_learning_encode_state", [_int, _int, _cstr], _byte)
_bind("mimir_learning_encode_goal", [_int, _int, _ptr, _int, _cstr], _byte)
_bind(
    "mimir_learning_encode_action_list",
    [_int, _int, _ptr, _int, _cstr],
    _byte,
)
_bind(
    "mimir_learning_encode_transition_effects",
    [_int, _int, _ptr, _int, _ptr, _int, _ptr, _int, _cstr],
    _byte,
)
_bind("mimir_learning_encode_virtual_node", [_int], _byte)
_bind("mimir_learning_encode_expressive_state", [_int, _int, _cstr], _byte)
_bind(
    "mimir_learning_encode_expressive_goal",
    [_int, _int, _ptr, _int, _cstr],
    _byte,
)


__all__: list[str] = []
