#include <mimir/search/config.hpp>
#include <mimir/search/state_view.hpp>
#include <mimir/search/grounded/state_builder.hpp>
#include <mimir/search/grounded/state_view.hpp>
#include <mimir/search/search_node_builder.hpp>
#include <mimir/search/search_node_view.hpp>
#include <mimir/search/search_node.hpp>
#include <mimir/buffer/containers/vector.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchNodeBuilderTest) {
    // Build a state.
    auto state_builder = Builder<State<GroundedTag>>();
    state_builder.set_id(5);
    state_builder.finish();
    EXPECT_NE(state_builder.get_buffer().get_data(), nullptr);
    EXPECT_EQ(state_builder.get_buffer().get_size(), 8);
    auto state_view = View<State<GroundedTag>>(state_builder.get_buffer().get_data());

    // Build a search node.
    auto search_node_builder = Builder<SearchNode<GroundedTag>>();
    search_node_builder.set_status(SearchNodeStatus::OPEN);
    search_node_builder.set_g_value(42);
    search_node_builder.set_parent_state(state_view);
    search_node_builder.set_ground_action(nullptr);
    search_node_builder.finish();
    EXPECT_NE(search_node_builder.get_buffer().get_data(), nullptr);
    EXPECT_EQ(search_node_builder.get_buffer().get_size(), 28);

    // View the data generated in the builder.
    auto search_node_view = View<SearchNode<GroundedTag>>(search_node_builder.get_buffer().get_data());
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_view.get_g_value(), 42);
    EXPECT_EQ(search_node_view.get_parent_state().get_id(), 5);
}


TEST(MimirTests, SearchNodeBuilderVectorTest) {
    /* A vector that automatically resizes when accessing elements at index i
       and creating default constructed objects.
       There is only 1 heap allocation every few thousand nodes that are being created. */
    auto vector = AutomaticVector<SearchNode<GroundedTag>>(
        Builder<SearchNode<GroundedTag>>(SearchNodeStatus::CLOSED, 42, View<State<GroundedTag>>(nullptr), nullptr));

    // Test default initialization a search node
    auto search_node_0 = vector[0];
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_g_value(), 42);
    EXPECT_EQ(search_node_0.get_ground_action(), nullptr);

    // Test mutation of a search node
    search_node_0.get_status() = SearchNodeStatus::OPEN;
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::OPEN);
    search_node_0.get_g_value() = 41;
    EXPECT_EQ(search_node_0.get_g_value(), 41);

    // Test default initialization of a second search node
    auto search_node_1 = vector[1];
    EXPECT_EQ(search_node_1.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_1.get_g_value(), 42);
    EXPECT_EQ(search_node_1.get_ground_action(), nullptr);
}

}
