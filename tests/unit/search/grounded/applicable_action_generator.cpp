#include <mimir/search/config.hpp>
#include <mimir/search/grounded/applicable_action_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedApplicableActionGeneratorTest) {
    // Instantiate grounded version
    auto grounded_successor_generator = ApplicableActionGenerator<Grounded>();
}

}
