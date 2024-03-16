add_executable(planner "planner.cpp")
target_link_options(planner PRIVATE -static-libstdc++)
target_link_libraries(planner mimir::core)
