#include <mimir/search/applicable_action_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchAAGsDefaultGroundedTest) {
    // Instantiate grounded version
    auto grounded_aag = AAG<AAGDispatcher<GroundedTag, BitsetStateTag>>();
}

}
