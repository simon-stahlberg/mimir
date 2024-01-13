cmake_minimum_required(VERSION 3.21)
project(InstallGoogletest)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    )

message(STATUS "Preparing external project \"pybind11\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
    PREFIX ${CMAKE_BINARY_DIR}/pybind11
    CMAKE_ARGS ${CMAKE_ARGS}
)

