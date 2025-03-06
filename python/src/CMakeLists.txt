find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(nanobind)

nanobind_add_module(_pymimir 
    pymimir/main.cpp 
    pymimir/formalism/bindings.cpp 
    pymimir/graphs/bindings.cpp 
    pymimir/search/bindings.cpp 
    pymimir/datasets/bindings.cpp
)

nanobind_add_stub(
  pymimir_stub
  INSTALL_TIME
  MODULE _pymimir
  OUTPUT _pymimir.pyi
  PYTHON_PATH $<TARGET_FILE_DIR:_pymimir>
  DEPENDS _pymimir
  MARKER_FILE py.typed
)

# Install the compiled extension
install(TARGETS _pymimir DESTINATION ".")

# Install stub and marker
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/_pymimir.pyi
    ${CMAKE_CURRENT_BINARY_DIR}/py.typed
    DESTINATION "."
)

# Install the Python package sources
install(DIRECTORY pymimir/ DESTINATION "pymimir")