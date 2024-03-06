#include <gtest/gtest.h>
#include <mimir/formalism/parser.hpp>
#include <mimir/formalism/problem/declarations.hpp>
#include <mimir/search/successor_state_generators.hpp>

namespace mimir::tests
{

TEST(MimirTests, SearchSSGBitsetTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    const auto problem = parser.get_problem();
    auto lifted_ssg = SSG<SSGDispatcher<LiftedTag, BitsetStateTag>>(problem);
    const auto initial_state = lifted_ssg.get_or_create_initial_state(problem);
}

}
