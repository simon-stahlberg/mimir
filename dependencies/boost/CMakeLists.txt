cmake_minimum_required(VERSION 3.21)
project(InstallBoost)

include(ExternalProject)

message(STATUS "Preparing external project \"boost\":")

set(BOOST_VERSION 1.84.0)
set(BOOST_VERSION_UNDERSCORE 1_84_0)

# Add the external project for Boost
ExternalProject_Add(
    boost
    URL https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_UNDERSCORE}.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}/boost
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "No configure needed"
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "No build needed"
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost ${CMAKE_INSTALL_PREFIX}/include/boost &&
        ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBOOST_VERSION=${BOOST_VERSION} -P ${CMAKE_CURRENT_SOURCE_DIR}/install_boost.cmake
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)
