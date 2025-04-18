find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)
  
find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

# Set result variables
find_package(nanobind)

nanobind_add_module(pymimir
    advanced/main.cpp 
    advanced/common/bindings.cpp 
    advanced/formalism/bindings.cpp 
    advanced/graphs/bindings.cpp 
    advanced/search/bindings.cpp 
    advanced/datasets/bindings.cpp
    advanced/languages/description_logics/bindings.cpp
)

target_link_libraries(pymimir PRIVATE mimir::core)

install(TARGETS pymimir DESTINATION "pymimir")

## Create and install stub for root module.
nanobind_add_stub(
    pymimir_stub
    MODULE pymimir
    OUTPUT __init__.pyi
    MARKER_FILE py.typed
    PYTHON_PATH "."
    DEPENDS pymimir
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.pyi" DESTINATION "pymimir")

## Create and install stub for submodules.
set(MODULES
    pymimir.advanced
    pymimir.advanced.common 
    pymimir.advanced.graphs
    pymimir.advanced.formalism
    pymimir.advanced.search
    pymimir.advanced.datasets
    pymimir.advanced.languages
    pymimir.advanced.languages.description_logics
)

foreach(mod IN LISTS MODULES)
    # Convert module name to valid target name
    string(REPLACE "." "_" mod_id "${mod}")

    # Target name must be globally unique in CMake
    set(stub_target "${mod_id}")

    # Strip "pymimir." prefix from module name for output path
    set(trimmed_mod "${mod}")
    string(REGEX REPLACE "^pymimir\\." "" trimmed_mod "${trimmed_mod}")
    string(REPLACE "." "/" mod_path "${trimmed_mod}")

    set(stub_output "${mod_path}/__init__.pyi")

    # message(STATUS "${stub_target} -> ${stub_output}")

    nanobind_add_stub(
        ${stub_target}
        MODULE ${mod}
        OUTPUT ${stub_output}
        PYTHON_PATH "."
        DEPENDS pymimir
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${stub_output}"
            DESTINATION "pymimir/${mod_path}")
endforeach()

