#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/planners.hpp"

#include <gtest/gtest.h>
#include <string>

namespace mimir::tests
{

TEST(MimirTests, SearchPlannersSingleTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto successor_generator = std::make_shared<LiftedAAG>(parser.get_problem(), parser.get_factories());
    auto blind_heuristic = std::make_shared<BlindHeuristic>();
    auto lifted_astar = std::make_shared<AStarAlgorithm>(successor_generator, blind_heuristic);
    auto planner = SinglePlanner(lifted_astar);
    const auto [status, plan] = planner.find_solution();
}

}
