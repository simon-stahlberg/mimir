import os
import shutil
import subprocess
import sys

from pathlib import Path
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        output_directory = ext_fullpath.parent.resolve()
        temp_directory = Path.cwd() / self.build_temp

        # Ensure that the output directory is empty
        shutil.rmtree(output_directory, ignore_errors=True)
        os.makedirs(output_directory)

        # Create the temporary build directory, if it does not already exist
        os.makedirs(temp_directory, exist_ok=True)

        cmake_args = [
            f"-DBUILD_TESTS=OFF",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE=Release",  # Is not used with MSVC, but causes no harm
        ]

        # Build package
        subprocess.run(
            ["cmake", ext.sourcedir, *cmake_args], cwd=temp_directory, check=True
        )

        subprocess.run(
            ["cmake", "--build", ".", "--config", "Release"], cwd=temp_directory, check=True  # Is used with MSVC
        )

        # Copy relevant files to output directory

        # Copy stub files
        if Path.exists(temp_directory / "mimir-stubs"):
            shutil.copytree(temp_directory / "mimir-stubs", output_directory / "mimir-stubs")
        elif Path.exists(temp_directory / "mimir.pyi"):
            shutil.copy(temp_directory / "mimir.pyi", output_directory / "mimir.pyi")

        # Copy the shared object file
        if Path.exists(temp_directory / "mimir.so"):
            shutil.copy(temp_directory / "mimir.so", output_directory / "mimir.so")
        elif Path.exists(temp_directory / "Release" / "mimir.dll"):
            shutil.copy(temp_directory / "Release" / "mimir.dll", output_directory / "mimir.pyd")
        else:
            raise FileNotFoundError(f"could not find 'mimir.so' or 'mimir.dll' ({temp_directory})")


setup(
    name="mimir",
    version="0.1.0",
    author="Simon Stahlberg",
    author_email="simon.stahlberg@gmail.com",
    description="Mimir: PDDL Parser and Planner Toolkit",
    long_description="",
    ext_modules=[CMakeExtension("mimir")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.7",
)
