find_package(Python REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(pybind11 2.11.1 REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
include_directories(${PYTHON_INCLUDE_DIRS})
add_library(pymimir MODULE pymimir.cpp ${MIMIR_SRC_FILES})
set_property(TARGET pymimir PROPERTY CXX_STANDARD 17)
set_target_properties(pymimir PROPERTIES PREFIX "")
target_link_libraries(pymimir mimir::core ${PYTHON_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(pymimir PRIVATE NDEBUG)
endif()

if(MSVC)
# Add MSVC specific library linking here
else()
    target_link_libraries(pymimir -lstdc++fs)

    # These settings seem to cause issues with torch.
    # target_link_libraries(pymimir -static-libstdc++ -static-libgcc)
endif()

# Generate pyi files, add generate_pyi_files dependent on pymimir
add_custom_target(generate_pyi_files ALL
    COMMENT "Generating .pyi files"
    COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen --output-dir ${CMAKE_CURRENT_BINARY_DIR} pymimir
)

add_dependencies(generate_pyi_files pymimir)
