#include <mimir/search/config.hpp>
#include <mimir/search/grounded/state_factory.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedStateFactoryTest) {
    // Instantiate grounded version
    auto grounded_state_factory = StateFactory<Grounded>();
}

}
