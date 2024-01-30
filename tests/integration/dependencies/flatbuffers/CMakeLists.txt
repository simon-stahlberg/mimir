cmake_minimum_required(VERSION 3.21)
project(InstallGoogletest)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

message(STATUS "Preparing external project \"flatbuffers\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v23.5.26
    PREFIX ${CMAKE_BINARY_DIR}/flatbuffers
    CMAKE_ARGS ${CMAKE_ARGS}
)
