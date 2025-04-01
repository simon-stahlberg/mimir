cmake_minimum_required(VERSION 3.21)
project(InstallNauty)

include(ExternalProject)

message(STATUS "Dependency \"nauty\"...")

find_package(Nauty QUIET PATHS ${CMAKE_INSTALL_PREFIX} ${CMAKE_INSTALL_PREFIX}/include ${CMAKE_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)

if (NOT Nauty_FOUND)

    message(STATUS "Preparing external project \"nauty\".")

    # We need to build nauty for cmake support
    ExternalProject_Add(
            nauty
            GIT_REPOSITORY https://github.com/drexlerd/nauty.git
            GIT_TAG cb44c50bd4fa543b977df6a4b70ec88e67020646
            PREFIX ${CMAKE_BINARY_DIR}/nauty
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ./configure --prefix=<INSTALL_DIR> CFLAGS=-fPIC --enable-tls --disable-shared
            BUILD_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> make
            INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> make install
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    )
else ()
    message(STATUS "nauty is already installed.")
endif ()

