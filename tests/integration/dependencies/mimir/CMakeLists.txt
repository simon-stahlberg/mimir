cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)


##############
# Googletest #
##############

add_subdirectory(dependencies/googletest)


#############
# Benchmark #
#############

add_subdirectory(dependencies/benchmark)


#########
# Boost #
#########

add_subdirectory(dependencies/boost)


########
# Loki #
########

add_subdirectory(dependencies/loki)


############
# Pybind11 #
############

add_subdirectory(dependencies/pybind11)


########
# Loki #
########

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/mimir-dependencies"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"loki\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    mimir
    GIT_REPOSITORY https://github.com/simon-stahlberg/mimir.git
    GIT_TAG loki
    PREFIX ${CMAKE_BINARY_DIR}/mimir
    DEPENDS mimir_googletest mimir_benchmark mimir_boost mimir_pybind11 mimir_loki
    CMAKE_ARGS "${CMAKE_ARGS}"
)
