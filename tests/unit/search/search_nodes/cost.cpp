#include "mimir/search/search_nodes.hpp"

#include <flatmemory/flatmemory.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

using SearchNodeLayout = flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t, int32_t>;
using SearchNodeBuilder = flatmemory::Builder<SearchNodeLayout>;
using SearchNodeView = flatmemory::View<SearchNodeLayout>;
using SearchNodeVector = flatmemory::FixedSizedTypeVector<SearchNodeLayout>;

TEST(MimirTests, SearchSearchNodesCostBuilderTest)
{
    // Build a search node.
    auto search_node_builder = CostSearchNodeBuilder(SearchNodeBuilder());
    search_node_builder.set_status(SearchNodeStatus::OPEN);
    search_node_builder.set_g_value(42);
    search_node_builder.set_parent_state_id(100);
    search_node_builder.set_creating_action_id(99);
    search_node_builder.finish();
    EXPECT_NE(search_node_builder.get_data(), nullptr);
    EXPECT_EQ(search_node_builder.get_size(), 20);

    // View the data generated in the builder.
    auto search_node_view = CostSearchNode(flat::CostSearchNode(search_node_builder.get_data()));
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_view.get_g_value(), 42);
    EXPECT_EQ(search_node_view.get_parent_state_id(), 100);
    EXPECT_EQ(search_node_view.get_creating_action_id(), 99);

    // Test mutation of a search node
    search_node_view.get_status() = SearchNodeStatus::CLOSED;
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::CLOSED);
    search_node_view.get_g_value() = 40;
    EXPECT_EQ(search_node_view.get_g_value(), 40);
}

TEST(MimirTests, SearchSearchNodesCostVectorTest)
{
    /* A vector that automatically resizes when accessing elements at index i
       and creating default constructed objects.
       There is only 1 heap allocation every few thousand nodes that are being created. */

    flatmemory::Builder<SearchNodeLayout> builder;
    builder.get<0>() = SearchNodeStatus::CLOSED;
    builder.get<1>() = 42;
    builder.get<2>() = -1;
    builder.finish();

    auto vector = SearchNodeVector(std::move(builder));

    // Test default initialization a search node
    auto search_node_0 = CostSearchNode(vector[0]);
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_g_value(), 42);
    EXPECT_EQ(search_node_0.get_parent_state_id(), -1);

    // Test mutation of a search node
    search_node_0.get_status() = SearchNodeStatus::OPEN;
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::OPEN);
    search_node_0.get_g_value() = 41;
    EXPECT_EQ(search_node_0.get_g_value(), 41);

    // Test default initialization of a second search node
    auto search_node_1 = CostSearchNode(vector[1]);
    EXPECT_EQ(search_node_1.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_1.get_g_value(), 42);
}

}
