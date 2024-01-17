#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/grounded/state_repository.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedStateRepositoryTest) {
    // Instantiate grounded version
    auto grounded_state_repository = StateRepository<Grounded>();
    auto problem = static_cast<Problem>(nullptr);
    const auto& initial_state = grounded_state_repository.get_or_create_initial_state(problem);
    const auto initial_state_id = initial_state.get_id();
}

}
