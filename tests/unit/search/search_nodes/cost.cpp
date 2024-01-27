#include <mimir/search/search_nodes.hpp>
#include <mimir/buffer/containers/vector.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchSearchNodesCostBuilderTest) {
    // Build a state.
    auto state_builder = Builder<StateDispatcher<BitsetStateTag, GroundedTag>>();
    state_builder.set_id(5);
    state_builder.finish();
    EXPECT_NE(state_builder.get_buffer_pointer(), nullptr);
    EXPECT_EQ(state_builder.get_size(), 8);
    auto state_view = View<StateDispatcher<BitsetStateTag, GroundedTag>>(state_builder.get_buffer_pointer());

    // Build a ground action.
    auto action_builder = Builder<ActionDispatcher<DefaultActionTag, GroundedTag, BitsetStateTag>>();
    action_builder.finish();
    auto action_view = View<ActionDispatcher<DefaultActionTag, GroundedTag, BitsetStateTag>>(action_builder.get_buffer_pointer());

    // Build a search node.
    auto search_node_builder = Builder<CostSearchNodeTag<GroundedTag, BitsetStateTag, DefaultActionTag>>();
    search_node_builder.set_status(SearchNodeStatus::OPEN);
    search_node_builder.set_g_value(42);
    search_node_builder.set_parent_state(state_view);
    search_node_builder.set_ground_action(action_view);
    search_node_builder.finish();
    EXPECT_NE(search_node_builder.get_buffer_pointer(), nullptr);
    EXPECT_EQ(search_node_builder.get_size(), 28);

    // View the data generated in the builder.
    auto search_node_view = View<CostSearchNodeTag<GroundedTag, BitsetStateTag, DefaultActionTag>>(search_node_builder.get_buffer_pointer());
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_view.get_g_value(), 42);
    EXPECT_EQ(search_node_view.get_parent_state().get_id(), 5);
}


TEST(MimirTests, SearchSearchNodesCostVectorTest) {
    /* A vector that automatically resizes when accessing elements at index i
       and creating default constructed objects.
       There is only 1 heap allocation every few thousand nodes that are being created. */
    auto vector = AutomaticVector<CostSearchNodeTag<GroundedTag, BitsetStateTag, DefaultActionTag>>(
        Builder<CostSearchNodeTag<GroundedTag, BitsetStateTag, DefaultActionTag>>(
            SearchNodeStatus::CLOSED,
            42,
            View<StateDispatcher<BitsetStateTag, GroundedTag>>(nullptr),
            View<ActionDispatcher<DefaultActionTag, GroundedTag, BitsetStateTag>>(nullptr))
    );

    // Test default initialization a search node
    auto search_node_0 = vector[0];
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_g_value(), 42);
    auto state = search_node_0.get_parent_state();
    auto action = search_node_0.get_ground_action();

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
