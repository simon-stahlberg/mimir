
add_executable(planner_astar "planner_astar.cpp")
target_link_libraries(planner_astar mimir::core)

add_executable(planner_gbfs "planner_gbfs.cpp")
target_link_libraries(planner_gbfs mimir::core)

add_executable(planner_brfs "planner_brfs.cpp")
target_link_libraries(planner_brfs mimir::core)

add_executable(planner_iw "planner_iw.cpp")
target_link_libraries(planner_iw mimir::core)

add_executable(planner_siw "planner_siw.cpp")
target_link_libraries(planner_siw mimir::core)

add_executable(kb "knowledge_base.cpp")
target_link_libraries(kb mimir::core)
