#include <mimir/search/config.hpp>
#include <mimir/search/grounded/state_repository.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedStateRepositoryTest) {
    // Instantiate grounded version
    auto grounded_state_repository = StateRepository<Grounded>();
}

}
