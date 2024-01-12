cmake_minimum_required(VERSION 3.21)
project(InstallMimir)

include(ExternalProject)

set(MIMIR_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/loki")


######################
# Mimir-dependencies #
######################
list(APPEND MIMIR_DEPENDENCIES_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${MIMIR_INSTALL_PREFIX}"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"mimir_dependencies\" with args:")
foreach(CMAKE_ARG ${MIMIR_DEPENDENCIES_CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    mimir_dependencies
    GIT_REPOSITORY https://github.com/simon-stahlberg.git
    GIT_TAG loki
    PREFIX ${CMAKE_BINARY_DIR}/mimir/dependencies
    SOURCE_SUBDIR dependencies
    CMAKE_ARGS "${MIMIR_DEPENDENCIES_CMAKE_ARGS}"
    INSTALL_COMMAND ""
)


#########
# Mimir #
#########

list(APPEND MIMIR_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${MIMIR_INSTALL_PREFIX}"
    "-DCMAKE_PREFIX_PATH=${MIMIR_INSTALL_PREFIX}"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"mimir\" with args:")
foreach(CMAKE_ARG ${MIMIR_CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    mimir
    GIT_REPOSITORY https://github.com/simon-stahlberg.git
    GIT_TAG loki
    PREFIX ${CMAKE_BINARY_DIR}/mimir
    DEPENDS mimir_dependencies
    CMAKE_ARGS "${MIMIR_CMAKE_ARGS}"
)
