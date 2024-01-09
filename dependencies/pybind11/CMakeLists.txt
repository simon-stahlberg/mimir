cmake_minimum_required(VERSION 3.13)
project(InstallPybind11)

include(ExternalProject)


### Configure, build, and install pybind11

message(STATUS "Preparing external project \"pybind11\"")

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/pybind11-install"
)

ExternalProject_Add(
    mimir_googletest
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
    PREFIX ${CMAKE_BINARY_DIR}/pybind11
    CMAKE_ARGS "${CMAKE_ARGS}"
)
