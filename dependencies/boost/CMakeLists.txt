cmake_minimum_required(VERSION 3.13)
project(DownloadBoost)

include(ExternalProject)

message(STATUS "Preparing external project \"boost\":")

ExternalProject_Add(
    boost
    # URL https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz  ## the official link is broken
    URL https://archives.boost.io/release/1.84.0/source/boost_1_84_0.tar.gz
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_DISCONNECTED true
)

ExternalProject_Get_Property(boost source_dir)
set(BOOST_INCLUDE_DIRS ${source_dir})
