cmake_minimum_required(VERSION 3.21)


##############################################################
# Language setup
##############################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


##############################################################
# Establish project
##############################################################

project(integration_example VERSION "0.0.1" LANGUAGES C CXX)


##############################################################
# Dependency handling
##############################################################

# We installed the dependencies into the subdirectories under the install prefix.
# Hence must append them to the single cmake_prefix_path.
message("Configure dependencies of Loki:")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
list(LENGTH CMAKE_PREFIX_PATH PREFIX_PATH_COUNT)
if(PREFIX_PATH_COUNT GREATER 1)
    message(FATAL_ERROR "Only one prefix path is allowed. Found multiple paths in CMAKE_PREFIX_PATH. Please add dependencies to the CMake Superbuild.")
endif()

list(APPEND MODIFIED_CMAKE_PREFIX_PATH
    "${CMAKE_PREFIX_PATH}/mimir")
message(${MODIFIED_CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH ${MODIFIED_CMAKE_PREFIX_PATH})
message(STATUS "MODIFIED_CMAKE_PREFIX_PATH:")
foreach(CMAKE_PREFIX_PATH_ARG ${CMAKE_PREFIX_PATH})
    message(STATUS "-- ${CMAKE_PREFIX_PATH_ARG}")
endforeach()
# set(CMAKE_FIND_DEBUG_MODE ON)


##############################################################
# CMake modules and macro files
##############################################################

find_package(loki 0.0.1 COMPONENTS parsers REQUIRED)


##############################################################
# Add library and executable targets
##############################################################

add_executable(integration_standalone "main.cpp")
target_link_libraries(integration_standalone loki::parsers)

add_custom_target(example_domain ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/gripper/domain.pddl" "${CMAKE_BINARY_DIR}/gripper/domain.pddl")
add_custom_target(example_problem_0 ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/gripper/p-2-0.pddl" "${CMAKE_BINARY_DIR}/gripper/p-2-0.pddl")


