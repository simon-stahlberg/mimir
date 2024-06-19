#include "mimir/datasets/faithful_abstraction.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsFaithfulAbstractionTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto abstraction = FaithfulAbstractionImpl::create(domain_file, problem_file, 10000, 10000);

    EXPECT_EQ(abstraction->get_num_abstract_states(), 12);
    EXPECT_EQ(abstraction->get_num_abstract_transitions(), 46);
    EXPECT_EQ(abstraction->get_num_abstract_goal_states(), 2);
    EXPECT_EQ(abstraction->get_num_abstract_deadend_states(), 0);
}
}
