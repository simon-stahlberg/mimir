cmake_minimum_required(VERSION 3.21)
project(InstallFlatmemory)

include(ExternalProject)

if(NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

message(STATUS "Preparing external project \"flatmemory\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    flatmemory
    GIT_REPOSITORY https://github.com/drexlerd/flatmemory.git
    GIT_TAG v0.0.1
    PREFIX ${CMAKE_BINARY_DIR}/flatmemory
    CMAKE_ARGS ${CMAKE_ARGS}
)
