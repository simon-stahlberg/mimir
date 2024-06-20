#include "mimir/datasets/faithful_abstraction.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsFaithfulAbstractionTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto abstraction = FaithfulAbstraction::create(domain_file, problem_file, 10000, 10000).value();

    EXPECT_EQ(abstraction.get_num_states(), 12);
    EXPECT_EQ(abstraction.get_num_transitions(), 46);
    EXPECT_EQ(abstraction.get_num_goal_states(), 2);
    EXPECT_EQ(abstraction.get_num_deadend_states(), 0);
}
}
