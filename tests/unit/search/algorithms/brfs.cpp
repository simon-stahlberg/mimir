#include <gtest/gtest.h>
#include <mimir/formalism/parser.hpp>
#include <mimir/formalism/declarations.hpp>
#include <mimir/search/algorithms.hpp>

namespace mimir::tests
{

TEST(MimirTests, SearchAlgorithmsBrFSGroundedTest)
{
    // Instantiate grounded version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto grounded_brfs = Algorithm<AlgorithmDispatcher<BrFSTag<GroundedTag>>>(parser.get_problem(), parser.get_factories());
    using ConstActionViewList = typename TypeTraits<decltype(grounded_brfs)>::ConstActionViewList;
    ConstActionViewList plan;
    const auto search_status = grounded_brfs.find_solution(plan);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto lifted_brfs = Algorithm<AlgorithmDispatcher<BrFSTag<LiftedTag>>>(parser.get_problem(), parser.get_factories());
    using ConstActionViewList = typename TypeTraits<decltype(lifted_brfs)>::ConstActionViewList;
    ConstActionViewList plan;
    const auto search_status = lifted_brfs.find_solution(plan);
}

}
