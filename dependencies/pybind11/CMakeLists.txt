cmake_minimum_required(VERSION 3.21)
project(InstallPybind11)

include(ExternalProject)

message(STATUS "Dependency \"pybind11\"...")

# Attempt to find the installed library
find_package(pybind11 QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT pybind11_FOUND)
    list(APPEND CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    )

    message(STATUS "Preparing external project \"pybind11\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()


    # If the library is not found, use ExternalProject_Add to download and build it
    ExternalProject_Add(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v2.11.1
            PREFIX ${CMAKE_BINARY_DIR}/pybind11
            CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "pybind11 is already installed.")
endif()


