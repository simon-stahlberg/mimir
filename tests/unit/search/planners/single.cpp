#include <gtest/gtest.h>
#include <mimir/search/planners.hpp>
#include <string>

namespace mimir::tests
{

TEST(MimirTests, SearchPlannersSingleTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    auto planner = Planner<SingleTag<AStarTag<GroundedTag, BlindTag>>>(domain_file, problem_file);
    const auto [status, plan] = planner.find_solution();
}

}
