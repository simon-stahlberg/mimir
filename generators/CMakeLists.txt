file(GLOB SRC_FILES "*.cpp")
add_library(generators STATIC ${SRC_FILES})
target_link_libraries(generators algorithms)
target_link_libraries(generators formalism)
