#include "mimir/datasets/state_space.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsStateSpaceCreateTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    EXPECT_EQ(state_space.get_num_states(), 28);
    EXPECT_EQ(state_space.get_num_transitions(), 104);
    EXPECT_EQ(state_space.get_num_goal_states(), 2);
    EXPECT_EQ(state_space.get_num_deadend_states(), 0);
}

TEST(MimirTests, DatasetsStateSpaceCreateParallelTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto state_spaces = StateSpace::create(domain_file, problem_files);

    EXPECT_EQ(state_spaces.size(), 2);
}
}
