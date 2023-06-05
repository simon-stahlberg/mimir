cmake_minimum_required(VERSION 3.0)
project(mimir)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

add_library(mimir MODULE mimir.cpp)
set_target_properties(mimir PROPERTIES PREFIX "")
target_link_libraries(mimir ${PYTHON_LIBRARIES})
