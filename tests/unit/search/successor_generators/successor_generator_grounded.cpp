#include <mimir/common/config.hpp>
#include <mimir/search/successor_generators/successor_generator_grounded.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedSuccessorGeneratorTest) {
    // Instantiate grounded version
    auto grounded_successor_generator = search::SuccessorGenerator<Grounded>();
}

}
