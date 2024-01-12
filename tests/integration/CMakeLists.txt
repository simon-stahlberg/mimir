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
# CMake modules and macro files
##############################################################

list(APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake"
)
include("configure_dependencies")


##############################################################
# Dependency Handling
##############################################################

# set(CMAKE_FIND_DEBUG_MODE ON)

set(DEPENDENCY_NAMES_LIST loki mimir)
configure_dependencies("${DEPENDENCY_NAMES_LIST}")


##############################################################
# CMake modules and macro files
##############################################################

find_package(loki 0.0.1 COMPONENTS parsers REQUIRED)
find_package(mimir COMPONENTS core REQUIRED)

##############################################################
# Add library and executable targets
##############################################################


add_executable(integration_standalone "main.cpp")
target_link_libraries(integration_standalone mimir loki::parsers)

add_custom_target(example_domain ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/gripper/domain.pddl" "${CMAKE_BINARY_DIR}/gripper/domain.pddl")
add_custom_target(example_problem_0 ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/gripper/p-2-0.pddl" "${CMAKE_BINARY_DIR}/gripper/p-2-0.pddl")


