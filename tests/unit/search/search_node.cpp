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

using namespace mimir::search;

namespace mimir::tests
{

struct SearchNodeTag
{
};

TEST(MimirTests, SearchSearchNodeTest)
{
    using SearchNodeType = SearchNodeImpl<double>;

    auto node = SearchNodeType();
    node.get_status() = SearchNodeStatus::NEW;
    node.get_parent_state() = 2;
    node.get_property<0>() = 3.14;

    auto buf = cista::buf<std::vector<uint8_t>> {};
    cista::serialize(buf, node);
    EXPECT_EQ(buf.size(), 16);

    auto deserialized_node = cista::deserialize<SearchNodeType>(buf.base(), buf.base() + buf.size());
    EXPECT_EQ(deserialized_node->get_status(), SearchNodeStatus::NEW);
    EXPECT_EQ(deserialized_node->get_parent_state(), 2);
    EXPECT_EQ(deserialized_node->get_property<0>(), 3.14);

    deserialized_node->get_status() = SearchNodeStatus::OPEN;
    deserialized_node->get_parent_state() = 4;
    deserialized_node->get_property<0>() = 9.99;

    EXPECT_EQ(deserialized_node->get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(deserialized_node->get_parent_state(), 4);
    EXPECT_EQ(deserialized_node->get_property<0>(), 9.99);
}

TEST(MimirTests, SearchSearchNodeVectorTest)
{
    using SearchNodeType = SearchNodeImpl<double>;

    auto node1 = SearchNodeType();
    node1.get_status() = SearchNodeStatus::NEW;
    node1.get_parent_state() = 2;
    node1.get_property<0>() = 3.14;

    auto node2 = SearchNodeType();
    node2.get_status() = SearchNodeStatus::OPEN;
    node2.get_parent_state() = 4;
    node2.get_property<0>() = 9.99;

    auto vec = mimir::buffering::Vector<SearchNodeType>();
    vec.push_back(node1);
    vec.push_back(node2);

    auto deserialized_node1 = vec[0];
    auto deserialized_node2 = vec[1];

    EXPECT_EQ(deserialized_node1->get_status(), SearchNodeStatus::NEW);
    EXPECT_EQ(deserialized_node1->get_parent_state(), 2);
    EXPECT_EQ(deserialized_node1->get_property<0>(), 3.14);

    EXPECT_EQ(deserialized_node2->get_status(), SearchNodeStatus::OPEN);
    EXPECT_EQ(deserialized_node2->get_parent_state(), 4);
    EXPECT_EQ(deserialized_node2->get_property<0>(), 9.99);
}
}
