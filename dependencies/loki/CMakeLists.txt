cmake_minimum_required(VERSION 3.21)
project(InstallLoki)

include(ExternalProject)

if(NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
)

message(STATUS "Preparing external project \"loki\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG v0.0.1
    PREFIX ${CMAKE_BINARY_DIR}/loki
    DEPENDS boost
    CMAKE_ARGS ${CMAKE_ARGS}
)
