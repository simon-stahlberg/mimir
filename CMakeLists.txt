cmake_minimum_required(VERSION 3.0)
project(mimir)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

add_library(mimir MODULE mimir.cpp)

# Add pybind11 include directory
set(PYBIND11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/pybind11/include)
target_include_directories(mimir PRIVATE ${PYBIND11_INCLUDE_DIR})

set_target_properties(mimir PROPERTIES PREFIX "")
target_link_libraries(mimir ${PYTHON_LIBRARIES})
