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

#include "cista/containers/dynamic_bitset.h"

#include "cista/serialization.h"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/ground_action.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{
TEST(CistaTests, CistaDynamicBitsetIteratorTest)
{
    size_t num_bits = 200;
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(num_bits);
    bitset.set(0);
    bitset.set(2);
    bitset.set(4);
    bitset.set(99);
    bitset.set(std::numeric_limits<uint32_t>::max());

    auto it = bitset.begin();
    auto end = bitset.end();
    EXPECT_EQ(*it, 0);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 99);
    ++it;
    EXPECT_EQ(*it, std::numeric_limits<uint32_t>::max());
    ++it;
    EXPECT_EQ(it, end);
}
}
