cmake_minimum_required(VERSION 3.21)


##############################################################
# Language setup
##############################################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


##############################################################
# Establish project
##############################################################

project(integration_example VERSION "0.0.1" LANGUAGES C CXX)


##############################################################
# CMake modules and macro files
##############################################################

find_package(mimir COMPONENTS core REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)


##############################################################
# Add library and executable targets
##############################################################

add_executable(integration_standalone "main.cpp")
target_link_libraries(integration_standalone mimir::core)
