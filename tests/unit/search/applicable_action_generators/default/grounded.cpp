#include <mimir/search/applicable_action_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchApplicableActionGeneratorsDefaultGroundedTest) {
    // Instantiate grounded version
    auto grounded_successor_generator = DefaultApplicableActionGenerator<GroundedTag>();
}

}
