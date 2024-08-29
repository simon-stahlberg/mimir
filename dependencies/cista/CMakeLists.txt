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
    # Build the cista-test-single-header target to generate the single header file
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target cista-test-single-header
    INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include
                   COMMAND ${CMAKE_COMMAND} -E copy
                   <BINARY_DIR>/cista.h
                   ${CMAKE_INSTALL_PREFIX}/include/cista.h
    UPDATE_COMMAND ""      # Skip the update step
)
