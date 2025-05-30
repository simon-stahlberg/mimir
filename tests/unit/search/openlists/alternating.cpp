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

#include "mimir/search/openlists.hpp"

#include <gtest/gtest.h>

using namespace mimir::search;

namespace mimir::tests
{

TEST(MimirTests, SearchOpenListsAlternatingTest)
{
    using Queue0 = PriorityQueue<double, int>;
    using Queue1 = PriorityQueue<int, int>;

    auto priority_queue_0 = Queue0();
    auto priority_queue_1 = Queue1();
    auto alternating_queue = AlternatingOpenList<Queue0, Queue1>(priority_queue_0, priority_queue_1, std::array<size_t, 2> { 2, 1 });

    priority_queue_0.insert(1.1, 0);
    priority_queue_0.insert(4.4, 1);
    priority_queue_0.insert(3.3, 2);
    priority_queue_0.insert(2.2, 3);
    priority_queue_0.insert(5.5, 4);
    priority_queue_1.insert(1, 5);
    priority_queue_1.insert(4, 6);
    priority_queue_1.insert(3, 7);
    priority_queue_1.insert(2, 8);
    priority_queue_1.insert(5, 9);

    EXPECT_EQ(alternating_queue.size(), 10);
    EXPECT_FALSE(alternating_queue.empty());
    auto element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 0);

    EXPECT_EQ(alternating_queue.size(), 9);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 3);

    EXPECT_EQ(alternating_queue.size(), 8);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 5);

    EXPECT_EQ(alternating_queue.size(), 7);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 2);

    EXPECT_EQ(alternating_queue.size(), 6);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 1);

    EXPECT_EQ(alternating_queue.size(), 5);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 8);

    EXPECT_EQ(alternating_queue.size(), 4);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 4);

    EXPECT_EQ(alternating_queue.size(), 3);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 7);

    EXPECT_EQ(alternating_queue.size(), 2);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 6);

    EXPECT_EQ(alternating_queue.size(), 1);
    EXPECT_FALSE(alternating_queue.empty());
    element = alternating_queue.top();
    alternating_queue.pop();
    EXPECT_EQ(element, 9);

    EXPECT_EQ(alternating_queue.size(), 0);
    EXPECT_TRUE(alternating_queue.empty());
}

}
