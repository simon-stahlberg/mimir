#include <mimir/common/config.hpp>
#include <mimir/search/grounded/successor_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedSuccessorGeneratorTest) {
    // Instantiate grounded version
    auto grounded_successor_generator = SuccessorGenerator<Grounded>();
}

}
