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
    // Instantiate grounded version
    auto priority_queue = PriorityQueue<int>();
    priority_queue.insert(1.1, 42);
    priority_queue.insert(2.2, 5);
    auto element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 42);
}

}
