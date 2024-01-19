#include <mimir/search/config.hpp>
#include <mimir/search/search_node_builder.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchSearchNodeBuilderTest) {
    // Instantiate lifted version
    auto builder = Builder<SearchNode<Grounded>>();
    builder.set_status(mimir::SearchNodeStatus::OPEN);
    builder.set_g_value(42);
    builder.set_parent_state(nullptr);
    builder.set_ground_action(nullptr);
    builder.finish();
    builder.get_data();
    EXPECT_EQ(builder.get_size(), 24);
}

}
