cmake_minimum_required(VERSION 3.21)
project(InstallCapnproto)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

message(STATUS "Preparing external project \"capnproto\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    capnproto
    GIT_REPOSITORY https://github.com/capnproto/capnproto.git
    GIT_TAG v1.0.2
    PREFIX ${CMAKE_BINARY_DIR}/capnproto
    CMAKE_ARGS ${CMAKE_ARGS}
)
