find_package(Python 3.8 REQUIRED
  COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH})
  # Set result variables
find_package(nanobind)
if(nanobind_FOUND)
   message(STATUS "Found nanobind: ${NB_DIR} (found version ${nanobind_VERSION})")
endif()


nanobind_add_module(pymimir
    NB_DOMAIN pymimir_abi_domain
    advanced/main.cpp
    advanced/common/bindings.cpp
    advanced/formalism/bindings.cpp
    advanced/graphs/bindings.cpp
    advanced/search/bindings.cpp
    advanced/datasets/bindings.cpp
    advanced/languages/description_logics/bindings.cpp
    advanced/languages/general_policies/bindings.cpp
)

target_link_libraries(pymimir PRIVATE mimir::core)

if(APPLE)
    set(_pymimir_rpath "@loader_path/lib")
elseif(UNIX)
    set(_pymimir_rpath "$ORIGIN/lib")
endif()
if(_pymimir_rpath)
    set_target_properties(pymimir PROPERTIES
        BUILD_RPATH "${_pymimir_rpath}"
        INSTALL_RPATH "${_pymimir_rpath}"
    )
endif()

install(TARGETS pymimir DESTINATION ".")

## Stubgen
set(MODULES
    # pymimir  # Handled by __init__.py
    pymimir.advanced
    pymimir.advanced.common
    pymimir.advanced.graphs
    pymimir.advanced.formalism
    pymimir.advanced.search
    pymimir.advanced.datasets
    pymimir.advanced.languages
    pymimir.advanced.languages.description_logics
    pymimir.advanced.languages.general_policies
)

foreach(mod IN LISTS MODULES)
    string(REPLACE "." "_" mod_target "${mod}")
    set(mod_target "${mod_target}_stub")
    string(REPLACE "." "/" mod_path "${mod}")
    string(REGEX REPLACE "^pymimir/" "" mod_path "${mod_path}")
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
