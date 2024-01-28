#include <mimir/search/actions.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchActionsDefaultTest) {
    // Build a state.
    auto builder = Builder<ActionDispatcher<DefaultActionTag, GroundedTag, BitsetStateTag>>();
    builder.clear();
}

}
