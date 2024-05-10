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
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto successor_generator = std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories());
    auto state_repository = std::make_shared<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem(), successor_generator);
    auto blind_heuristic = std::make_shared<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>();
    auto event_handler = std::make_shared<MinimalEventHandler>();
    auto lifted_astar =
        std::make_shared<AStarAlgorithm>(parser.get_problem(), parser.get_factories(), state_repository, successor_generator, blind_heuristic, event_handler);

    auto planner = SinglePlanner(parser.get_domain(), parser.get_problem(), parser.get_factories(), lifted_astar);
    const auto [status, plan] = planner.find_solution();
}

}
