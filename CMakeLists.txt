cmake_minimum_required(VERSION 3.18)
project(mimir VERSION 0.1.1)

# -------------
# Build Targets
# -------------
option(BUILD_LIBMIMIR "Build" ON)
option(BUILD_PYMIMIR "Build" OFF)
option(BUILD_TESTS "Build" ON)
option(BUILD_PROFILING "Build" ON)

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

# CCache
configure_ccache()

# Boost
# Find Boost headers only according to https://cmake.org/cmake/help/latest/module/FindBoost.html
configure_boost()
# set(CMAKE_FIND_DEBUG_MODE ON)
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Loki
find_package(loki 0.1 COMPONENTS parsers REQUIRED)


file(GLOB_RECURSE MIMIR_SRC_FILES "src/*.cpp" "src/**/*.cpp")
file(GLOB_RECURSE MIMIR_PRIVATE_HEADER_FILES "src/*.hpp" "src/**/*.hpp")
file(GLOB_RECURSE MIMIR_PUBLIC_HEADER_FILES "include/*.hpp" "include/**/*.hpp")
file(GLOB_RECURSE MIMIR_TEST_FILES "test/*.cpp" "test/**/*.cpp")

# ------------
# Target Mimir
# ------------
if(BUILD_LIBMIMIR)
    add_library(mimir STATIC lib/mimir.cpp ${MIMIR_SRC_FILES} ${MIMIR_PRIVATE_HEADER_FILES} ${MIMIR_HEADER_FILES})
    set_property(TARGET mimir PROPERTY CXX_STANDARD 17)
    target_link_libraries(mimir loki::parsers)

    target_include_directories(mimir
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    install(
        TARGETS mimir
        EXPORT MimirTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(DIRECTORY include/ DESTINATION include)
    export(TARGETS mimir NAMESPACE mimir:: FILE MimirConfig.cmake)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "MimirConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    install(
        EXPORT MimirTargets
        FILE MimirTargets.cmake
        NAMESPACE mimir::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/loki"
    )

    install(
        EXPORT MimirTargets
        FILE MimirConfig.cmake
        NAMESPACE mimir::
        DESTINATION lib/cmake/mimir
    )

    install(
        EXPORT MimirTargets
        FILE MimirConfigVersion.cmake
        NAMESPACE mimir::
        DESTINATION lib/cmake/mimir
    )
endif()

# -------------------
# Target Python Mimir
# -------------------
if(BUILD_PYMIMIR)
    find_package(Python REQUIRED)
    find_package(PythonLibs REQUIRED)
    set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
    include_directories(${PYTHON_INCLUDE_DIRS})
    add_library(pymimir MODULE lib/pymimir.cpp ${MIMIR_SRC_FILES})
    set_property(TARGET pymimir PROPERTY CXX_STANDARD 17)
    set_target_properties(pymimir PROPERTIES PREFIX "")
    target_link_libraries(pymimir ${PYTHON_LIBRARIES})

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(pymimir PRIVATE NDEBUG)
    endif()

    if(MSVC)
    # Add MSVC specific library linking here
    else()
        target_link_libraries(pymimir -lstdc++fs)

        # These settings seem to cause issues with torch.
        # target_link_libraries(pymimir -static-libstdc++ -static-libgcc)
    endif()

    # Add pybind11 include directory
    # TODO (Dominik): Use find_package instead
    set(PYBIND11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/pybind11/include)
    target_include_directories(pymimir PRIVATE ${PYBIND11_INCLUDE_DIR})

    # Generate pyi files, add generate_pyi_files dependent on pymimir
    add_custom_target(generate_pyi_files ALL
        COMMENT "Generating .pyi files"
        COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen --output-dir ${CMAKE_CURRENT_BINARY_DIR} pymimir
    )

    add_dependencies(generate_pyi_files pymimir)
endif()

# ----------------
# Target Profiling
# ----------------
if(BUILD_PROFILING)
    find_package(benchmark REQUIRED)
    add_executable(mimir-profile bin/profiling.cpp ${MIMIR_SRC_FILES})
    set_property(TARGET mimir-profile PROPERTY CXX_STANDARD 17)
endif()

# -----------
# Target Test
# -----------
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    add_executable(mimir-test ${MIMIR_TEST_FILES} ${MIMIR_SRC_FILES})
    target_link_libraries(mimir-test ${GTEST_BOTH_LIBRARIES} GTest::GTest GTest::Main)
    gtest_discover_tests(mimir-test)
endif()
