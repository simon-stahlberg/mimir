cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)


##############
# Googletest #
##############

list(APPEND LOKI_GOOGLETEST_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/loki-dependencies"
    )

message(STATUS "Preparing external project \"loki_googletest\" with args:")
foreach(CMAKE_ARG ${LOKI_GOOGLETEST_CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki_googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    PREFIX ${CMAKE_BINARY_DIR}/loki-dependencies/googletest
    CMAKE_ARGS "${LOKI_GOOGLETEST_CMAKE_ARGS}"
)


#############
# Benchmark #
#############

list(APPEND LOKI_BENCHMARK_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/loki-dependencies"
    "-DCMAKE_BUILD_TYPE=Release"
    "-DBENCHMARK_ENABLE_GTEST_TESTS=OFF"
    )

message(${CMAKE_BINARY_DIR}/benchmark-install)

message(STATUS "Preparing external project \"loki_benchmark\" with args:")
foreach(CMAKE_ARG ${LOKI_BENCHMARK_CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki_benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    PREFIX ${CMAKE_BINARY_DIR}/loki-dependencies/benchmark
    CMAKE_ARGS "${LOKI_BENCHMARK_CMAKE_ARGS}"
)


#########
# Boost #
#########

message(STATUS "Preparing external project \"loki_boost\":")

ExternalProject_Add(
    loki_boost
    URL https://archives.boost.io/release/1.84.0/source/boost_1_84_0.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}/loki-dependencies/boost
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost ${CMAKE_INSTALL_PREFIX}/loki-dependencies/include/boost
            COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/tools/cmake ${CMAKE_INSTALL_PREFIX}/loki-dependencies/lib/cmake
    UPDATE_DISCONNECTED 1
)

set(BOOST_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX})


########
# Loki #
########

list(APPEND LOKI_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/loki-dependencies"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"loki\" with args:")
foreach(CMAKE_ARG ${LOKI_CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/loki
    DEPENDS loki_googletest loki_benchmark loki_boost
    CMAKE_ARGS "${LOKI_CMAKE_ARGS}"
)
