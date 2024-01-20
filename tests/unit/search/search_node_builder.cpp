#include <mimir/search/config.hpp>
#include <mimir/search/search_node_builder.hpp>
#include <mimir/search/search_node_view.hpp>
#include <mimir/search/search_node.hpp>
#include <mimir/buffer/containers/vector.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <cstdint>


namespace mimir::tests
{

TEST(MimirTests, SearchNodeBuilderTest) {
    auto builder = Builder<SearchNode<Grounded>>();
    builder.set_status(mimir::SearchNodeStatus::OPEN);
    builder.set_g_value(42);
    builder.set_parent_state(nullptr);
    builder.set_ground_action(nullptr);
    builder.finish();
    EXPECT_NE(builder.get_buffer().get_data(), nullptr);
    EXPECT_EQ(builder.get_buffer().get_size(), 28);
}

TEST(MimirTests, SearchNodeVectorTest) {
    auto vector = AutomaticVector<SearchNode<Grounded>>(Builder<SearchNode<Grounded>>(SearchNodeStatus::CLOSED, 42, nullptr, nullptr));
    auto search_node_0 = vector[0];
    // Test default initialization a search node
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_g_value(), 42);
    EXPECT_EQ(search_node_0.get_parent_state(), nullptr);
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
    EXPECT_EQ(search_node_1.get_parent_state(), nullptr);
    EXPECT_EQ(search_node_1.get_ground_action(), nullptr);
}

}
