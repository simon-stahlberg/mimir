#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchAAGsDenseGroundedTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto aag_event_handler = std::make_shared<DefaultGroundedAAGEventHandler>();
    auto aag = std::make_shared<GroundedAAG>(parser.get_problem(), parser.get_factories(), aag_event_handler);
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    auto brfs_event_handler = std::make_shared<DefaultBrFSAlgorithmEventHandler>();
    auto brfs = BrFSAlgorithm(aag, ssg, brfs_event_handler);
    auto ground_actions = GroundActionList {};
    const auto status = brfs.find_solution(ground_actions);
    EXPECT_EQ(status, SearchStatus::SOLVED);

    const auto& aag_statistics = aag_event_handler->get_statistics();
    EXPECT_EQ(aag_statistics.get_num_delete_free_reachable_fluent_ground_atoms(), 9);
    EXPECT_EQ(aag_statistics.get_num_delete_free_reachable_derived_ground_atoms(), 8);
    EXPECT_EQ(aag_statistics.get_num_delete_free_actions(), 7);
    EXPECT_EQ(aag_statistics.get_num_delete_free_axioms(), 20);

    EXPECT_EQ(aag_statistics.get_num_ground_actions(), 10);
    EXPECT_EQ(aag_statistics.get_num_nodes_in_action_match_tree(), 12);

    EXPECT_EQ(aag_statistics.get_num_ground_axioms(), 16);
    EXPECT_EQ(aag_statistics.get_num_nodes_in_axiom_match_tree(), 12);

    const auto& brfs_statistics = brfs_event_handler->get_statistics();

    EXPECT_EQ(brfs_statistics.get_num_generated_until_f_value().back(), 94);
    EXPECT_EQ(brfs_statistics.get_num_expanded_until_f_value().back(), 36);
}

}
