find_package(pybind11 CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(pybind11)

add_library(_pymimir_objects
        OBJECT
        pymimir/bindings.cpp
)
target_include_directories(_pymimir_objects
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/pymimir
)

target_link_libraries(_pymimir_objects
        PUBLIC
        mimir::core
        pybind11::module
)

pybind11_add_module(_pymimir MODULE)
target_sources(_pymimir
        PRIVATE
        $<TARGET_OBJECTS:_pymimir_objects>
        pymimir/main.cpp
)
target_link_libraries(_pymimir
        PRIVATE
        _pymimir_objects
)
target_compile_definitions(_pymimir PUBLIC MIMIR_VERSION_INFO="${MIMIR_VERSION_INFO}")

target_compile_definitions(_pymimir PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
