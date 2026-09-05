"""ctypes signatures for heuristic-related Mimir.Interop exports."""

from __future__ import annotations

import ctypes

from ._native import bind_function as _bind

_int = ctypes.c_int
_dbl = ctypes.c_double
_byte = ctypes.c_byte
_ptr = ctypes.c_void_p

_bind("mimir_heuristic_blind", [], _int)
_bind("mimir_heuristic_goal_count", [_int], _int)
_bind("mimir_heuristic_lifted_ff", [_int], _int)
_bind("mimir_heuristic_ff_grounded", [_int], _int)
_bind("mimir_heuristic_perfect", [_int, _int], _int)
_bind("mimir_heuristic_add", [_int], _int)
_bind("mimir_heuristic_max", [_int], _int)
_bind("mimir_heuristic_set_add", [_int], _int)
_bind("mimir_heuristic_h2", [_int], _int)
_bind("mimir_heuristic_evaluate", [_int, _int, _int], _dbl)
_bind("mimir_heuristic_evaluate_batch", [_int, _ptr, _int, _int, _ptr], _byte)
_bind("mimir_heuristic_get_preferred_actions", [_int, _int, _int, _int], _int)
_bind("mimir_heuristic_callback", [_int, _ptr, _ptr, _ptr], _int)

HEURISTIC_CALLBACK = ctypes.CFUNCTYPE(_byte, _int, ctypes.POINTER(_dbl))
PREFERRED_CALLBACK = ctypes.CFUNCTYPE(_byte, _int, _int, ctypes.POINTER(_byte))
QHEURISTIC_CALLBACK = ctypes.CFUNCTYPE(
    _byte, ctypes.POINTER(_int), ctypes.POINTER(_int), _int, ctypes.POINTER(_dbl)
)

HEURISTIC_BATCH_CALLBACK = ctypes.CFUNCTYPE(_byte, ctypes.POINTER(_int), _int, ctypes.POINTER(_dbl))
