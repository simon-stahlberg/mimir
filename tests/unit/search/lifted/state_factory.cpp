#include <mimir/search/config.hpp>
#include <mimir/search/lifted/state_factory.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedStateFactoryTest) {
    // Instantiate lifted version
    auto lifted_state_factory = StateFactory<Lifted>();
}

}
