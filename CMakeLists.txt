cmake_minimum_required(VERSION 3.21)
project(mimir VERSION 0.1.1)


##############################################################
# Build Targets
##############################################################

option(BUILD_PYMIMIR "Build" OFF)
option(BUILD_TESTS "Build" OFF)
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
    # TODO: Add -Wextra and fix all warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -DNDEBUG")
    set(CMAKE_CXX_FLAGS_PROFILING "-O3 -Wall -pg")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g3 -ggdb")
endif()

message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")


##############################################################
# CMake modules and macro files
##############################################################

list(APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake"
)
include("configure_boost")
include("configure_ccache")


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
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  message(STATUS "Found Boost: ${Boost_DIR} (found version ${Boost_VERSION})")
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
    # add_subdirectory(tests)
endif()


###########
# Install #
###########

# Install header files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/mimir"
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
