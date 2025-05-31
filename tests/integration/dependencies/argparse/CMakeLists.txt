cmake_minimum_required(VERSION 3.21)
project(InstallArgparse)

include(ExternalProject)

message(STATUS "Dependency \"argparse\"...")

# Attempt to find the installed loki library
find_package(argparse QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT argparse_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )

    message(STATUS "Preparing external project \"argparse\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        argparse
            GIT_REPOSITORY https://github.com/p-ranav/argparse.git
            GIT_TAG v3.2
            PREFIX ${CMAKE_BINARY_DIR}/argparse
            CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "argparse is already installed.")
endif()