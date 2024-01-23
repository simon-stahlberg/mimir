cmake_minimum_required(VERSION 3.21)
project(InstallBoost)

include(ExternalProject)

message(STATUS "Preparing external project \"boost\":")

# We need to build boost for cmake support
ExternalProject_Add(
    boost
    URL https://archives.boost.io/release/1.84.0/source/boost_1_84_0.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}/boost
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ./bootstrap.sh --prefix=<INSTALL_DIR>
    BUILD_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ./b2 headers
    INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ./b2 install
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)
