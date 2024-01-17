cmake_minimum_required(VERSION 3.18)
project(mimir VERSION 0.1.1 LANGUAGES CXX C)

# -------------
# Build Targets
# -------------
option(BUILD_LIBMIMIR "Build" ON)
option(BUILD_PYMIMIR "Build" ON)
option(BUILD_TESTS "Build" ON)
option(BUILD_PROFILING "Build" ON)

option(ENABLE_CACHE "Enable cache if available" ON)
option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
option(ENABLE_IPO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" ON)
option(ENABLE_PCH "Enable Precompiled Headers" ON)
option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" OFF)
option(ENABLE_WARNING_PADDING "Warn when structs are memory padded" OFF)

option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
# ---------------
# Common Settings
# ---------------
set(PROJ_CXX_STD_FEATURE cxx_std_20)
set(PROJ_CXX_STANDARD C++20)

# if cmake is called within a virtual env then we want to use that python interpreter to run e.g. stub-gen
# This exectuable has to be passed in though! (-DPTYHON_VENV_EXE=...)
set(PYTHON_VENV_EXECUTABLE)

message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

file(GLOB_RECURSE MIMIR_SRC_FILES "src/*.cpp" "src/**/*.cpp")
file(GLOB_RECURSE MIMIR_TEST_FILES "test/*.cpp" "test/**/*.cpp")


# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

target_compile_features(project_options INTERFACE ${PROJ_CXX_STD_FEATURE})

# set the compiler options
include(cmake/settings/compiler_options.cmake)
set_project_compiler_options(project_options)
# include target printing utilities (property printing)
include(cmake/settings/utilities.cmake)
# set the standard project settings
include(cmake/settings/standard_project_settings.cmake)
# enable cache system
include(cmake/settings/cache.cmake)
# standard compiler warnings
include(cmake/settings/compiler_warnings.cmake)
set_project_warnings(project_warnings)
# sanitizer options if supported by compiler
include(cmake/settings/sanitizers.cmake)
enable_sanitizers(project_options)
# load extra clang dev tools
include(cmake/settings/clang-cxx-dev-tools.cmake)
# enable cache system
include(cmake/settings/cache.cmake)
# allow for static analysis options
include(cmake/settings/static_analyzers.cmake)

find_package(Boost REQUIRED)

include(cmake/targets/mimir.cmake)
include(cmake/targets/pymimir.cmake)
include(cmake/targets/test.cmake)
include(cmake/targets/profiling.cmake)


# This prints a summary of found dependencies
include(FeatureSummary)
feature_summary(WHAT ALL)
