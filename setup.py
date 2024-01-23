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
            f"-DBUILD_LIBMIMIR=OFF",
            f"-DBUILD_TESTS=OFF",
            f"-DBUILD_PYMIMIR=ON",
            f"-DBUILD_PROFILING=OFF",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE=Release",  # Is not used with MSVC, but causes no harm
            f"-DCMAKE_PREFIX_PATH={str(temp_directory)}/dependencies/installs"
        ]

        # Build dependencies
        subprocess.run(
            ["cmake", "-S", f"{ext.sourcedir}/dependencies", "-B", f"{str(temp_directory)}/dependencies/build", f"-DCMAKE_INSTALL_PREFIX={str(temp_directory)}/dependencies/installs"], cwd=temp_directory, check=True
        )

        subprocess.run(
            ["cmake", "--build", f"{str(temp_directory)}/dependencies/build", "-j16"]
        )

        # Build package
        subprocess.run(
            ["cmake", "-S", ext.sourcedir, "-B", f"{str(temp_directory)}/build", *cmake_args], cwd=temp_directory, check=True
        )

        subprocess.run(
            ["cmake", "--build", f"{str(temp_directory)}/build", "-j16", "--config", "Release"], cwd=temp_directory, check=True  # --config Release is is used with MSVC
        )

        # Copy relevant files to output directory

        lib_directory = (temp_directory / "build") / "lib"

        # Copy stub files
        if Path.exists(lib_directory / "pymimir-stubs"):
            shutil.copytree(lib_directory / "pymimir-stubs", output_directory / "pymimir-stubs")
        elif Path.exists(lib_directory / "pymimir.pyi"):
            shutil.copy(lib_directory / "pymimir.pyi", output_directory / "pymimir.pyi")

        # Copy the shared object file
        if Path.exists(lib_directory / "pymimir.so"):
            shutil.copy(lib_directory / "pymimir.so", output_directory / "pymimir.so")
        elif Path.exists(lib_directory / "Release" / "pymimir.dll"):
            shutil.copy(lib_directory / "Release" / "pymimir.dll", output_directory / "pymimir.pyd")
        else:
            raise FileNotFoundError(f"could not find 'pymimir.so' or 'pymimir.dll' ({lib_directory})")


setup(
    name="pymimir",
    version="0.1.1",
    author="Simon Stahlberg",
    author_email="simon.stahlberg@gmail.com",
    description="Mimir: PDDL Parser and Planner Toolkit",
    long_description="",
    ext_modules=[CMakeExtension("mimir")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.7",
)
