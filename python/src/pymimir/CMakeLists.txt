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

# Extend rpath to allow moving dependencies around.
if(APPLE)
    set(ORIGIN_RPATH "@loader_path")
elseif(UNIX AND NOT APPLE)
    set(ORIGIN_RPATH "$ORIGIN")
endif()

if (ORIGIN_RPATH)
  get_target_property(existing_rpath _pymimir INSTALL_RPATH)
  # This still requires that all .so files live in ${ORIGIN_RPATH} or ${ORIGIN_RPATH}/dependencies/installs/lib
  # We use this explicitly when moving the python installation around.
  set(extra_rpath "${ORIGIN_RPATH}" "${ORIGIN_RPATH}/dependencies/installs/lib")

  if(NOT existing_rpath)
    set(new_rpath "${extra_rpath}")
  else()
      list(APPEND existing_rpath ${extra_rpath})
      list(REMOVE_DUPLICATES existing_rpath)  # Optional but safe
      set(new_rpath "${existing_rpath}")
  endif()

  set_target_properties(_pymimir PROPERTIES
    INSTALL_RPATH "${new_rpath}"
    BUILD_RPATH "${new_rpath}"  # Optional but good for dev
  )
endif()

nanobind_add_stub(
    _pymimir_stub
    MODULE _pymimir
    OUTPUT __init__.pyi
    PYTHON_PATH "."
    DEPENDS _pymimir
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.pyi" DESTINATION "pymimir")

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

    message(STATUS "${stub_target} -> ${stub_output}")

    nanobind_add_stub(
        ${stub_target}
        MODULE ${mod}
        OUTPUT ${stub_output}
        PYTHON_PATH "."
        DEPENDS _pymimir
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${stub_output}"
            DESTINATION pymimir/${mod_path})
endforeach()

