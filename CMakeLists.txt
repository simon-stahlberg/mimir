cmake_minimum_required(VERSION 3.18)
project(mimir VERSION 0.1.1)

# -------------
# Build Targets
# -------------
option(BUILD_LIBMIMIR "Build" ON)
option(BUILD_PYMIMIR "Build" OFF)
option(BUILD_TESTS "Build" ON)
option(BUILD_PROFILING "Build" ON)

if (BUILD_PYMIMIR OR BUILD_TESTS OR BUILD_PROFILING)
  set(BUILD_LIBMIMIR ON)
endif()

# ---------------
# Common Settings
# ---------------

# make cache variables for install destinations
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake"
)
include("configure_boost")
include("configure_ccache")

if(MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1 /EHsc /bigobj /MP")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /IGNORE:4006,4044,4075")
else()
    # TODO: Add -Wextra and fix all warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -DNDEBUG")
    set(CMAKE_CXX_FLAGS_PROFILING "-O3 -Wall -pg")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g3 -ggdb")
endif()

message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")


# -------------------
# Dependency handling
# -------------------

# We installed the indirect (transitive) dependencies into the subdirectories under the install prefix.
# Hence must append them to the single cmake_prefix_path.
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
if(PREFIX_PATH_COUNT GREATER 1)
    message(FATAL_ERROR "Only one prefix path is allowed. Found multiple paths in CMAKE_PREFIX_PATH. Please add dependencies to the CMake Superbuild.")
endif()
set(ORIGINAL_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
list(LENGTH ORIGINAL_CMAKE_PREFIX_PATH PREFIX_PATH_COUNT)
list(APPEND CMAKE_PREFIX_PATH
    "${ORIGINAL_CMAKE_PREFIX_PATH}/loki-dependencies")
message(STATUS "MODIFIED_CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
# set(CMAKE_FIND_DEBUG_MODE ON)


# -----------------------------
# CMake modules and macro files
# -----------------------------

# CCache
configure_ccache()

# Boost
# Find Boost headers only according to https://cmake.org/cmake/help/latest/module/FindBoost.html
configure_boost()
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()


# ------------
# Target Mimir
# ------------
if(BUILD_LIBMIMIR)
    add_subdirectory(src)
endif()

# -------------------
# Target Python Mimir
# -------------------
if(BUILD_PYMIMIR)
    add_subdirectory(lib)
endif()

# ----------------
# Target Profiling
# ----------------
if(BUILD_PROFILING)
    add_subdirectory(benchmark)
endif()

# -----------
# Target Test
# -----------
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()


# -------
# Install
# -------

# Install header files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/mimir"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install cmake scripts
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mimir/cmake"
)


# -------
# Exports
# -------

# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html

include(CMakePackageConfigHelpers)

# Generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mimirConfigVersion.cmake"
    VERSION "${mimir_VERSION}"
    COMPATIBILITY ExactVersion
)

# Create config file
# https://cmake.org/cmake/help/book/mastering-cmake/cmake/Help/guide/importing-exporting/index.html
# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-configuration-file
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mimirConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mimir"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Install config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mimirConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mimirConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mimir"
)