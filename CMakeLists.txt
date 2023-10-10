cmake_minimum_required(VERSION 3.18)
project(mimir)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

find_package(GTest REQUIRED)
find_package(Boost REQUIRED)
find_package(Python REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
add_subdirectory(mimir)
add_subdirectory(test)

message(STATUS "CMAKE_BUILD_TYPE is \"${CMAKE_BUILD_TYPE}\"")
message(STATUS "Python_EXECUTABLE is \"${Python_EXECUTABLE}\"")

# ------------
# Target Mimir
# ------------
add_library(mimir MODULE mimir/mimir.cpp)
set_property(TARGET mimir PROPERTY CXX_STANDARD 17)
set_target_properties(mimir PROPERTIES PREFIX "")
target_link_libraries(mimir formalism)
target_link_libraries(mimir generators)
target_link_libraries(mimir pddl)
target_link_libraries(mimir ${PYTHON_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(mimir PRIVATE NDEBUG)
endif()

if(MSVC)
# Add MSVC specific library linking here
else()
    target_link_libraries(mimir -static-libstdc++ -lstdc++fs -static-libgcc)
endif()

# Add pybind11 include directory
set(PYBIND11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/pybind11/include)
target_include_directories(mimir PRIVATE ${PYBIND11_INCLUDE_DIR})

# Generate pyi files, add generate_pyi_files dependent on mimir
add_custom_target(generate_pyi_files ALL
    COMMENT "Generating .pyi files"
    COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen --output-dir ${CMAKE_CURRENT_BINARY_DIR} mimir
)

add_dependencies(generate_pyi_files mimir)

# ----------------
# Target Profiling
# ----------------
add_executable(profiling mimir/profiling.cpp)
set_property(TARGET profiling PROPERTY CXX_STANDARD 17)
target_link_libraries(profiling formalism)
target_link_libraries(profiling generators)
target_link_libraries(profiling pddl)

# -----------
# Target Test
# -----------
include(GoogleTest)
file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(mimir-test ${TEST_SOURCES})
target_link_libraries(mimir-test pddl)
target_link_libraries(mimir-test formalism)
target_link_libraries(mimir-test algorithms)
target_link_libraries(mimir-test generators)
target_link_libraries(mimir-test ${GTEST_BOTH_LIBRARIES} pthread)
gtest_discover_tests(mimir-test)