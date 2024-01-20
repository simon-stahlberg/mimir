#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/lifted/successor_state_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedSuccessorStateGeneratorTest) {
    // Instantiate lifted version
    auto lifted_state_repository = SuccessorStateGenerator<Lifted>();
    auto problem = static_cast<Problem>(nullptr);
    const auto initial_state = lifted_state_repository.get_or_create_initial_state(problem);
}

}
