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

## Stubgen
set(MODULES
    pymimir
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
    string(REPLACE "." "_" mod_target "${mod}")
    set(mod_target "${mod_target}_stub")
    string(REPLACE "." "/" mod_path "${mod}")
    set(mod_output "${mod_path}/__init__.pyi")

    nanobind_add_stub(
        "${mod_target}"
        MODULE "${mod}"
        OUTPUT "${mod_output}"
        PYTHON_PATH "."
        DEPENDS pymimir
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${mod_output}"
            DESTINATION ${mod_path})
endforeach()

