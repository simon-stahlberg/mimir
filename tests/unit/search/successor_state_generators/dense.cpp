#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchSSGDenseTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    const auto problem = parser.get_problem();
    auto lifted_aag = std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(problem, parser.get_factories());
    auto ssg = SSG<SSGDispatcher<DenseStateTag>>(lifted_aag);
    const auto initial_state = ssg.get_or_create_initial_state();
}

}
