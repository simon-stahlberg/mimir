import os
import shutil
import subprocess
import sys

from pathlib import Path
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext
import multiprocessing
import sysconfig

# Get the extension suffix for the current Python version
extension_suffix = sysconfig.get_config_var("EXT_SUFFIX")


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

        print("Python VENV executable:", sys.executable)

        cmake_args = [
            f"-DBUILD_LIBMIMIR=OFF",
            f"-DBUILD_TESTS=OFF",
            f"-DBUILD_PYMIMIR=ON",
            f"-DBUILD_PROFILING=OFF",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE=Release",  # Is not used with MSVC, but causes no harm,
            f"-DPYTHON_VENV_EXE={sys.executable}",
            f"-DENABLE_IPO=ON"
        ]

        # Build package
        run_args = [
            os.path.join(Path.cwd(), "configure.sh"),
            "--conan",
            "--source",
            str(ext.sourcedir),
            "--output",
            str(temp_directory),
            *cmake_args,
        ]
        print(f"Cmake configuration script call: {' '.join(run_args)}", flush=True)

        subprocess.run(
            run_args,
            check=True,
        )

        print(os.listdir(str(temp_directory)), ext.sourcedir, sep="\n", flush=True)

        build_args = [
            "cmake",
            "--build",
            str(temp_directory),
            "--target",
            "pymimir",
            "--config",
            "Release",
            "--parallel",
            str(multiprocessing.cpu_count() - 1),
        ]

        print(f"Cmake build call: {' '.join(build_args)}", flush=True)
        subprocess.run(
            build_args,
            # cwd=temp_directory,
            check=True,  # Is used with MSVC
        )

        # Copy relevant files to output directory

        # Copy stub files
        if Path.exists(temp_directory / "pymimir-stubs"):
            shutil.copytree(
                temp_directory / "pymimir-stubs", output_directory / "pymimir-stubs"
            )
        elif Path.exists(temp_directory / "pymimir.pyi"):
            shutil.copy(
                temp_directory / "pymimir.pyi", output_directory / "pymimir.pyi"
            )

        # Copy the shared object file
        if Path.exists(temp_directory / f"pymimir{extension_suffix}"):
            shutil.copy(temp_directory / f"pymimir{extension_suffix}", output_directory / f"pymimir{extension_suffix}")
        else:
            raise FileNotFoundError(
                f"could not find 'pymimir{extension_suffix}' ({temp_directory})"
            )


setup(
    ext_modules=[CMakeExtension("mimir")],
    cmdclass={"build_ext": CMakeBuild},
)
