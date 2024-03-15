find_package(pybind11 CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(pybind11)

pybind11_add_module(_mimir MODULE)
target_sources(_mimir
    PRIVATE
        src/main.cpp
        src/mimir/formalism/bindings.cpp
        src/mimir/search/bindings.cpp)
target_link_libraries(_mimir
    PRIVATE
        pybind11::module
        mimir::core)
target_link_options(_mimir PRIVATE -static-libstdc++)
target_compile_definitions(_mimir PUBLIC MIMIR_VERSION_INFO="${MIMIR_VERSION_INFO}")
