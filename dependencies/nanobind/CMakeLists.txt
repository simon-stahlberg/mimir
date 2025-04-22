cmake_minimum_required(VERSION 3.21)
project(InstallNanobind)

include(ExternalProject)

message(STATUS "Dependency \"nanobind\"...")

# Attempt to find the installed library
find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

find_package(nanobind QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT nanobind_FOUND)
    list(APPEND CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    )

    message(STATUS "Preparing external project \"nanobind\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()


    # If the library is not found, use ExternalProject_Add to download and build it
    ExternalProject_Add(
            nanobind
            GIT_REPOSITORY https://github.com/wjakob/nanobind.git
            GIT_TAG v2.7.0
            PREFIX ${CMAKE_BINARY_DIR}/nanobind
            CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "nanobind is already installed.")
endif()


