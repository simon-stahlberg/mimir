cmake_minimum_required(VERSION 3.21)
project(InstallGTL)

include(ExternalProject)

message(STATUS "Dependency \"gtl\"...")

# Attempt to find the installed gtl library
find_package(gtl QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT gtl_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            # -DCMAKE_CXX_FLAGS="-fsanitize=address"
    )

    message(STATUS "Preparing external project \"gtl\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        gtl
        GIT_REPOSITORY https://github.com/greg7mdp/gtl.git
        GIT_TAG dc7a24fe4b7f2f94480d954da6562c7381ace41f
        PREFIX ${CMAKE_BINARY_DIR}/gtl
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "gtl is already installed.")
endif()