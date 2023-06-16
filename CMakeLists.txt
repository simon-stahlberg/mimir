cmake_minimum_required(VERSION 3.18)
project(mimir)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_package(Boost REQUIRED)
find_package(Python REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

# Set the output directory of the stub files.
set(STUBS_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}")

if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(STUBS_OUTPUT "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif()

# pip install pybind11-stubgen
add_custom_target(generate_pyi_files ALL
    COMMENT "Generating .pyi files to ${STUBS_OUTPUT}"
    COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen --output-dir ${STUBS_OUTPUT} mimir
)

# Make generate_pyi_files target dependent on mimir target
add_dependencies(generate_pyi_files mimir)

add_subdirectory(mimir)
add_library(mimir MODULE mimir/mimir.cpp)

# install(TARGETS mimir LIBRARY DESTINATION mimir)
# install(FILES ${CMAKE_BINARY_DIR}/mimir-stubs/__init__.pyi DESTINATION mimir)
# install(FILES ${CMAKE_BINARY_DIR}/mimir-stubs/setup.py DESTINATION mimir)
set_property(TARGET mimir PROPERTY CXX_STANDARD 17)
target_link_libraries(mimir -static-libstdc++ -lstdc++fs -static-libgcc)
target_link_libraries(mimir formalism)
target_link_libraries(mimir generators)
target_link_libraries(mimir pddl)

# Add pybind11 include directory
set(PYBIND11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/pybind11/include)
target_include_directories(mimir PRIVATE ${PYBIND11_INCLUDE_DIR})

set_target_properties(mimir PROPERTIES PREFIX "")
target_link_libraries(mimir ${PYTHON_LIBRARIES})
