cmake_minimum_required(VERSION 3.21)
project(InstallGoogletest)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/loki-dependencies"
    )

message(STATUS "Preparing external project \"loki_googletest\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki_googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    PREFIX ${CMAKE_BINARY_DIR}/mimir-dependencies/loki-dependencies/googletest
    CMAKE_ARGS "${CMAKE_ARGS}"
)
