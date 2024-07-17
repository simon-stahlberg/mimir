enable_testing()

find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(GTest)

# Helper function to create and register a test executable
function(add_gtest test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE mimir::core GTest::GTest GTest::Main)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add each test source file as a separate test executable
add_gtest(memory_pool_test                                 "algorithms/memory_pool.cpp")
add_gtest(nauty_test                                       "algorithms/nauty.cpp")

add_gtest(grouped_vector_test                              "common/grouped_vector.cpp")

add_gtest(faithful_abstraction_test                        "datasets/faithful_abstraction.cpp")
add_gtest(global_faithful_abstraction_test                 "datasets/global_faithful_abstraction.cpp")
add_gtest(state_space_test                                 "datasets/state_space.cpp")

add_gtest(to_positive_normal_form_test                     "formalism/transformers/to_positive_normal_form.cpp")
add_gtest(to_disjunctive_normal_form_test                  "formalism/translators/to_disjunctive_normal_form.cpp")
add_gtest(to_negation_normal_form_test                     "formalism/translators/to_negation_normal_form.cpp")
add_gtest(remove_types_test                                "formalism/translators/remove_types.cpp")
add_gtest(remove_universal_quantifiers_test                "formalism/translators/remove_universal_quantifiers.cpp")

add_gtest(object_graph_test                                "graphs/object_graph.cpp")
add_gtest(object_graph_pruning_strategy_test               "graphs/object_graph_pruning_strategy.cpp")
add_gtest(tuple_graph_test                                 "graphs/tuple_graph.cpp")

add_gtest(dl_constructors_test                             "languages/description_logics/constructors.cpp")
add_gtest(dl_parser_test                                   "languages/description_logics/parser.cpp")
add_gtest(dl_grammar_test                                  "languages/description_logics/grammar.cpp")

add_gtest(brfs_test                                        "search/algorithms/brfs.cpp")
add_gtest(iw_test                                          "search/algorithms/iw.cpp")
add_gtest(dense_grounded_test                              "search/applicable_action_generators/dense-grounded.cpp")
add_gtest(dense_lifted_test                                "search/applicable_action_generators/dense-lifted.cpp")
add_gtest(priority_queue_test                              "search/openlists/priority_queue.cpp")
add_gtest(single_test                                      "search/planners/single.cpp")
add_gtest(uninformed_test                                  "search/search_nodes/uninformed.cpp")
add_gtest(successor_state_generators_test                  "search/successor_state_generator.cpp")
