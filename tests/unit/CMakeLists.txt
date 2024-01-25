enable_testing()

find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(GTest)

file(GLOB_RECURSE MIMIR_TEST_SOURCE_FILES
    "*.cpp" "**/*.cpp")
file(GLOB_RECURSE MIMIR_TEST_HEADER_FILES
    "*.hpp" "**/*.hpp")

set(TEST_NAME ${CMAKE_PROJECT_NAME}_tests)

# add_executable(${TEST_NAME} ${MIMIR_TEST_SOURCE_FILES} ${MIMIR_TEST_HEADER_FILES})

add_executable(${TEST_NAME}
    "search/algorithms/astar.cpp"
    "search/algorithms/brfs.cpp"
    "search/applicable_action_generators/default-grounded.cpp"
    "search/applicable_action_generators/default-lifted.cpp"
    "search/heuristics/blind.cpp"
    "search/openlists/priority_queue.cpp"
    "search/planners/parallel.cpp"
    "search/planners/sequential.cpp"
    "search/search_nodes/cost.cpp"
    "search/states/bitset-grounded.cpp"
    "search/states/bitset-lifted.cpp"
    "search/successor_state_generators/default-grounded.cpp"
    "search/successor_state_generators/default-lifted.cpp"
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        mimir::core
        GTest::GTest
        GTest::Main)

add_test(domain_gtests ${TEST_NAME})
