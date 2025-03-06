find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(nanobind)

nanobind_add_module(_pymimir 
    pymimir/main.cpp 
    pymimir/common/bindings.cpp 
    pymimir/formalism/bindings.cpp 
    pymimir/graphs/bindings.cpp 
    pymimir/search/bindings.cpp 
    pymimir/datasets/bindings.cpp
)

target_link_libraries(_pymimir PRIVATE mimir::core)
