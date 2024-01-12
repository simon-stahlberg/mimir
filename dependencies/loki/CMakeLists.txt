cmake_minimum_required(VERSION 3.21)
project(InstallLoki)

include(ExternalProject)

set(LOKI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/loki")


#####################
# Loki-dependencies #
#####################
list(APPEND LOKI_DEPENDENCIES_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${LOKI_INSTALL_PREFIX}"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"loki_dependencies\" with args:")
foreach(CMAKE_ARG ${LOKI_DEPENDENCIES_CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki_dependencies
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/loki/dependencies
    SOURCE_SUBDIR dependencies
    CMAKE_ARGS "${LOKI_DEPENDENCIES_CMAKE_ARGS}"
    INSTALL_COMMAND ""
)


########
# Loki #
########

list(APPEND LOKI_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${LOKI_INSTALL_PREFIX}"
    "-DCMAKE_PREFIX_PATH=${LOKI_INSTALL_PREFIX}"
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
    DEPENDS loki_dependencies
    CMAKE_ARGS "${LOKI_CMAKE_ARGS}"
)
