cmake_minimum_required(VERSION 3.21)
project(InstallUnorderedDense)

include(ExternalProject)

message(STATUS "Dependency \"unordered_dense\"...")

# Attempt to find the installed unordered_dense library
find_package(unordered_dense QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT unordered_dense_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    )

    message(STATUS "Preparing external project \"unordered_dense\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        unordered_dense
        GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
        GIT_TAG 73f3cbb237e84d483afafc743f1f14ec53e12314
        PREFIX ${CMAKE_BINARY_DIR}/unordered_dense
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "unordered_dense is already installed.")
endif()