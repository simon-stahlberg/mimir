cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)

### Configure and build lokis dependencies

message(STATUS "Preparing external project \"Loki's dependencies\"")

ExternalProject_Add(
    loki-dependencies
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX dependencies
    SOURCE_SUBDIR dependencies
    BINARY_DIR dependencies/build
    INSTALL_COMMAND ""
)

### Configure, build, and install loki

message(STATUS "Preparing external project \"Loki\"")

set(BOOST_INSTALL_DIR      "${CMAKE_SOURCE_DIR}/build/loki/dependencies/build/boost/boost-prefix/src/boost")
set(GOOGLETEST_INSTALL_DIR "${CMAKE_SOURCE_DIR}/build/loki/dependencies/build/googletest-install")
set(BENCHMARK_INSTALL_DIR  "${CMAKE_SOURCE_DIR}/build/loki/dependencies/build/benchmark-install")

list(APPEND CMAKE_PREFIX_PATH_LOKI
    "${BOOST_INSTALL_DIR}"
    "${GOOGLETEST_INSTALL_DIR}"
    "{BENCHMARK_INSTALL_DIR}")

# Create a list with an alternate separator e.g. pipe symbol
# https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
# https://cmake.org/cmake/help/latest/module/ExternalProject.html#miscellaneous-options
string(REPLACE ";" "|" CMAKE_PREFIX_PATH_LOKI_ALT_SEP "${CMAKE_PREFIX_PATH_LOKI}")

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/loki-install"
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_LOKI_ALT_SEP}"
    "-DCMAKE_BUILD_TYPE=Release"
)

ExternalProject_Add(
    loki
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX loki
    DEPENDS loki-dependencies
    LIST_SEPARATOR |  # declare the list separator that we used above
    CMAKE_ARGS "${CMAKE_ARGS}"
)
