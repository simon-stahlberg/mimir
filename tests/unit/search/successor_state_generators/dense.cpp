#include "mimir/formalism/formalism.hpp"
#include "mimir/search/successor_state_generator.hpp"

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
    auto lifted_aag = std::make_shared<LiftedAAG>(problem, parser.get_factories());
    auto ssg = SuccessorStateGenerator(lifted_aag);
    [[maybe_unused]] const auto initial_state = ssg.get_or_create_initial_state();
}

}
