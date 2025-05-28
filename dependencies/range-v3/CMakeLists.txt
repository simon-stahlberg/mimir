cmake_minimum_required(VERSION 3.21)
project(InstallRangeV3)

include(ExternalProject)

message(STATUS "Dependency \"range-v3\"...")

# Attempt to find the installed range-v3 library
find_package(range-v3 QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT range-v3_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DRANGE_V3_TESTS=OFF
    )

    message(STATUS "Preparing external project \"range-v3\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        range-v3
            GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
            GIT_TAG 0.12.0
            PREFIX ${CMAKE_BINARY_DIR}/range-v3
            CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "range-v3 is already installed.")
endif()