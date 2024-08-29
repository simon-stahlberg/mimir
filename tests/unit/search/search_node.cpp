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

#include "mimir/search/search_node.hpp"

#include "cista/storage/byte_buffer_vector.hpp"
#include "mimir/search/search_node2.hpp"

#include <flatmemory/flatmemory.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

struct SearchNodeTag
{
};

TEST(MimirTests, SearchSearchNodeBuilderTest)
{
    // Build a search node.
    auto search_node_builder = SearchNodeBuilder<SearchNodeTag, double>();
    search_node_builder.set_status(SearchNodeStatus::OPEN);
    search_node_builder.get_property<0>() = 42;
    search_node_builder.finish();
    EXPECT_NE(search_node_builder.get_data(), nullptr);
    EXPECT_EQ(search_node_builder.get_size(), 24);

    // View the data generated in the builder.
    auto search_node_view = SearchNode<SearchNodeTag, double>(FlatSearchNode<double>(search_node_builder.get_data()));
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_view.get_parent_state(), std::numeric_limits<StateIndex>::max());
    EXPECT_EQ(search_node_view.get_creating_action(), std::numeric_limits<GroundActionIndex>::max());
    EXPECT_EQ(search_node_view.get_property<0>(), 42);

    // Test mutation of a search node
    search_node_view.set_status(SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_view.get_status(), SearchNodeStatus::CLOSED);
    search_node_view.set_property<0>(40);
    EXPECT_EQ(search_node_view.get_property<0>(), 40);
}

TEST(MimirTests, SearchSearchNodeVectorTest)
{
    /* A vector that automatically resizes when accessing elements at index i
       and creating default constructed objects.
       There is only 1 heap allocation every few thousand nodes that are being created. */

    auto builder = SearchNodeBuilder<SearchNodeTag, double>();
    builder.set_status(SearchNodeStatus::CLOSED);
    builder.get_property<0>() = 42;
    builder.get_flatmemory_builder().finish();

    auto vector = FlatSearchNodeVector<double>(std::move(builder.get_flatmemory_builder()));

    // Test default initialization a search node
    auto search_node_0 = SearchNode<SearchNodeTag, double>(vector[0]);
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_0.get_parent_state(), std::numeric_limits<StateIndex>::max());
    EXPECT_EQ(search_node_0.get_creating_action(), std::numeric_limits<GroundActionIndex>::max());
    EXPECT_EQ(search_node_0.get_property<0>(), 42);

    // Test mutation of a search node
    search_node_0.set_status(SearchNodeStatus::OPEN);
    EXPECT_EQ(search_node_0.get_status(), SearchNodeStatus::OPEN);
    search_node_0.set_property<0>(41);
    EXPECT_EQ(search_node_0.get_property<0>(), 41);

    // Test default initialization of a second search node
    auto search_node_1 = SearchNode<SearchNodeTag, double>(vector[1]);
    EXPECT_EQ(search_node_1.get_status(), SearchNodeStatus::CLOSED);
    EXPECT_EQ(search_node_1.get_property<0>(), 42);
}

TEST(MimirTests, SearchSearchNode2Test)
{
    auto node = CistaSearchNode<>();
    set_status(node, SearchNodeStatus::NEW);
    set_state(node, 2);
    set_action(node, 3);

    auto buf = cista::buf<std::vector<uint8_t>> {};
    cista::serialize(buf, node);

    auto deserialized_node = cista::deserialize<CistaSearchNode<>>(buf.base(), buf.base() + buf.size());
    EXPECT_EQ(get_status(*deserialized_node), SearchNodeStatus::NEW);
    EXPECT_EQ(get_state(*deserialized_node), 2);
    EXPECT_EQ(get_action(*deserialized_node), 3);

    set_status(*deserialized_node, SearchNodeStatus::OPEN);
    set_state(*deserialized_node, 4);
    set_action(*deserialized_node, 5);

    EXPECT_EQ(get_status(*deserialized_node), SearchNodeStatus::OPEN);
    EXPECT_EQ(get_state(*deserialized_node), 4);
    EXPECT_EQ(get_action(*deserialized_node), 5);
}

TEST(MimirTests, SearchSearchNode2VectorTest)
{
    auto node1 = CistaSearchNode<>();
    set_status(node1, SearchNodeStatus::NEW);
    set_state(node1, 2);
    set_action(node1, 3);

    auto node2 = CistaSearchNode<>();
    set_status(node2, SearchNodeStatus::OPEN);
    set_state(node2, 4);
    set_action(node2, 5);

    auto vec = cista::storage::ByteBufferVector<CistaSearchNode<>>();
    vec.push_back(node1);
    vec.push_back(node2);

    auto deserialized_node1 = vec[0];
    auto deserialized_node2 = vec[1];

    EXPECT_EQ(get_status(*deserialized_node1), SearchNodeStatus::NEW);
    EXPECT_EQ(get_state(*deserialized_node1), 2);
    EXPECT_EQ(get_action(*deserialized_node1), 3);

    EXPECT_EQ(get_status(*deserialized_node2), SearchNodeStatus::OPEN);
    EXPECT_EQ(get_state(*deserialized_node2), 4);
    EXPECT_EQ(get_action(*deserialized_node2), 5);
}
}
