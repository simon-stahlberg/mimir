cmake_minimum_required(VERSION 3.21)
project(InstallAbseil)

include(ExternalProject)

message(STATUS "Dependency \"abseil\"...")

# Attempt to find the installed abseil library
find_package(abseil QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT abseil_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )

    message(STATUS "Preparing external project \"abseil\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 5913446e0c0f8c1976627654956c3553e1519d2c
        PREFIX ${CMAKE_BINARY_DIR}/abseil
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "abseil is already installed.")
endif()