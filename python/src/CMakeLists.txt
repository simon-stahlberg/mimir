find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)
  
find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

# Set result variables
find_package(nanobind)

nanobind_add_module(_pymimir NOSTRIP
    pymimir/advanced/main.cpp 
    pymimir/advanced/common/bindings.cpp 
    pymimir/advanced/formalism/bindings.cpp 
    pymimir/advanced/graphs/bindings.cpp 
    pymimir/advanced/search/bindings.cpp 
    pymimir/advanced/datasets/bindings.cpp
    pymimir/advanced/languages/description_logics/bindings.cpp
)

target_link_libraries(_pymimir PRIVATE mimir::core)
