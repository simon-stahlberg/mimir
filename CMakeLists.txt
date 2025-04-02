cmake_minimum_required(VERSION 3.21)
project(mimir VERSION 0.11.1)


##############################################################
# Language setup
##############################################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


##############################################################
# Options
##############################################################


##############################################################
# Build Targets
##############################################################

option(BUILD_PYMIMIR "Build" OFF)
option(BUILD_TESTS "Build" OFF)
option(BUILD_EXECUTABLES "Build" OFF)
option(BUILD_PROFILING "Build" OFF)

##############################################################
# Common Settings
##############################################################

# make cache variables for install destinations
include(GNUInstallDirs)

if(MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1 /EHsc /bigobj /MP")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /IGNORE:4006,4044,4075")
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif()
    # TODO: Add -Wextra and fix all warnings

    message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
    # if(COMPILER_ID MATCHES "GNU")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto -fcoroutines")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -ftime-trace")  #  -fcoroutines-ts seems outdated
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -DNDEBUG")
    set(CMAKE_CXX_FLAGS_PROFILING "-O3 -Wall -pg")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g3 -ggdb")
endif()

message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/")
add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
message("DATA_DIR: ${DATA_DIR}")

if(VERBOSE)
    add_compile_definitions(VERBOSE)
endif()

##############################################################
# CMake modules and macro files
##############################################################

list(APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake"
)
include("configure_boost")
include("configure_ccache")
include("configure_loki")

##############################################################
# CCache
##############################################################

# CCache
configure_ccache()


##############################################################
# Dependency Handling
##############################################################

# set(CMAKE_FIND_DEBUG_MODE TRUE)

# Boost
# Find Boost headers only according to https://cmake.org/cmake/help/latest/module/FindBoost.html
configure_boost()
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS iostreams PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  message(STATUS "Found Boost: ${Boost_DIR} (found version ${Boost_VERSION})")
endif()

find_package(ZLIB REQUIRED)

configure_loki()
find_package(loki ${LOKI_MIN_VERSION} COMPONENTS parsers REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

# TODO(Dominik): NO_DEFAULT_PATH does not work
find_package(Nauty REQUIRED)
if(Nauty_FOUND)
  include_directories(${Nauty_INCLUDE_DIR})
  message(STATUS "Found Nauty: ${NAUTY_LIBRARY} ${NAUTY_INCLUDE_DIR}")
endif()


find_package(fmt REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(fmt_FOUND)
  include_directories(${fmt_INCLUDE_DIRS})
  message(STATUS "Found fmt: ${fmt_DIR} (found version ${fmt_VERSION})")
endif()


find_package(absl CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(absl_FOUND)
  include_directories(${absl_INCLUDE_DIRS})
  message(STATUS "Found absl: ${absl_DIR} (found version ${absl_VERSION})")
endif()


##############################################################
# Add library and executable targets
##############################################################

# ------------
# Target Mimir
# ------------
add_subdirectory(src)

# -------------------
# Target Python Mimir
# -------------------
if(BUILD_PYMIMIR)
    add_subdirectory(python/src/pymimir)
endif()

# ----------
# Target Exe
# ----------
if (BUILD_EXECUTABLES)
    add_subdirectory(exe)
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



###########
# Install #
###########

# Install header files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/mimir" "${PROJECT_SOURCE_DIR}/include/cista"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# Install cmake scripts
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mimir/cmake"
)


###########
# Exports #
###########

# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html

include(CMakePackageConfigHelpers)

# Generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mimirConfigVersion.cmake"
    VERSION ${mimir_VERSION}
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
