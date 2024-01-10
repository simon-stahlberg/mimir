cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}"
    "-DCMAKE_BUILD_TYPE=Release"
    )

message(STATUS "Preparing external project \"loki\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    loki
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/loki
    DEPENDS BoostHeaders googletest benchmark
    CMAKE_ARGS "${CMAKE_ARGS}"
)
