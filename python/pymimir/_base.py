"""Native-handle base class and the term types shared by :mod:`pymimir.model` and :mod:`pymimir.numeric`."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypeVar

from .advanced import free_handle, lib, take_string, value_equals, value_hash
from .advanced._native import _Finalizer, _NativeOwner, _create_finalizer

if TYPE_CHECKING:
    from .model import Domain, Problem


def _required_string(ptr: int, what: str) -> str:
    """Decode a UTF-8 string returned by the native side, raising on NULL.

    The C# side never returns NULL for valid handles, so a null pointer here
    is a contract violation (stale handle, type mismatch) — not a recoverable
    empty-string condition.
    """
    s = take_string(ptr)
    if s is None:
        raise RuntimeError(f"native returned NULL string for {what}")
    return s


_TERM_KIND_OBJECT = 1
_TERM_KIND_VARIABLE = 2


# =============================================================================
# Base class for wrapper objects that own a native handle
# =============================================================================


_HandleT = TypeVar("_HandleT", bound="_Handle")


class _Handle(_NativeOwner):
    """Owns a native handle and frees it on garbage collection."""

    __slots__ = ("_handle", "_owner", "_finalizer", "__weakref__")

    _handle: int
    _owner: Domain | Problem | None
    _finalizer: _Finalizer | None

    def __init__(self) -> None:
        raise TypeError(f"{type(self).__name__} values are created by Domain or Problem")

    def _initialize(
        self,
        handle: int,
        owner: Domain | Problem | None = None,
    ) -> None:
        if handle == 0:
            raise RuntimeError(f"{type(self).__name__} got null handle")
        finalizer = _create_finalizer(self, free_handle, handle)
        try:
            self._handle = handle
            self._owner = owner
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise

    @classmethod
    def _allocate_for_handle(cls: type[_HandleT], handle: int) -> _HandleT:
        try:
            return object.__new__(cls)
        except BaseException:
            free_handle(handle)
            raise

    @classmethod
    def _from_handle(
        cls: type[_HandleT],
        handle: int,
        owner: Domain | Problem | None = None,
    ) -> _HandleT:
        value = cls._allocate_for_handle(handle)
        value._initialize(handle, owner)
        return value

    def __eq__(self, other: object) -> bool:
        return (
            type(self) is type(other)
            and isinstance(other, _Handle)
            and value_equals(self._handle, other._handle)
        )

    def __hash__(self) -> int:
        return value_hash(self._handle)


# =============================================================================
# Variable & Object
# =============================================================================


class Variable(_Handle):
    """A typed PDDL variable (e.g. ``?x``)."""

    @property
    def name(self) -> str:
        return _required_string(lib.mimir_variable_get_name(self._handle), "Variable.name")

    @property
    def type_name(self) -> str:
        return _required_string(lib.mimir_variable_get_type(self._handle), "Variable.type")

    def __str__(self) -> str:
        return self.name

    def __repr__(self) -> str:
        return f"Variable({self.name!r})"

class Object(_Handle):
    """A PDDL object (a.k.a. Constant)."""

    @property
    def name(self) -> str:
        return _required_string(lib.mimir_object_get_name(self._handle), "Object.name")

    @property
    def type_name(self) -> str:
        return _required_string(lib.mimir_object_get_type(self._handle), "Object.type")

    def __str__(self) -> str:
        return self.name

    def __repr__(self) -> str:
        return f"Object({self.name!r})"

Term = Object | Variable


def _wrap_term(handle: int, owner: Domain | Problem | None = None) -> Term:
    if handle == 0:
        raise RuntimeError("_wrap_term received null handle (contract violation)")
    kind = lib.mimir_term_get_kind(handle)
    if kind == _TERM_KIND_OBJECT:
        return Object._from_handle(handle, owner)
    if kind == _TERM_KIND_VARIABLE:
        return Variable._from_handle(handle, owner)
    free_handle(handle)
    raise RuntimeError(f"_wrap_term: unknown term kind code {kind}")
