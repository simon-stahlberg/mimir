#include <gtest/gtest.h>
#include <mimir/formalism/parser.hpp>
#include <mimir/search/applicable_action_generators.hpp>

namespace mimir::tests
{

TEST(MimirTests, SearchAAGsDefaultLiftedTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto lifted_aag = AAG<AAGDispatcher<LiftedTag, BitsetStateTag>>(parser.get_problem(), parser.get_factories());
}

}
