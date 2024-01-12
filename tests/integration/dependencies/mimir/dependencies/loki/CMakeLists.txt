cmake_minimum_required(VERSION 3.21)
project(InstallMimirLoki)

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

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/mimir-dependencies/loki-dependencies"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"loki\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    mimir_loki
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/mimir-dependencies/loki
    DEPENDS loki_googletest loki_benchmark loki_boost
    CMAKE_ARGS "${CMAKE_ARGS}"
)
