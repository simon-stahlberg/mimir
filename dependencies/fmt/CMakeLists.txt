cmake_minimum_required(VERSION 3.21)
project(InstallFMT)

include(ExternalProject)

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


