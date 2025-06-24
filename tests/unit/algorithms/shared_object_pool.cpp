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

// Include the copy before the pool.
#include "shared_object_pool.hpp"
//
#include "mimir/algorithms/shared_object_pool.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace mimir::tests
{

TEST(MimirTests, AlgorithmsSharedObjectPoolTest)
{
    SharedObjectPool<std::vector<int>> pool;
    EXPECT_EQ(pool.get_size(), 0);
    auto object_0 = pool.get_or_allocate();
    EXPECT_EQ(pool.get_size(), 1);
    EXPECT_EQ(pool.get_num_free(), 0);

    // Test shared_ownership
    EXPECT_EQ(object_0.ref_count(), 1);
    {
        // Test copy assignment
        auto object_0_1 = object_0;
        EXPECT_EQ(object_0_1.ref_count(), 2);

        auto object_0_2 = SharedObjectPoolPtr<std::vector<int>>();
        EXPECT_EQ(object_0.ref_count(), 2);
        EXPECT_EQ(object_0_1.ref_count(), 2);

        // Test move assignment
        object_0_2 = std::move(object_0_1);
        EXPECT_EQ(object_0.ref_count(), 2);
        EXPECT_EQ(object_0_2.ref_count(), 2);

        // Test copy constructor
        auto object_0_3 = SharedObjectPoolPtr<std::vector<int>>(object_0);
        EXPECT_EQ(object_0.ref_count(), 3);
        EXPECT_EQ(object_0_3.ref_count(), 3);

        // Test move constructor
        auto object_0_4 = object_0;
        auto object_0_5 = SharedObjectPoolPtr<std::vector<int>>(std::move(object_0_4));
        EXPECT_EQ(object_0.ref_count(), 4);
        EXPECT_EQ(object_0_5.ref_count(), 4);
    }
    EXPECT_EQ(object_0.ref_count(), 1);

    // Test clone
    auto object_0_clone = object_0.clone();
    EXPECT_EQ(object_0.ref_count(), 1);
    EXPECT_EQ(object_0_clone.ref_count(), 1);

    {
        auto object_1 = pool.get_or_allocate();
        object_1->push_back(42);
        EXPECT_EQ(pool.get_size(), 3);
        EXPECT_EQ(pool.get_num_free(), 0);
        EXPECT_EQ(object_1->size(), 1);
        EXPECT_EQ(object_1->back(), 42);
        // destructor of object_1 is called
    }
    // pool size remains 2
    EXPECT_EQ(pool.get_size(), 3);
    EXPECT_EQ(pool.get_num_free(), 1);

    // reuse element, it still contains the data.
    auto object_1 = pool.get_or_allocate();
    EXPECT_EQ(pool.get_size(), 3);
    EXPECT_EQ(pool.get_num_free(), 0);
    EXPECT_EQ(object_1->size(), 1);
    EXPECT_EQ(object_1->back(), 42);
}

TEST(MimirTests, AlgorithmsSharedObjectPoolVectorTest)
{
    SharedObjectPool<std::vector<int>> pool;

    auto vec = std::vector<SharedObjectPoolPtr<std::vector<int>>> {};

    vec.push_back(pool.get_or_allocate());
    vec.emplace_back(pool.get_or_allocate());
    auto object_0 = pool.get_or_allocate();
    vec.push_back(object_0);
    auto object_1 = pool.get_or_allocate();
    vec.emplace_back(object_1);
    auto object_2 = pool.get_or_allocate();
    vec.push_back(std::move(object_2));
    auto object_3 = pool.get_or_allocate();
    vec.emplace_back(std::move(object_3));
}

}
