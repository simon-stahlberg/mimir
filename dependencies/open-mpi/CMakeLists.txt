cmake_minimum_required(VERSION 3.21)
project(InstallClingo)

include(ExternalProject)

message(STATUS "Dependency \"openmpi\"...")

# Attempt to find the installed openmpi library
find_package(MPI QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT MPI_FOUND)
    list(APPEND CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    )

    message(STATUS "Preparing external project \"openmpi\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        openmpi
        GIT_REPOSITORY https://github.com/open-mpi/ompi.git
        GIT_TAG v5.0.7
        PREFIX ${CMAKE_BINARY_DIR}/openmpi
        GIT_SUBMODULES_RECURSE ON
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/openmpi/src/openmpi
            ${CMAKE_BINARY_DIR}/openmpi/src/openmpi/autogen.pl &&
            ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/openmpi/src/openmpi
            ${CMAKE_BINARY_DIR}/openmpi/src/openmpi/configure --prefix=${CMAKE_INSTALL_PREFIX}

        BUILD_COMMAND make -C ${CMAKE_BINARY_DIR}/openmpi/src/openmpi -j$(nproc)
        INSTALL_COMMAND make -C ${CMAKE_BINARY_DIR}/openmpi/src/openmpi install
    )
else()
    message(STATUS "openmpi is already installed.")
endif()