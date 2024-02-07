cmake_minimum_required(VERSION 3.21)
project(InstallFlatmemory)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

message(STATUS "Preparing external project \"flatmemory\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    flatmemory
    GIT_REPOSITORY https://github.com/drexlerd/flatmemory.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/flatmemory
    CMAKE_ARGS ${CMAKE_ARGS}
)
