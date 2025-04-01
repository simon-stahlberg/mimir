import glob
import os
import sys
import subprocess
import multiprocessing
import shutil

from pathlib import Path

from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

__version__ = "0.12.1"
HERE = Path(__file__).resolve().parent


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        output_directory = ext_fullpath.parent.resolve()
        temp_directory = Path.cwd() / self.build_temp

        print("ext_fullpath", ext_fullpath)
        print("output_directory", output_directory)
        print("temp_directory", temp_directory)

        build_type = "Debug" if os.environ.get('PYMIMIR_DEBUG_BUILD') else "Release"
        print("Pymimir build type:", build_type)

        # Create the temporary build directory, if it does not already exist
        os.makedirs(temp_directory, exist_ok=True)

        # 1. Build and install dependencies and delete the build directory
        if os.path.exists(f"{ext.sourcedir}/dependencies/installs"):
            shutil.copytree(f"{ext.sourcedir}/dependencies/installs", temp_directory / "dependencies/installs", dirs_exist_ok=True)
            for file in glob.glob(f"{temp_directory}/**/CMakeCache.txt", recursive=True):
                print(f"Removing CMakeCache.txt: ", file)
                os.remove(file)

        cmake_args = [
            f"-DCMAKE_BUILD_TYPE={build_type}",
            f"-DCMAKE_INSTALL_PREFIX={str(temp_directory)}/dependencies/installs",
            f"-DCMAKE_PREFIX_PATH={str(temp_directory)}/dependencies/installs",
            f"-DPython_EXECUTABLE={sys.executable}"
        ]

        subprocess.run(
            ["cmake", "-S", f"{ext.sourcedir}/dependencies", "-B", f"{str(temp_directory)}/dependencies/build"] + cmake_args, cwd=str(temp_directory), check=True
        )

        subprocess.run(
            ["cmake", "--build", f"{str(temp_directory)}/dependencies/build", f"-j{multiprocessing.cpu_count()}"]
        )

        shutil.rmtree(f"{str(temp_directory)}/dependencies/build")

        # 2. Build mimir

        cmake_args = [
            "-DBUILD_PYMIMIR=ON",
            f"-DMIMIR_VERSION_INFO={__version__}",
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={output_directory}",
            f"-DCMAKE_BUILD_TYPE={build_type}",  # not used on MSVC, but no harm
            f"-DCMAKE_PREFIX_PATH={str(temp_directory)}/dependencies/installs",
            f"-DPython_EXECUTABLE={sys.executable}"
        ]
        build_args = []
        build_args += ["--target", ext.name]

        subprocess.run(
            ["cmake", "-S", ext.sourcedir, "-B", f"{str(temp_directory)}/build"] + cmake_args, cwd=str(temp_directory), check=True
        )
        subprocess.run(
            ["cmake", "--build", f"{str(temp_directory)}/build", f"-j{multiprocessing.cpu_count()}"] + build_args, cwd=str(temp_directory), check=True
        )

        print("Generate subgen dir:", output_directory)

        # 3. Generate pyi stub files
        subprocess.run(
            [sys.executable, '-m', 'nanobind.stubgen', '-m', '_pymimir', '-O', temp_directory, '-r'], cwd=output_directory, check=True
        )
        # The resulting structure is a bit weird:
        # _pymimir.so
        # _pymimir.pyi
        # advanced/__init__.pyi
        # advanced/common/__init__.pyi
        #
        # We adapt it while copying to install location:
        # _pymimir.so
        # pymimir/__init__.pyi
        # pymimir/advanced/__init__.pyi
        # pymimir/advanced/common/__init__.pyi

        # Define submodule list that must be copied to install location
        module_dirs = [
            "advanced",
            "advanced/common",
            "advanced/formalism",
            "advanced/graphs",
            "advanced/search",
            "advanced/datasets",
            "advanced/languages",
            "advanced/languages/description_logics"
        ]

        # Copy the _pyimir.pyi into pyimir/__init__.pyi
        shutil.copy(temp_directory / "_pymimir.pyi", output_directory / "pymimir" / "__init__.pyi")

        # Copy the submodule/__init__.pyi into pymimir/submodule/__init__.pyi
        for module_dir in module_dirs:
            os.makedirs(output_directory / module_dir, exist_ok=True)
            shutil.copy(temp_directory / module_dir / "__init__.pyi", output_directory / "pymimir" / module_dir / "__init__.pyi")


# The information here can also be placed in setup.cfg - better separation of
# logic and declaration, and simpler if you include description/version in a file.
setup(
    name="pymimir",
    version=__version__,
    author="Simon Stahlberg, Dominik Drexler",
    author_email="simon.stahlberg@gmail.com, dominik.drexler@liu.se",
    url="https://github.com/simon-stahlberg/mimir",
    description="Mimir planning library",
    long_description="",
    install_requires=["cmake>=3.21"],
    packages=find_packages(where="python/src"),
    package_dir={"": "python/src"},
    package_data={
        "": [],
    },
    ext_modules=[CMakeExtension("_pymimir")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    extras_require={
        'test': [
            'pytest',
        ],
    }
)
