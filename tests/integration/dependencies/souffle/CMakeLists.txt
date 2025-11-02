cmake_minimum_required(VERSION 3.21)
project(InstallSouffle)

include(ExternalProject)

message(STATUS "Dependency \"souffle\"...")

# Attempt to find the installed souffle library
find_package(souffle QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT souffle_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DSOUFFLE_USE_SQLITE=OFF
    )

    message(STATUS "Preparing external project \"souffle\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        souffle
        GIT_REPOSITORY https://github.com/souffle-lang/souffle.git
        GIT_TAG 2.5
        PREFIX ${CMAKE_BINARY_DIR}/souffle
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "souffle is already installed.")
endif()