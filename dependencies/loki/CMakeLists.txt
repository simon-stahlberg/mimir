cmake_minimum_required(VERSION 3.21)
project(InstallLoki)

include(ExternalProject)

message(STATUS "Dependency \"loki\"...")

# Attempt to find the installed loki library
find_package(loki QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT loki_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )

    message(STATUS "Preparing external project \"loki\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
            loki
            GIT_REPOSITORY https://github.com/drexlerd/Loki.git
            GIT_TAG d08dc41b67765201e1742618d0eb981f5e95601e
            PREFIX ${CMAKE_BINARY_DIR}/loki
            DEPENDS boost abseil
            CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "loki is already installed.")
endif()