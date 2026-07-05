import importlib
import sys
import sysconfig


def _is_free_threaded_build():
    return sysconfig.get_config_var("Py_GIL_DISABLED") not in (None, 0, "0", False)


def _assert_gil_disabled(context):
    is_gil_enabled = getattr(sys, "_is_gil_enabled", None)
    if is_gil_enabled is None:
        raise AssertionError(f"free-threaded build lacks sys._is_gil_enabled before {context}")
    if is_gil_enabled():
        raise AssertionError(f"GIL is enabled {context}")


if _is_free_threaded_build():
    _assert_gil_disabled("importing pymimir.advanced")

importlib.import_module("pymimir.advanced")

if _is_free_threaded_build():
    _assert_gil_disabled("importing pymimir.advanced")
