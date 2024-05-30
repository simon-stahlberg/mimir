#include "mimir/datasets/state_space.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsStateSpaceTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);

    const auto state_space = StateSpaceImpl::create(domain_file, problem_file, 10000, 10000);

    EXPECT_EQ(state_space->get_num_states(), 28);
    EXPECT_EQ(state_space->get_num_transitions(), 104);
    EXPECT_EQ(state_space->get_num_goal_states(), 8);
    EXPECT_EQ(state_space->get_num_deadend_states(), 0);
}
}
