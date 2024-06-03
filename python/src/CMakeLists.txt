find_package(pybind11 CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(pybind11)

pybind11_add_module(_pymimir MODULE)
target_sources(_pymimir
    PRIVATE
        pymimir/main.cpp
        pymimir/bindings.cpp)
target_link_libraries(_pymimir
    PRIVATE
        pybind11::module
        mimir::core)
target_compile_definitions(_pymimir PUBLIC MIMIR_VERSION_INFO="${MIMIR_VERSION_INFO}")
