cmake_minimum_required(VERSION 3.21)
project(InstallCista)

include(ExternalProject)

list(APPEND CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

message(STATUS "Preparing external project \"cista\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    cista
    GIT_REPOSITORY https://github.com/felixguendling/cista.git
    GIT_TAG v0.15
    PREFIX ${CMAKE_BINARY_DIR}/cista
    CMAKE_ARGS ${CMAKE_ARGS}
    CONFIGURE_COMMAND ""   # Skip the configure step
    BUILD_COMMAND ""       # Skip the build step since it's header-only
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_BINARY_DIR}/cista/src/cista/include
                   ${CMAKE_INSTALL_PREFIX}/include
    UPDATE_COMMAND ""      # Skip the update step
)
