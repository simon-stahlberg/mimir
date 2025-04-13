import glob
import os
import sys
import subprocess
import multiprocessing
import shutil

from pathlib import Path

from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext


__version__ = "0.12.11"
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

        #######################################################################
        # Build mimir
        #######################################################################

        cmake_args = [
            "-DBUILD_PYMIMIR=ON",
            f"-DMIMIR_VERSION_INFO={__version__}",
            f"-DCMAKE_BUILD_TYPE={build_type}",  # not used on MSVC, but no harm
            f"-DCMAKE_PREFIX_PATH={str(temp_directory)}/dependencies/installs",
            f"-DPython_EXECUTABLE={sys.executable}"
        ]

        subprocess.run(
            ["cmake", "-S", ext.sourcedir, "-B", f"{str(temp_directory)}/build"] + cmake_args, cwd=str(temp_directory), check=True
        )

        subprocess.run(
            ["cmake", "--build", f"{str(temp_directory)}/build", f"-j{multiprocessing.cpu_count()}"], cwd=str(temp_directory), check=True
        )

        subprocess.run(
            ["cmake", "--install", f"{str(temp_directory)}/build", "--prefix", f"{str(output_directory)}"], check=True
        )

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
    ext_modules=[CMakeExtension("_pymimir")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    extras_require={
        'test': [
            'pytest',
        ],
    }
)
