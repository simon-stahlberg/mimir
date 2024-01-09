cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)


### Configure, build, and install pybind11

message(STATUS "Preparing external project \"googletest\"")

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/googletest-install"
)

ExternalProject_Add(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    PREFIX ${CMAKE_BINARY_DIR}/googletest
    CMAKE_ARGS "${CMAKE_ARGS}"
)
