#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/event_handlers.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/planners.hpp"

#include <gtest/gtest.h>
#include <string>

namespace mimir::tests
{

TEST(MimirTests, SearchPlannersSingleTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto state_repository = std::make_unique<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem());
    auto successor_generator = std::make_unique<AAG<LiftedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories());
    auto blind_heuristic = std::make_unique<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>();
    auto event_handler = std::make_unique<MinimalEventHandler>();
    auto lifted_astar = std::make_unique<AStarAlgorithm>(parser.get_problem(),
                                                         parser.get_factories(),
                                                         std::move(state_repository),
                                                         std::move(successor_generator),
                                                         std::move(blind_heuristic),
                                                         std::move(event_handler));

    auto planner = SinglePlanner(parser.get_domain(), parser.get_problem(), parser.get_factories(), std::move(lifted_astar));
    const auto [status, plan] = planner.find_solution();
}

}
