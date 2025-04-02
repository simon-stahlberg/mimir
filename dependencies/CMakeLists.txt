cmake_minimum_required(VERSION 3.21)

project(dependencies)

##############################################################
# Common Settings
##############################################################

# Option to set the build type (default to Release)
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

# Pass the build type to subdirectories
set(BUILD_TYPE ${CMAKE_BUILD_TYPE})

if(COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
endif ()


##############################################################
# Add subprojects
##############################################################

# add_subdirectory(open-mpi)  # takes a long time to install. perhaps we want users to install it separately
add_subdirectory(abseil)
add_subdirectory(benchmark)
add_subdirectory(boost)
add_subdirectory(fmt)
add_subdirectory(googletest)
add_subdirectory(loki)
add_subdirectory(nanobind)
add_subdirectory(nauty)
