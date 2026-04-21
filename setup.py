import os
import sys
import subprocess
import multiprocessing
import shutil

from pathlib import Path

from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext


__version__ = "0.13.62"
HERE = Path(__file__).resolve().parent


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = Path(os.path.abspath(sourcedir))


def get_cmake_generator_args():
    generator = os.environ.get("CMAKE_GENERATOR")
    if generator:
        generator_args = [f"-G{generator}"]
        make_program = None
        if generator == "Ninja":
            ninja_executable = shutil.which("ninja")
            if ninja_executable:
                generator_args.append(f"-DCMAKE_MAKE_PROGRAM={ninja_executable}")
                make_program = ninja_executable
        print("Using CMake generator from environment:", generator)
        return generator, generator_args, make_program

    ninja_executable = shutil.which("ninja")
    if ninja_executable:
        print("Using Ninja generator:", ninja_executable)
        return "Ninja", ["-GNinja", f"-DCMAKE_MAKE_PROGRAM={ninja_executable}"], ninja_executable

    return None, [], None


def get_cached_cmake_generator(build_directory: Path):
    cache_file = build_directory / "CMakeCache.txt"
    if not cache_file.exists():
        return None

    with cache_file.open() as file:
        for line in file:
            if line.startswith("CMAKE_GENERATOR:INTERNAL="):
                return line.partition("=")[2].strip()

    return None


def get_cached_cmake_value(build_directory: Path, key: str):
    cache_file = build_directory / "CMakeCache.txt"
    if not cache_file.exists():
        return None

    prefix = f"{key}:"
    with cache_file.open() as file:
        for line in file:
            if line.startswith(prefix):
                return line.partition("=")[2].strip()

    return None


def get_launcher_executable(cached_launcher: str):
    return cached_launcher.split(";", 1)[0] if cached_launcher else None


def get_stale_cmake_cache_reason(build_directory: Path, expected_generator, expected_make_program, current_cmake_executable):
    if not build_directory.exists():
        return None

    cached_generator = get_cached_cmake_generator(build_directory)
    if expected_generator is not None and cached_generator is not None and cached_generator != expected_generator:
        return f"generator changed from {cached_generator} to {expected_generator}"

    cached_make_program = get_cached_cmake_value(build_directory, "CMAKE_MAKE_PROGRAM")
    if cached_make_program and not Path(cached_make_program).exists():
        return f"cached make program no longer exists: {cached_make_program}"
    if expected_make_program and cached_make_program and cached_make_program != expected_make_program:
        return f"cached make program changed from {cached_make_program} to {expected_make_program}"

    for launcher_key in ["CMAKE_C_COMPILER_LAUNCHER", "CMAKE_CXX_COMPILER_LAUNCHER"]:
        cached_launcher = get_cached_cmake_value(build_directory, launcher_key)
        if not cached_launcher:
            continue

        launcher_executable = get_launcher_executable(cached_launcher)
        if launcher_executable and not Path(launcher_executable).exists():
            return f"cached launcher executable no longer exists: {launcher_executable}"
        if current_cmake_executable and launcher_executable and launcher_executable != current_cmake_executable:
            return f"cached launcher executable changed from {launcher_executable} to {current_cmake_executable}"

    return None


def clear_stale_cmake_cache(build_directory: Path, expected_generator, expected_make_program, current_cmake_executable):
    reason = get_stale_cmake_cache_reason(build_directory, expected_generator, expected_make_program, current_cmake_executable)
    if reason is None:
        return

    print(f"Removing stale CMake build directory {build_directory} because {reason}.")
    shutil.rmtree(build_directory)


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

        generator, generator_args, make_program = get_cmake_generator_args()
        current_cmake_executable = shutil.which("cmake")
        dependency_build_directory = temp_directory / "dependencies" / "build"
        main_build_directory = temp_directory / "build"

        clear_stale_cmake_cache(dependency_build_directory, generator, make_program, current_cmake_executable)
        clear_stale_cmake_cache(main_build_directory, generator, make_program, current_cmake_executable)

        cmake_args = [
            f"-DCMAKE_BUILD_TYPE={build_type}",
            f"-DCMAKE_INSTALL_PREFIX={str(temp_directory / 'dependencies' / 'installs')}",
            f"-DCMAKE_PREFIX_PATH={str(temp_directory / 'dependencies' / 'installs')}",
            f"-DPython_EXECUTABLE={sys.executable}"
        ]

        subprocess.run(
            ["cmake", "-S", f"{str(ext.sourcedir / 'dependencies')}", "-B", f"{str(dependency_build_directory)}"] + generator_args + cmake_args,
            cwd=str(temp_directory),
            check=True,
        )

        subprocess.run(
            ["cmake", "--build", f"{str(dependency_build_directory)}", f"-j{multiprocessing.cpu_count()}"]
        )

        subprocess.run(
            ["cmake", "--install", f"{str(dependency_build_directory)}" ]
        )

        shutil.rmtree(str(dependency_build_directory))

        #######################################################################
        # Build mimir
        #######################################################################

        cmake_args = [
            "-DBUILD_PYMIMIR=ON",
            "-DMIMIR_BUILD_SHARED_CORE=ON",
            "-DCMAKE_INSTALL_LIBDIR=lib",
            "-DCMAKE_INSTALL_BINDIR=bin",
            "-DCMAKE_INSTALL_INCLUDEDIR=include",
            f"-DMIMIR_VERSION_INFO={__version__}",
            f"-DCMAKE_BUILD_TYPE={build_type}",  # not used on MSVC, but no harm
            f"-DCMAKE_PREFIX_PATH={str(temp_directory / 'dependencies' / 'installs')}",
            f"-DPython_EXECUTABLE={sys.executable}"
        ]

        subprocess.run(
            ["cmake", "-S", ext.sourcedir, "-B", f"{str(main_build_directory)}" ] + generator_args + cmake_args,
            cwd=str(temp_directory),
            check=True,
        )

        subprocess.run(
            ["cmake", "--build", f"{str(main_build_directory)}", f"-j{multiprocessing.cpu_count()}"], cwd=str(temp_directory), check=True
        )

        install_cmd = ["cmake", "--install", f"{str(main_build_directory)}", "--prefix", f"{str(output_directory / 'pymimir')}"]
        # Reduce wheel size: strip debug symbols from installed binaries when supported.
        # (Manylinux builds often inject -g via environment CFLAGS/CXXFLAGS.)
        if build_type != "Debug" and os.name != "nt":
            install_cmd.append("--strip")

        subprocess.run(install_cmd, check=True)

        # Remove unwanted directories.
        unwanted_dirs = ["include", "lib", "lib64"]
        for unwanted_dir in unwanted_dirs:
            dir_path = output_directory / unwanted_dir
            if os.path.exists(dir_path):
                shutil.rmtree(dir_path)
                print(f"Removed {dir_path} from the wheel.")


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
    ext_modules=[CMakeExtension("pymimir")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    extras_require={
        "test": [
            "pytest",
        ],
    }
)
