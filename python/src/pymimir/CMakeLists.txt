find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)
  
find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

# Set result variables
find_package(nanobind)

nanobind_add_module(_pymimir
    advanced/main.cpp 
    advanced/common/bindings.cpp 
    advanced/formalism/bindings.cpp 
    advanced/graphs/bindings.cpp 
    advanced/search/bindings.cpp 
    advanced/datasets/bindings.cpp
    advanced/languages/description_logics/bindings.cpp
)

target_link_libraries(_pymimir PRIVATE mimir::core)

install(TARGETS _pymimir DESTINATION "pymimir")

## Create and install stub for root module.
nanobind_add_stub(
    _pymimir_stub
    MODULE _pymimir
    OUTPUT __init__.pyi
    PYTHON_PATH "."
    DEPENDS _pymimir
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.pyi" DESTINATION "pymimir")

## Create and install stub for submodules.
set(MODULES
    _pymimir.advanced
    _pymimir.advanced.common 
    _pymimir.advanced.graphs
    _pymimir.advanced.formalism
    _pymimir.advanced.search
    _pymimir.advanced.languages
    _pymimir.advanced.languages.description_logics
)

foreach(mod IN LISTS MODULES)
    # Convert module name to valid target name
    string(REPLACE "." "_" mod_id "${mod}")

    # Target name must be globally unique in CMake
    set(stub_target "${mod_id}_stub")

    # Strip "_pymimir." prefix from module name for output path
    set(trimmed_mod "${mod}")
    string(REGEX REPLACE "^_pymimir\\." "" trimmed_mod "${trimmed_mod}")
    string(REPLACE "." "/" mod_path "${trimmed_mod}")

    set(stub_output "${mod_path}/__init__.pyi")

    # message(STATUS "${stub_target} -> ${stub_output}")

    nanobind_add_stub(
        ${stub_target}
        MODULE ${mod}
        OUTPUT ${stub_output}
        PYTHON_PATH "."
        DEPENDS _pymimir
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${stub_output}"
            DESTINATION "pymimir/${mod_path}")
endforeach()

