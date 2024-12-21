cmake_minimum_required(VERSION 3.21)
project(InstallFMT)

include(ExternalProject)


message(STATUS "Dependency \"fmt\"...")

# Attempt to find the installed fmt library
find_package(fmt QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT fmt_FOUND)
    list(APPEND CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )
    message(STATUS "Preparing external project \"fmt\" with args:")
    foreach (CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach ()

    ExternalProject_Add(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt
            GIT_TAG 11.0.2
            PREFIX ${CMAKE_BINARY_DIR}/fmt
            CMAKE_ARGS ${CMAKE_ARGS}
    )
else ()
    message(STATUS "fmt is already installed.")
endif ()

