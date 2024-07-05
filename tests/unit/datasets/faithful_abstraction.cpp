#include "mimir/datasets/faithful_abstraction.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsFaithfulAbstractionCreateTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto [abstraction, states_by_certificate] = FaithfulAbstraction::create(domain_file, problem_file, false, true, true, true).value();

    EXPECT_EQ(abstraction.get_num_states(), 12);
    EXPECT_EQ(abstraction.get_num_transitions(), 36);
    EXPECT_EQ(abstraction.get_num_goal_states(), 2);
    EXPECT_EQ(abstraction.get_num_deadend_states(), 0);
}

TEST(MimirTests, DatasetsFaithfulAbstractionCreateParallelTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto abstractions_data = FaithfulAbstraction::create(domain_file, problem_files);

    EXPECT_EQ(abstractions_data.size(), 2);

    // Reduced from 8 to 6 abstract states.
    const auto& [abstraction_0, states_by_certificate_0] = abstractions_data.at(0);
    EXPECT_EQ(abstraction_0.get_num_states(), 6);
    // Reduced from 28 to 12 abstract states.
    const auto& [abstraction_1, states_by_certificate_1] = abstractions_data.at(1);
    EXPECT_EQ(abstraction_1.get_num_states(), 12);
}
}
