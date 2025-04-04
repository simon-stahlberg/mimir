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

#include "cista/containers/flexible_index_vector.h"

#include "cista/serialization.h"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/ground_action.hpp"

#include <bitset>
#include <gtest/gtest.h>
#include <iostream>

namespace mimir::tests
{

void print_bits(const uint16_t* vec, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        std::bitset<16> bits(vec[i]);  // 16 bits for uint8_t
        std::cout << bits << " ";
    }
    std::cout << std::endl;
}

TEST(CistaTests, CistaFlexibleIndexVectorEmptyTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_index_vector<uint16_t>;

    auto vec = Vector();
    vec.compress();

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(buf.size(), 32);
    EXPECT_EQ(deserialized.size(), 0);
}

TEST(CistaTests, CistaFlexibleIndexVectorSize1Test)
{
    namespace data = cista::offset;

    using Vector = data::flexible_index_vector<uint16_t>;

    auto vec = Vector();
    vec.push_back(5);
    vec.compress();

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(buf.size(), 34);
    EXPECT_EQ(deserialized.size(), 1);
    EXPECT_EQ(deserialized[0], 5);
}

TEST(CistaTests, CistaFlexibleIndexVectorIterateUncompressedTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_index_vector<uint16_t>;

    auto vec = Vector({ 1, 16, 2, 4, 9 });

    // test default vector iterator ;-)
    auto it = vec.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 16);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 9);
    ++it;
    EXPECT_EQ(it, vec.end());

    // test our custom const_iterator
    auto cit = vec.cbegin();
    EXPECT_EQ(*cit, 1);
    ++cit;
    EXPECT_EQ(*cit, 16);
    ++cit;
    EXPECT_EQ(*cit, 2);
    ++cit;
    EXPECT_EQ(*cit, 4);
    ++cit;
    EXPECT_EQ(*cit, 9);
    ++cit;
    EXPECT_EQ(cit, vec.cend());
}

TEST(CistaTests, CistaFlexibleIndexVectorTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_index_vector<uint16_t>;

    auto vec = Vector({ 1, 16, 2, 4, 9 });

    EXPECT_EQ(vec.bit_width(), 16);
    EXPECT_EQ(vec.bit_width_log2(), 4);
    EXPECT_EQ(vec.elements_per_block(), 1);
    EXPECT_EQ(vec.elements_per_block_log2(), 0);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.blocks().size(), 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 16);
    EXPECT_EQ(vec[2], 2);
    EXPECT_EQ(vec[3], 4);
    EXPECT_EQ(vec[4], 9);

    auto vec2 = vec;
    std::sort(vec2.begin(), vec2.end());
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 4);
    EXPECT_EQ(vec2[3], 9);
    EXPECT_EQ(vec2[4], 16);

    vec.compress();

    // print_bits(reinterpret_cast<const uint16_t*>(vec.blocks().data()), vec.blocks().size());

    const auto& const_vec = vec;

    EXPECT_EQ(const_vec.bit_width(), 8);
    EXPECT_EQ(const_vec.bit_width_log2(), 3);
    EXPECT_EQ(const_vec.elements_per_block(), 2);
    EXPECT_EQ(const_vec.elements_per_block_log2(), 1);
    EXPECT_EQ(const_vec.size(), 5);
    EXPECT_EQ(const_vec.blocks().size(), 3);
    EXPECT_EQ(const_vec[0], 1);
    EXPECT_EQ(const_vec[1], 16);
    EXPECT_EQ(const_vec[2], 2);
    EXPECT_EQ(const_vec[3], 4);
    EXPECT_EQ(const_vec[4], 9);  // yields 0

    auto it = const_vec.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 16);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 9);
    ++it;
    EXPECT_EQ(it, const_vec.end());

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(deserialized.bit_width(), 8);
    EXPECT_EQ(deserialized.bit_width_log2(), 3);
    EXPECT_EQ(deserialized.elements_per_block(), 2);
    EXPECT_EQ(deserialized.elements_per_block_log2(), 1);
    EXPECT_EQ(deserialized.size(), 5);
    EXPECT_EQ(deserialized.blocks().size(), 3);
    EXPECT_EQ(deserialized[0], 1);
    EXPECT_EQ(deserialized[1], 16);
    EXPECT_EQ(deserialized[2], 2);
    EXPECT_EQ(deserialized[3], 4);
    EXPECT_EQ(deserialized[4], 9);

    it = const_vec.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 16);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 9);
    ++it;
    EXPECT_EQ(it, const_vec.end());

    vec.clear();
    vec.push_back(4);
    EXPECT_EQ(vec[0], 4);
    EXPECT_EQ(vec.size(), 1);
}

}
