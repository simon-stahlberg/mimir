#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/lifted/state_repository.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedStateRepositoryTest) {
    // Instantiate lifted version
    auto lifted_state_repository = StateRepository<Lifted>();
    auto problem = static_cast<Problem>(nullptr);
    auto initial_state = lifted_state_repository.get_or_create_initial_state(problem);
    initial_state.get_id();
}

}
