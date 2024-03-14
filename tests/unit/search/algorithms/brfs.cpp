#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/event_handlers.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchAlgorithmsBrFSGroundedTest)
{
    // Instantiate grounded version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto state_repository = std::make_unique<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem());
    auto successor_generator = std::make_unique<AAG<GroundedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories());
    auto event_handler = std::make_unique<MinimalEventHandler>();
    auto grounded_brfs =
        BrFsAlgorithm(parser.get_problem(), parser.get_factories(), std::move(state_repository), std::move(successor_generator), std::move(event_handler));
    auto plan = std::vector<ConstView<ActionDispatcher<StateReprTag>>> {};
    const auto search_status = grounded_brfs.find_solution(plan);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto state_repository = std::make_unique<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem());
    auto successor_generator = std::make_unique<AAG<LiftedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories());
    auto event_handler = std::make_unique<MinimalEventHandler>();
    auto lifted_brfs =
        BrFsAlgorithm(parser.get_problem(), parser.get_factories(), std::move(state_repository), std::move(successor_generator), std::move(event_handler));
    auto plan = std::vector<ConstView<ActionDispatcher<StateReprTag>>> {};
    const auto search_status = lifted_brfs.find_solution(plan);
}

}
