"""Native library loader for Mimir.Interop (NativeAOT-published shared library).

Module attribute :data:`lib` exposes the loaded :class:`ctypes.CDLL` instance.
All ``advanced.*`` modules attach their function signatures to ``lib`` lazily.
"""

from __future__ import annotations

import ctypes
import glob
import os
import platform
import sys
import weakref
from collections.abc import Sequence
from typing import Callable, List, NoReturn, Optional, Protocol, SupportsIndex, Tuple, TypeVar, cast


_NativeOwnerT = TypeVar("_NativeOwnerT", bound="_NativeOwner")


class _Finalizer(Protocol):
    """Stable local view of ``weakref.finalize`` across typeshed versions."""

    @property
    def alive(self) -> bool: ...

    def __call__(self, _ignored: object = None) -> object | None: ...

    def detach(self) -> object | None: ...


class _NativeOwner:
    __slots__ = ()

    def __copy__(self: _NativeOwnerT) -> _NativeOwnerT:
        return self

    def __deepcopy__(
        self: _NativeOwnerT,
        memo: dict[int, object],
    ) -> _NativeOwnerT:
        memo[id(self)] = self
        return self

    def __reduce_ex__(self, _protocol: SupportsIndex, /) -> NoReturn:
        raise TypeError(f"{type(self).__name__} values cannot be pickled")


def _lib_filename() -> str:
    if sys.platform == "win32":
        return "Mimir.Interop.dll"
    if sys.platform == "darwin":
        return "Mimir.Interop.dylib"
    return "Mimir.Interop.so"


def _runtime_identifier() -> str:
    machine = platform.machine().lower()
    arch = "arm64" if machine in {"arm64", "aarch64"} else "x64"

    if sys.platform == "win32":
        return f"win-{arch}"
    if sys.platform == "darwin":
        return f"osx-{arch}"
    if sys.platform.startswith("linux"):
        return f"linux-{arch}"
    return f"{sys.platform}-{arch}"


def _find_lib() -> Tuple[str, List[str]]:
    lib_name = _lib_filename()
    searched: List[str] = []

    # 1) Same directory as this module (when installed as a wheel)
    here = os.path.dirname(os.path.abspath(__file__))
    pkg_local = os.path.join(here, lib_name)
    searched.append(pkg_local)
    if os.path.exists(pkg_local):
        return pkg_local, searched

    # 2) Development tree: src/Mimir.Interop/bin/Release/net*/<rid>/publish/*.so
    repo_root = os.path.normpath(os.path.join(here, "..", "..", ".."))
    build_root = os.path.join(repo_root, "src", "Mimir.Interop", "bin", "Release")
    if os.path.isdir(build_root):
        rid = _runtime_identifier()
        rid_pattern = os.path.join(build_root, "**", rid, "publish", lib_name)
        searched.append(rid_pattern)
        rid_matches = glob.glob(rid_pattern, recursive=True)
        if rid_matches:
            return rid_matches[0], searched

    searched.append(lib_name)
    return lib_name, searched  # let ctypes search PATH / LD_LIBRARY_PATH / DYLD_LIBRARY_PATH


def _load_library() -> ctypes.CDLL:
    path, searched = _find_lib()
    try:
        return ctypes.cdll.LoadLibrary(path)
    except OSError as exc:
        rid = _runtime_identifier()
        searched_text = "\n  - ".join(searched)
        raise RuntimeError(
            f"Could not load Mimir native library from {path!r}. "
            f"Expected runtime identifier: {rid}. Searched:\n  - {searched_text}\n"
            "If installing from source, run: "
            f"dotnet publish src/Mimir.Interop/Mimir.Interop.csproj -c Release -r {rid}"
        ) from exc


lib: ctypes.CDLL = _load_library()


try:
    lib.mimir_abi_version.argtypes = []
    lib.mimir_abi_version.restype = ctypes.c_int
except AttributeError as exc:
    raise RuntimeError(
        "Loaded Mimir native library does not expose mimir_abi_version. "
        "Rebuild it with: dotnet publish src/Mimir.Interop/Mimir.Interop.csproj -c Release -r <rid>"
    ) from exc

if lib.mimir_abi_version() != 24:
    raise RuntimeError(
        "Loaded Mimir native library has an incompatible ABI. "
        "Rebuild it with: dotnet publish src/Mimir.Interop/Mimir.Interop.csproj -c Release -r <rid>"
    )


# -- Core handle/string helpers (always needed) --

lib.mimir_free_handle.argtypes = [ctypes.c_int]
lib.mimir_free_handle.restype = None

lib.mimir_free_string.argtypes = [ctypes.c_void_p]
lib.mimir_free_string.restype = None


class _NativeError(ctypes.Structure):
    _fields_ = [
        ("code", ctypes.c_int),
        ("document_type", ctypes.c_int),
        ("error_code", ctypes.c_int),
        ("offset", ctypes.c_int),
        ("length", ctypes.c_int),
        ("line", ctypes.c_int),
        ("column", ctypes.c_int),
        ("message", ctypes.c_void_p),
        ("source_path", ctypes.c_void_p),
    ]


lib.mimir_take_last_error.argtypes = [ctypes.POINTER(_NativeError)]
lib.mimir_take_last_error.restype = ctypes.c_byte

lib.mimir_value_equals.argtypes = [ctypes.c_int, ctypes.c_int]
lib.mimir_value_equals.restype = ctypes.c_int

lib.mimir_value_hash.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_int)]
lib.mimir_value_hash.restype = ctypes.c_byte


def take_string(ptr: int) -> Optional[str]:
    """Take ownership of a UTF8 char* returned by the native library.

    Decodes it into a Python ``str`` and frees the underlying memory. Returns
    ``None`` if the pointer is null.
    """
    if not ptr:
        return None
    try:
        value = ctypes.cast(ptr, ctypes.c_char_p).value
        if value is None:
            raise RuntimeError("non-null native string pointer contained NULL")
        return value.decode("utf-8")
    finally:
        lib.mimir_free_string(ptr)


def free_handle(handle: int) -> None:
    if handle:
        lib.mimir_free_handle(handle)


def _create_finalizer(
    owner: object,
    callback: Callable[..., object],
    *args: object,
) -> _Finalizer:
    try:
        return cast(_Finalizer, weakref.finalize(owner, callback, *args))
    except BaseException:
        callback(*args)
        raise


def value_equals(left_handle: int, right_handle: int) -> bool:
    result = int(lib.mimir_value_equals(left_handle, right_handle))
    if result < 0:
        raise ValueError("cannot compare invalid native values")
    return result == 1


def value_hash(handle: int) -> int:
    result = ctypes.c_int()
    if not lib.mimir_value_hash(handle, ctypes.byref(result)):
        raise ValueError("cannot hash an invalid native value")
    return int(result.value)


def relationship_value(result: int, what: str) -> bool:
    value = int(result)
    if value < 0:
        raise ValueError(what)
    return value == 1


def raise_last_error(default_message: str) -> None:
    """Raise the structured native error associated with the current thread."""
    error = _NativeError()
    if not lib.mimir_take_last_error(ctypes.byref(error)):
        raise ValueError(default_message)

    _raise_native_error(error, default_message)


def _raise_native_error(error: _NativeError, default_message: str) -> None:
    message = take_string(error.message) or default_message
    source_path = take_string(error.source_path)
    if error.code == 1:
        raise ReferenceError(message)
    if error.code == 2:
        raise ValueError(message)
    if error.code == 8:
        from ..errors import UnsupportedError

        raise UnsupportedError(message)
    if error.code == 7:
        from ..errors import StateSpaceLimitExceeded

        raise StateSpaceLimitExceeded(message)
    if error.code == 4:
        from ..errors import PddlError

        document_types = {0: "domain", 1: "problem"}
        error_codes = {
            0: "lexical",
            1: "syntax",
            2: "unknown_construct",
            3: "unsupported_feature",
            4: "duplicate_section",
            5: "missing_section",
            6: "invalid_number",
            7: "validation",
        }
        if error.document_type not in document_types:
            raise RuntimeError(f"unknown PDDL document type code: {error.document_type}")
        if error.error_code not in error_codes:
            raise RuntimeError(f"unknown PDDL error code: {error.error_code}")
        raise PddlError(
            message,
            document_type=document_types[error.document_type],
            error_code=error_codes[error.error_code],
            source_path=source_path,
            offset=None if error.offset < 0 else error.offset,
            length=None if error.length < 0 else error.length,
            line=None if error.line < 0 else error.line,
            column=None if error.column < 0 else error.column,
        )

    from ..errors import MimirError
    raise MimirError(message)


def native_errcheck(
    result: object,
    function: object,
    _arguments: tuple[object, ...],
) -> object:
    """ctypes errcheck used by every ABI function bound by advanced modules."""
    error = _NativeError()
    if lib.mimir_take_last_error(ctypes.byref(error)):
        function_name = getattr(function, "__name__", None)
        if not isinstance(function_name, str):
            raise TypeError("native function has no string __name__")
        _raise_native_error(error, f"native call {function_name} failed")
    return result


def bind_function(
    name: str,
    argtypes: Sequence[object],
    restype: object,
) -> None:
    function = getattr(lib, name)
    function.argtypes = argtypes
    function.restype = restype
    function.errcheck = native_errcheck
