#include <mimir/search/algorithms.hpp>

#include <mimir/formalism/problem/declarations.hpp>
#include <mimir/formalism/parser.hpp>
#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchAlgorithmsAstarGroundedBlindTest) {
    // Instantiate grounded version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto grounded_astar = Algorithm<AlgorithmDispatcher<AStarTag<GroundedTag, BlindTag>>>(parser.get_problem(), parser.get_factories());
    using ConstActionViewList = typename TypeTraits<decltype(grounded_astar)>::ConstActionViewList;
    ConstActionViewList plan;
    const auto search_status = grounded_astar.find_solution(plan);
}


TEST(MimirTests, SearchAlgorithmsAstarLiftedBlindTest) {
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto lifted_astar = Algorithm<AlgorithmDispatcher<AStarTag<LiftedTag, BlindTag>>>(parser.get_problem(), parser.get_factories());
    using ConstActionViewList = typename TypeTraits<decltype(lifted_astar)>::ConstActionViewList;
    ConstActionViewList plan;
    const auto search_status = lifted_astar.find_solution(plan);
}

}
