file(GLOB SRC_FILES "*.cpp")
add_library(pddl STATIC ${SRC_FILES})
target_link_libraries(pddl formalism)
