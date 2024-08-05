/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/search_nodes.hpp"

#include <flatmemory/flatmemory.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchSearchNodesUninformedBuilderTest)
{
    // Build a search node.
    auto search_node_builder = UninformedSearchNodeBuilder();
    search_node_builder.set_status(SearchNodeStatus::OPEN);
    search_node_builder.set_g_value(42);
    search_node_builder.set_parent_state(std::nullopt);
    search_node_builder.set_creating_action(std::nullopt);
    search_node_builder.finish();
    EXPECT_NE(search_node_builder.get_data(), nullptr);
    EXPECT_EQ(search_node_builder.get_size(), 48);

    // View the data generated in the builder.
    auto search_node_view = UninformedSearchNode(FlatUninformedSearchNode(search_node_builder.get_data()));
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_view.get_g_value(), 42);
    EXPECT_EQ(search_node_view.get_parent_state(), std::nullopt);
    EXPECT_EQ(search_node_view.get_creating_action(), std::nullopt);

    // Test mutation of a search node
    search_node_view.get_status() = SearchNodeStatus::CLOSED;
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::CLOSED);
    search_node_view.get_g_value() = 40;
    EXPECT_EQ(search_node_view.get_g_value(), 40);
}

TEST(MimirTests, SearchSearchNodesUninformedVectorTest)
{
    /* A vector that automatically resizes when accessing elements at index i
       and creating default constructed objects.
       There is only 1 heap allocation every few thousand nodes that are being created. */

    auto builder = UninformedSearchNodeBuilder();
    builder.set_status(SearchNodeStatus::CLOSED);
    builder.set_g_value(42);
    builder.set_parent_state(std::nullopt);
    builder.get_flatmemory_builder().finish();

    auto vector = FlatUninformedSearchNodeVector(std::move(builder.get_flatmemory_builder()));

    // Test default initialization a search node
    auto search_node_0 = UninformedSearchNode(vector[0]);
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_g_value(), 42);
    EXPECT_EQ(search_node_0.get_parent_state(), std::nullopt);

    // Test mutation of a search node
    search_node_0.get_status() = SearchNodeStatus::OPEN;
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::OPEN);
    search_node_0.get_g_value() = 41;
    EXPECT_EQ(search_node_0.get_g_value(), 41);

    // Test default initialization of a second search node
    auto search_node_1 = UninformedSearchNode(vector[1]);
    EXPECT_EQ(search_node_1.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_1.get_g_value(), 42);
}

}
