"""Unstable low-level ctypes bindings for Mimir.Interop.

This package exposes the raw C-callable surface as ``argtypes`` / ``restype``
declarations on a single shared :class:`ctypes.CDLL` instance. The public
:mod:`pymimir` modules build Pythonic objects on top.

This module is deliberately outside the public compatibility contract. Most
users should import from :mod:`pymimir` directly. Raw callback callers must keep
their ``ctypes`` callback objects alive for every native invocation and must
adopt or free every handle received by a callback.
"""

from . import builders, datasets, formalism, heuristics, learning, search
from ._native import free_handle, lib, relationship_value, take_string, value_equals, value_hash

__all__ = [
    "lib", "take_string", "free_handle", "value_equals", "value_hash", "relationship_value",
    "builders", "formalism", "search", "heuristics", "datasets", "learning",
]
