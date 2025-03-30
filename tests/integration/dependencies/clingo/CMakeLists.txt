cmake_minimum_required(VERSION 3.21)
project(InstallClingo)

include(ExternalProject)

message(STATUS "Dependency \"clingo\"...")

# Attempt to find the installed clingo library
find_package(Clingo QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT Clingo_FOUND)
    list(APPEND CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    )

    message(STATUS "Preparing external project \"clingo\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        clingo
        GIT_REPOSITORY https://github.com/potassco/clingo.git
        GIT_TAG 65c976c0396405823ec65dd9eadb305511ba51eb
        PREFIX ${CMAKE_BINARY_DIR}/clingo
        GIT_SUBMODULES_RECURSE ON
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "clingo is already installed.")
endif()