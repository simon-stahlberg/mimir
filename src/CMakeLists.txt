add_subdirectory(formalism)
add_subdirectory(generators)
add_subdirectory(pddl)
add_subdirectory(search)

file(GLOB ALGORITHM_SRC_FILES "algorithms/*.cpp")
add_library(algorithms STATIC ${ALGORITHM_SRC_FILES} ${DATASTRUCTURES_SRC_FILES})
