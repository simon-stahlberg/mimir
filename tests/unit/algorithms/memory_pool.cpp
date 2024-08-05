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

#include "mimir/algorithms/memory_pool.hpp"

#include <gtest/gtest.h>
#include <string>

namespace mimir::tests
{

TEST(MimirTests, AlgorithmsMemoryPoolTest)
{
    MemoryPool<std::vector<int>> pool;
    EXPECT_EQ(pool.get_size(), 0);
    auto object_0 = pool.get_or_allocate();
    EXPECT_EQ(pool.get_size(), 1);
    EXPECT_EQ(pool.get_num_free(), 0);
    {
        auto object_1 = pool.get_or_allocate();
        object_1->push_back(42);
        EXPECT_EQ(pool.get_size(), 2);
        EXPECT_EQ(pool.get_num_free(), 0);
        EXPECT_EQ(object_1->size(), 1);
        EXPECT_EQ(object_1->back(), 42);
        // destructor of object_1 is called
    }
    // pool size remains 2
    EXPECT_EQ(pool.get_size(), 2);
    EXPECT_EQ(pool.get_num_free(), 1);

    // reuse element, it still contains the data.
    auto object_1 = pool.get_or_allocate();
    EXPECT_EQ(pool.get_size(), 2);
    EXPECT_EQ(pool.get_num_free(), 0);
    EXPECT_EQ(object_1->size(), 1);
    EXPECT_EQ(object_1->back(), 42);
}

}
