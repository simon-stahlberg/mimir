cmake_minimum_required(VERSION 3.0)
project(mimir)

find_package(PythonLibs REQUIRED)
find_package(Python REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

# pip install pybind11-stubgen
add_custom_target(generate_pyi_files ALL
    COMMENT "Generating .pyi files"
    COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen --output-dir ${CMAKE_CURRENT_BINARY_DIR} mimir
)

# Make generate_pyi_files target dependent on mimir target
add_dependencies(generate_pyi_files mimir)

add_library(mimir MODULE mimir.cpp)

# Add pybind11 include directory
set(PYBIND11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/pybind11/include)
target_include_directories(mimir PRIVATE ${PYBIND11_INCLUDE_DIR})

set_target_properties(mimir PROPERTIES PREFIX "")
target_link_libraries(mimir ${PYTHON_LIBRARIES})
