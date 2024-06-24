#include "mimir/datasets/global_faithful_abstraction.hpp"

#include "mimir/datasets/state_space.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsGlobalFaithfulAbstractionCreateTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_3 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2, problem_file_3 };

    const auto abstractions = GlobalFaithfulAbstraction::create(domain_file, problem_files);

    // Problem 1 was pruned because it has 0 global non isomorphic states.
    EXPECT_EQ(abstractions.size(), 2);

    // Reduced from 8 to 6 abstract states.
    const auto& abstraction_0 = abstractions.at(0);
    EXPECT_EQ(abstraction_0.get_num_states(), 6);
    EXPECT_EQ(abstraction_0.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_0.get_num_non_isomorphic_states(), 6);
    // Reduced from 28 to 12 abstract states.
    const auto& abstraction_1 = abstractions.at(1);
    EXPECT_EQ(abstraction_1.get_num_states(), 12);
    EXPECT_EQ(abstraction_1.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_1.get_num_non_isomorphic_states(), 12);

    auto memories = std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>> {};
    for (const auto& abstraction : abstractions.at(0).get_abstractions())
    {
        memories.emplace_back(abstraction.get_pddl_parser(), abstraction.get_aag(), abstraction.get_ssg());
    }
    auto state_spaces = StateSpace::create(memories);
}
}
