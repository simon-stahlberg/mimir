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
    "algorithms/memory_pool.cpp"
    "datasets/state_space.cpp"
    "formalism/translators/to_disjunctive_normal_form.cpp"
    "formalism/translators/to_negation_normal_form.cpp"
    "formalism/translators/remove_types.cpp"
    "formalism/translators/remove_universal_quantifiers.cpp"
    "search/algorithms/brfs.cpp"
    "search/algorithms/iw.cpp"
    "search/applicable_action_generators/dense-grounded.cpp"
    "search/applicable_action_generators/dense-lifted.cpp"
    "search/openlists/priority_queue.cpp"
    "search/planners/single.cpp"
    "search/search_nodes/cost.cpp"
    "search/successor_state_generators/dense.cpp"
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        mimir::core
        GTest::GTest
        GTest::Main)
add_test(domain_gtests ${TEST_NAME})
