cmake_minimum_required(VERSION 3.13)
project(InstallGoogletest)

include(ExternalProject)

message(STATUS "Preparing external project \"Loki's dependencies\"")
ExternalProject_Add(
    loki-dependencies
    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
    GIT_TAG main
    PREFIX ${CMAKE_SOURCE_DIR}
    SOURCE_SUBDIR dependencies
    BINARY_DIR dependencies/build
    INSTALL_COMMAND ""
)

#list(APPEND CMAKE_ARGS
#    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/loki-install"
#    "-DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/build/boost/boost-prefix/src/boost;${CMAKE_SOURCE_DIR}/build/benchmark-install;${CMAKE_SOURCE_DIR}/build/googletest-install"
#    "-DENABLE_TESTING=ON"
#    "-DENABLE_BENCHMARKING=ON"
#    )
#
#message(STATUS "Preparing external project \"loki\" with args:")
#foreach(CMAKE_ARG ${CMAKE_ARGS})
#    message(STATUS "-- ${CMAKE_ARG}")
#endforeach()
#
#ExternalProject_Add(
#    loki
#    GIT_REPOSITORY https://github.com/drexlerd/Loki.git
#    GIT_TAG main
#    PREFIX ${CMAKE_BINARY_DIR}/loki
#    CMAKE_ARGS "${CMAKE_ARGS}"
#)
