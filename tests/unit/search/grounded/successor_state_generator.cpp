#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/grounded/successor_state_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedSuccessorStateGeneratorTest) {
    // Instantiate grounded version
    auto grounded_state_repository = SuccessorStateGenerator<GroundedTag>();
    auto problem = static_cast<Problem>(nullptr);
    const auto initial_state = grounded_state_repository.get_or_create_initial_state(problem);
}

}
