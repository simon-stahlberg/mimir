cmake_minimum_required(VERSION 3.13)
project(InstallBenchmark)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/benchmark-install"
    "-DCMAKE_BUILD_TYPE=Release"
    "-DBENCHMARK_ENABLE_GTEST_TESTS=OFF"
    )

message(${CMAKE_BINARY_DIR}/benchmark-install)

message(STATUS "Preparing external project \"benchmark\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    PREFIX ${CMAKE_BINARY_DIR}/benchmark
    CMAKE_ARGS "${CMAKE_ARGS}"
)

