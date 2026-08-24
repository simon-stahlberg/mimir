from pathlib import Path
import glob
import platform
import shutil
import subprocess
import sys

from setuptools import find_packages, setup
from setuptools.command.build_py import build_py
from setuptools.command.editable_wheel import editable_wheel

try:
    from setuptools.command.bdist_wheel import bdist_wheel
except ImportError:
    from wheel.bdist_wheel import bdist_wheel


ROOT = Path(__file__).resolve().parent


def _runtime_identifier():
    machine = platform.machine().lower()
    arch = "arm64" if machine in {"arm64", "aarch64"} else "x64"

    if sys.platform == "win32":
        return f"win-{arch}"
    if sys.platform == "darwin":
        return f"osx-{arch}"
    if sys.platform.startswith("linux"):
        return f"linux-{arch}"
    raise RuntimeError(f"Unsupported platform for Mimir native publish: {sys.platform}")


def _lib_filename():
    if sys.platform == "win32":
        return "Mimir.Interop.dll"
    if sys.platform == "darwin":
        return "Mimir.Interop.dylib"
    return "Mimir.Interop.so"


def _publish_native():
    rid = _runtime_identifier()
    project = ROOT / "src" / "Mimir.Interop" / "Mimir.Interop.csproj"
    subprocess.check_call([
        "dotnet",
        "publish",
        str(project),
        "-c",
        "Release",
        "-r",
        rid,
    ], cwd=ROOT)

    lib_name = _lib_filename()
    build_root = ROOT / "src" / "Mimir.Interop" / "bin" / "Release"
    matches = glob.glob(str(build_root / "**" / rid / "publish" / lib_name), recursive=True)
    if not matches:
        raise RuntimeError(f"Native publish succeeded but {lib_name} was not found under {build_root}")

    return Path(matches[0])


class build_py_with_native(build_py):
    def run(self):
        native_lib = _publish_native()
        package_dir = (Path(self.build_lib) / "pymimir").resolve()
        build_dir = (ROOT / "build").resolve()
        if not package_dir.is_relative_to(build_dir):
            raise RuntimeError(f"Refusing to clean unexpected build path: {package_dir}")
        if package_dir.exists():
            shutil.rmtree(package_dir)
        super().run()

        target_dir = package_dir / "advanced"
        target_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(native_lib, target_dir / native_lib.name)


class editable_wheel_with_native(editable_wheel):
    def run(self):
        _publish_native()
        super().run()


class bdist_wheel_with_native(bdist_wheel):
    def finalize_options(self):
        super().finalize_options()
        self.root_is_pure = False

    def get_tag(self):
        _, _, platform_tag = super().get_tag()
        # The bundled library is loaded through ctypes and does not use the CPython ABI.
        return "py3", "none", platform_tag


setup(
    name="pymimir",
    version="0.14.0b1",
    long_description=(ROOT / "README.md").read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    package_data={
        "pymimir": ["py.typed"],
        "pymimir.advanced": ["Mimir.Interop.dll", "Mimir.Interop.so", "Mimir.Interop.dylib"],
    },
    python_requires=">=3.10",
    install_requires=[],
    extras_require={
        "test": ["pytest>=8", "mypy>=1.11"],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
    ],
    zip_safe=False,
    cmdclass={
        "build_py": build_py_with_native,
        "bdist_wheel": bdist_wheel_with_native,
        "editable_wheel": editable_wheel_with_native,
    },
)
