#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/event_handlers.hpp"
#include "mimir/search/heuristics.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchAlgorithmsAstarGroundedBlindTest)
{
    // Instantiate grounded version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto state_repository = std::make_shared<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem());
    auto successor_generator = std::make_shared<AAG<GroundedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories());
    auto blind_heuristic = std::make_shared<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>();
    auto event_handler = std::make_shared<MinimalEventHandler>();
    auto grounded_astar = AStarAlgorithm(parser.get_problem(),
                                         parser.get_factories(),
                                         std::move(state_repository),
                                         std::move(successor_generator),
                                         std::move(blind_heuristic),
                                         std::move(event_handler));
    auto plan = std::vector<ConstView<ActionDispatcher<StateReprTag>>> {};
    const auto search_status = grounded_astar.find_solution(plan);
}

TEST(MimirTests, SearchAlgorithmsAstarLiftedBlindTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto state_repository = std::make_shared<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem());
    auto successor_generator = std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories());
    auto blind_heuristic = std::make_shared<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>();
    auto event_handler = std::make_shared<MinimalEventHandler>();
    auto lifted_astar = AStarAlgorithm(parser.get_problem(),
                                       parser.get_factories(),
                                       std::move(state_repository),
                                       std::move(successor_generator),
                                       std::move(blind_heuristic),
                                       std::move(event_handler));
    auto plan = std::vector<ConstView<ActionDispatcher<StateReprTag>>> {};
    const auto search_status = lifted_astar.find_solution(plan);
}

}
