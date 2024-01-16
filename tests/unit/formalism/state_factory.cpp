#include <mimir/common/config.hpp>
#include <mimir/search/state_factory.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedStateFactoryTest) {
    // Instantiate grounded version
    auto grounded_state_factory = StateFactory<Grounded>();
}


TEST(MimirTests, LiftedStateFactoryTest) {
    // Instantiate lifted version
    auto lifted_state_factory = StateFactory<Lifted>();
}

}
