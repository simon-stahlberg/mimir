#include <mimir/search/config.hpp>
#include <mimir/search/lifted/state_repository.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedStateRepositoryTest) {
    // Instantiate lifted version
    auto lifted_state_repository = StateRepository<Lifted>();
}

}
