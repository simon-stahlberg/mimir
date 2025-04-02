cmake_minimum_required(VERSION 3.21)
project(InstallMimir)

include(ExternalProject)


list(APPEND CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_BUILD_TYPE=Release
    )

message(STATUS "Preparing external project \"mimir\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    mimir
    GIT_REPOSITORY https://github.com/simon-stahlberg/mimir.git
    GIT_TAG main
    PREFIX ${CMAKE_BINARY_DIR}/mimir
    DEPENDS loki boost fmt nauty abseil
    CMAKE_ARGS ${CMAKE_ARGS}
)
