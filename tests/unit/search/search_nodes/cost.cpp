#include <mimir/search/search_nodes.hpp>
#include <mimir/buffer/containers/vector.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

using StateBuilder = Builder<StateDispatcher<BitsetStateTag, GroundedTag>>;
using StateView = View<StateDispatcher<BitsetStateTag, GroundedTag>>;
using ActionBuilder = Builder<ActionDispatcher<GroundedTag, BitsetStateTag>>;
using ActionView = View<ActionDispatcher<GroundedTag, BitsetStateTag>>;
using SearchNodeBuilder = Builder<CostSearchNodeTag<GroundedTag, BitsetStateTag>>;
using SearchNodeView = View<CostSearchNodeTag<GroundedTag, BitsetStateTag>>;
using SearchNodeVector = AutomaticVector<CostSearchNodeTag<GroundedTag, BitsetStateTag>>;


TEST(MimirTests, SearchSearchNodesCostBuilderTest) {
    // Build a search node.
    auto search_node_builder = SearchNodeBuilder();
    search_node_builder.set_status(SearchNodeStatus::OPEN);
    search_node_builder.set_g_value(42);
    search_node_builder.set_parent_state_id(100);
    search_node_builder.finish();
    EXPECT_NE(search_node_builder.get_buffer_pointer(), nullptr);
    EXPECT_EQ(search_node_builder.get_size(), 12);

    // View the data generated in the builder.
    auto search_node_view = SearchNodeView(search_node_builder.get_buffer_pointer());
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_view.get_g_value(), 42);
    EXPECT_EQ(search_node_view.get_parent_state_id(), 100);

    // Test mutation of a search node
    search_node_view.set_status(SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::CLOSED);
    search_node_view.set_g_value(40);
    EXPECT_EQ(search_node_view.get_g_value(), 40);
}


TEST(MimirTests, SearchSearchNodesCostVectorTest) {
    /* A vector that automatically resizes when accessing elements at index i
       and creating default constructed objects.
       There is only 1 heap allocation every few thousand nodes that are being created. */

    // Build a state.
    auto state_builder = StateBuilder();
    state_builder.set_id(5);
    state_builder.finish();
    EXPECT_NE(state_builder.get_buffer_pointer(), nullptr);
    EXPECT_EQ(state_builder.get_size(), 48);
    auto state_view = StateView(state_builder.get_buffer_pointer());

    auto vector = SearchNodeVector(
        SearchNodeBuilder(SearchNodeStatus::CLOSED, 42, -1)
    );

    // Test default initialization a search node
    auto search_node_0 = vector[0];
    EXPECT_EQ(search_node_0.get_size(), 12);
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_g_value(), 42);
    EXPECT_EQ(search_node_0.get_parent_state_id(), -1);

    // Test mutation of a search node
    search_node_0.set_status(SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::OPEN);
    search_node_0.set_g_value(41);
    EXPECT_EQ(search_node_0.get_g_value(), 41);

    // Test default initialization of a second search node
    auto search_node_1 = vector[1];
    EXPECT_EQ(search_node_1.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_1.get_g_value(), 42);

}

}
