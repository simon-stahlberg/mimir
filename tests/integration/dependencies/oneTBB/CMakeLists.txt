cmake_minimum_required(VERSION 3.21)
project(InstallOneTBB)

include(ExternalProject)

message(STATUS "Dependency \"oneTBB\"...")

# Attempt to find the installed oneTBB library
find_package(oneTBB QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT oneTBB_FOUND)
    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DBUILD_SHARED_LIBS=OFF
            -DTBB_STRICT=OFF
            -DTBB_TEST=OFF
    )

    message(STATUS "Preparing external project \"oneTBB\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        oneTBB
        GIT_REPOSITORY https://github.com/uxlfoundation/oneTBB.git
        GIT_TAG v2022.2.0
        PREFIX ${CMAKE_BINARY_DIR}/oneTBB
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "oneTBB is already installed.")
endif()