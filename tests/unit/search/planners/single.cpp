#include <gtest/gtest.h>
#include <mimir/formalism/parser.hpp>
#include <mimir/search/algorithms/astar/astar.hpp>
#include <mimir/search/heuristics/blind/blind.hpp>
#include <mimir/search/planners.hpp>
#include <string>

namespace mimir::tests
{

TEST(MimirTests, SearchPlannersSingleTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto planner = Planner<SingleTag<AStarTag<GroundedTag, BlindTag>>>(parser.get_domain(), parser.get_problem(), parser.get_factories());
    const auto [status, plan] = planner.find_solution();
}

}
