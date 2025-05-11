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

TEST(MimirTests, SearchOpenListsPriorityQueueTest)
{
    auto priority_queue = PriorityQueue<double, int>();
    priority_queue.insert(1.1, 0);
    priority_queue.insert(2.2, 1);
    auto element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 0);
    element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 1);
}

TEST(MimirTests, SearchOpenListsPriorityQueue2Test)
{
    auto priority_queue = PriorityQueue<std::tuple<double, double>, int>();
    priority_queue.insert(std::make_tuple(1.1, 5.1), 0);
    priority_queue.insert(std::make_tuple(2.2, 3.1), 1);
    priority_queue.insert(std::make_tuple(1.1, 4.1), 2);
    priority_queue.insert(std::make_tuple(2.2, 2.1), 3);
    auto element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 2);
    element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 0);
    element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 3);
    element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 1);
}

}
