add_executable(planner_brfs "planner_brfs.cpp")
target_link_libraries(planner_brfs mimir::core)

add_executable(planner_iw "planner_iw.cpp")
target_link_libraries(planner_iw mimir::core)

add_executable(planner_siw "planner_siw.cpp")
target_link_libraries(planner_siw mimir::core)
