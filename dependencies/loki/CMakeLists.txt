cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)

message(STATUS "Preparing external project \"Loki's dependencies\"")
ExternalProject_Add(
    loki-dependencies
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX dependencies  # Root
    SOURCE_SUBDIR dependencies  # Source for configure
    BINARY_DIR dependencies/build
    INSTALL_COMMAND ""
)
message("${CMAKE_SOURCE_DIR}")

set(CMAKE_PREFIX_PATH 
   "${CMAKE_PREFIX_PATH}"
   "${CMAKE_SOURCE_DIR}/build/loki/dependencies/build/boost/boost-prefix/src/boost"
   "${CMAKE_SOURCE_DIR}/build/loki/dependencies/build/benchmark-install"
   "${CMAKE_SOURCE_DIR}/build/loki/dependencies/build/googletest-install")


# Create a list with an alternate separator e.g. pipe symbol
string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH}")
   

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/loki-install"
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}"
)

message("${DEPENDENCIES_INSTALL_PATH}")

message(STATUS "Preparing external project \"loki\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

message("${CMAKE_BINARY_DIR}/loki")
ExternalProject_Add(
    loki
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/loki
    DEPENDS loki-dependencies
    CMAKE_ARGS "${CMAKE_ARGS}"
)
