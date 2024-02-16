#include <mimir/search/applicable_action_generators.hpp>

#include <mimir/formalism/parser.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchAAGsDefaultGroundedTest) {
    // Instantiate grounded version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto grounded_aag = AAG<AAGDispatcher<GroundedTag, BitsetStateTag>>(parser.get_problem(), parser.get_factories());
}

}
