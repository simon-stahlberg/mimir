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

#include <gtest/gtest.h>

namespace mimir::tests
{

struct SearchNodeTag
{
};

TEST(MimirTests, SearchSearchNodeTest)
{
    using SearchNodeType = SearchNodeImpl<double>;

    auto node = SearchNodeType();
    set_status(&node, SearchNodeStatus::NEW);
    set_parent_state(&node, 2);
    set_creating_action(&node, 3);
    set_property<0>(&node, 3.14);

    auto buf = cista::buf<std::vector<uint8_t>> {};
    cista::serialize(buf, node);
    EXPECT_EQ(buf.size(), 24);

    auto deserialized_node = cista::deserialize<SearchNodeType>(buf.base(), buf.base() + buf.size());
    EXPECT_EQ(get_status(deserialized_node), SearchNodeStatus::NEW);
    EXPECT_EQ(get_parent_state(deserialized_node), 2);
    EXPECT_EQ(get_creating_action(deserialized_node), 3);
    EXPECT_EQ(get_property<0>(deserialized_node), 3.14);

    set_status(deserialized_node, SearchNodeStatus::OPEN);
    set_parent_state(deserialized_node, 4);
    set_creating_action(deserialized_node, 5);
    set_property<0>(deserialized_node, 9.99);

    EXPECT_EQ(get_status(deserialized_node), SearchNodeStatus::OPEN);
    EXPECT_EQ(get_parent_state(deserialized_node), 4);
    EXPECT_EQ(get_creating_action(deserialized_node), 5);
    EXPECT_EQ(get_property<0>(deserialized_node), 9.99);
}

TEST(MimirTests, SearchSearchNodeVectorTest)
{
    using SearchNodeType = SearchNodeImpl<double>;

    auto node1 = SearchNodeType();
    set_status(&node1, SearchNodeStatus::NEW);
    set_parent_state(&node1, 2);
    set_creating_action(&node1, 3);
    set_property<0>(&node1, 3.14);

    auto node2 = SearchNodeType();
    set_status(&node2, SearchNodeStatus::OPEN);
    set_parent_state(&node2, 4);
    set_creating_action(&node2, 5);
    set_property<0>(&node2, 9.99);

    auto vec = cista::storage::Vector<SearchNodeType>();
    vec.push_back(node1);
    vec.push_back(node2);

    auto deserialized_node1 = vec[0];
    auto deserialized_node2 = vec[1];

    EXPECT_EQ(get_status(deserialized_node1), SearchNodeStatus::NEW);
    EXPECT_EQ(get_parent_state(deserialized_node1), 2);
    EXPECT_EQ(get_creating_action(deserialized_node1), 3);
    EXPECT_EQ(get_property<0>(deserialized_node1), 3.14);

    EXPECT_EQ(get_status(deserialized_node2), SearchNodeStatus::OPEN);
    EXPECT_EQ(get_parent_state(deserialized_node2), 4);
    EXPECT_EQ(get_creating_action(deserialized_node2), 5);
    EXPECT_EQ(get_property<0>(deserialized_node2), 9.99);
}
}
