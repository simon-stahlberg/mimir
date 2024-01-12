cmake_minimum_required(VERSION 3.21)
project(InstallBoost)

include(ExternalProject)

message(STATUS "Preparing external project \"boost\":")

ExternalProject_Add(
    mimir_boost
    URL https://archives.boost.io/release/1.84.0/source/boost_1_84_0.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}/boost
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost ${CMAKE_INSTALL_PREFIX}/boost/include/boost
            COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/tools/cmake ${CMAKE_INSTALL_PREFIX}/boost/lib/cmake
    UPDATE_DISCONNECTED 1
)
