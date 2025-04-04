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

#include "cista/containers/dual_dynamic_bitset.h"

#include "cista/serialization.h"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/ground_action.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

void print_buffer(const std::vector<unsigned char>& buf)
{
    std::cout << "Serialized buffer (" << buf.size() << " bytes):" << std::endl;
    for (size_t i = 0; i < buf.size(); ++i)
    {
        if (i % 16 == 0)
            std::cout << std::endl;  // new line every 16 bytes
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buf[i]) << " ";
    }
    std::cout << std::dec << std::endl;  // switch back to decimal output
}

TEST(CistaTests, CistaDualDynamicBitsetTest)
{
    namespace data = cista::raw;

    using Bitset = data::dual_dynamic_bitset<uint64_t>;

    auto obj = Bitset(1000);
    for (size_t i = 0; i < 1000; ++i)
    {
        obj.set(i);
    }

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(obj);
    }

    print_buffer(buf);

    // Deserialize.
    [[maybe_unused]] auto deserialized = cista::deserialize<Bitset>(buf.begin().base(), buf.end().base());
}

TEST(CistaTests, CistaDualDynamicBitsetDefaultConstructorTest)
{
    // Test default constructor bitset
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>();
    EXPECT_FALSE(bitset.default_bit_value_);
    EXPECT_EQ(bitset.blocks_.size(), 0);
}

TEST(CistaTests, CistaDualDynamicBitsetConstructorSizeTest)
{
    // Test default constructor bitset
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>(65);
    EXPECT_FALSE(bitset.default_bit_value_);
    EXPECT_EQ(bitset.blocks_.size(), 2);
}

TEST(CistaTests, CistaDualDynamicBitsetConstructorSizeValueTest)
{
    // Test size constructor
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>(10, true);
    EXPECT_TRUE(bitset.default_bit_value_);
    EXPECT_EQ(bitset.blocks_.size(), 1);
    EXPECT_TRUE(bitset.get(100));
}

TEST(CistaTests, CistaDualDynamicBitsetConstructorCopyTest)
{
    // Test size constructor
    size_t num_bits = 2;
    auto builder = cista::raw::dual_dynamic_bitset<uint64_t>(num_bits);
    builder.set(1);

    // Test Builder
    auto copy_builder = cista::raw::dual_dynamic_bitset<uint64_t>(builder);
    EXPECT_EQ(builder, copy_builder);
}

/**
 * Assignments
 */

TEST(CistaTests, CistaDualDynamicBitsetAssignmentTest)
{
    // Test size constructor
    size_t num_bits = 2;
    auto builder = cista::raw::dual_dynamic_bitset<uint64_t>(num_bits);
    builder.set(1);

    // Test Builder
    auto tmp_builder = cista::raw::dual_dynamic_bitset<uint64_t>();
    tmp_builder = builder;
    EXPECT_EQ(builder, tmp_builder);
}

/**
 * Operators
 */

TEST(CistaTests, CistaDualDynamicBitsetEqualTest)
{
    auto builder1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    builder1.set(42);

    auto builder2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    builder2.set(42);
    builder2.set(64);

    auto builder3 = cista::raw::dual_dynamic_bitset<uint64_t>();
    builder3.set(42);

    // Test Builder
    EXPECT_TRUE((builder1 == builder3));
    EXPECT_EQ(loki::Hash<cista::raw::dual_dynamic_bitset<uint64_t>>()(builder1), loki::Hash<cista::raw::dual_dynamic_bitset<uint64_t>>()(builder3));
    EXPECT_FALSE((builder1 == builder2));
    EXPECT_NE(loki::Hash<cista::raw::dual_dynamic_bitset<uint64_t>>()(builder1), loki::Hash<cista::raw::dual_dynamic_bitset<uint64_t>>()(builder2));
}

TEST(CistaTests, CistaDualDynamicBitsetAreDisjointTest)
{
    auto bitset_1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_1.set(1);

    auto bitset_2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_2.set(2);

    EXPECT_TRUE(bitset_1.are_disjoint(bitset_2));

    bitset_2.set(1);

    EXPECT_FALSE(bitset_1.are_disjoint(bitset_2));
}

TEST(CistaTests, CistaDualDynamicBitsetIsSupersetTest)
{
    auto bitset_1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_1.set(1);

    auto bitset_2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_2.set(2);

    EXPECT_FALSE(bitset_2.is_superseteq(bitset_1));

    bitset_2.set(1);

    EXPECT_TRUE(bitset_2.is_superseteq(bitset_1));
}

/**
 * Modifiers
 */

TEST(CistaTests, CistaDualDynamicBitsetShrinkToFitTest)
{
    auto builder = cista::raw::dual_dynamic_bitset<uint64_t>(250);
    builder.set(42);
    EXPECT_EQ(builder.blocks_.size(), 4);
    EXPECT_TRUE(builder.get(42));

    builder.shrink_to_fit();
    EXPECT_EQ(builder.blocks_.size(), 1);
}

TEST(CistaTests, CistaDualDynamicBitsetUnsetAllTest)
{
    auto builder = cista::raw::dual_dynamic_bitset<uint64_t>(250);
    builder.set(42);
    EXPECT_EQ(builder.blocks_.size(), 4);

    builder.unset_all();
    EXPECT_FALSE(builder.get(42));
    EXPECT_EQ(builder.blocks_.size(), 0);
}

TEST(CistaTests, CistaDualDynamicBitsetSetTest)
{
    // Is included in CistaDualDynamicBitsetGetTest
}

TEST(CistaTests, CistaDualDynamicBitsetNotTest)
{
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset.set(1);

    ~bitset;

    EXPECT_TRUE(bitset.default_bit_value_);
    EXPECT_TRUE(bitset.get(0));
    EXPECT_FALSE(bitset.get(1));
    EXPECT_TRUE(bitset.get(2));
}

TEST(CistaTests, CistaDualDynamicBitsetOrTest)
{
    // B1  B2  R
    // 0 & 0 = 0
    // 1 & 0 = 1
    // 0 & 1 = 1
    // 1 & 1 = 1
    auto bitset_1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_1.set(1);
    bitset_1.set(3);

    auto bitset_2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_2.set(2);
    bitset_2.set(3);

    const auto result = bitset_1 | bitset_2;
    EXPECT_EQ(result.blocks_.size(), 1);
    EXPECT_FALSE(result.default_bit_value_);
    EXPECT_FALSE(result.get(0));
    EXPECT_TRUE(result.get(1));
    EXPECT_TRUE(result.get(2));
    EXPECT_TRUE(result.get(3));
}

TEST(CistaTests, CistaDualDynamicBitsetOrEqualTest)
{
    // B1  B2  R
    // 0 & 0 = 0
    // 1 & 0 = 1
    // 0 & 1 = 1
    // 1 & 1 = 1
    auto bitset_1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_1.set(1);
    bitset_1.set(3);

    auto bitset_2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_2.set(2);
    bitset_2.set(3);

    bitset_1 |= bitset_2;
    EXPECT_EQ(bitset_1.blocks_.size(), 1);
    EXPECT_FALSE(bitset_1.default_bit_value_);
    EXPECT_FALSE(bitset_1.get(0));
    EXPECT_TRUE(bitset_1.get(1));
    EXPECT_TRUE(bitset_1.get(2));
    EXPECT_TRUE(bitset_1.get(3));
}

TEST(CistaTests, CistaDualDynamicBitsetAndTest)
{
    // B1  B2  R
    // 0 & 0 = 0
    // 1 & 0 = 0
    // 0 & 1 = 0
    // 1 & 1 = 1

    auto bitset_1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_1.set(1);
    bitset_1.set(3);

    auto bitset_2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_2.set(2);
    bitset_2.set(3);

    const auto result = bitset_1 & bitset_2;
    EXPECT_EQ(bitset_1.blocks_.size(), 1);
    EXPECT_FALSE(bitset_1.default_bit_value_);
    EXPECT_FALSE(result.get(0));
    EXPECT_FALSE(result.get(1));
    EXPECT_FALSE(result.get(2));
    EXPECT_TRUE(result.get(3));
}

TEST(CistaTests, CistaDualDynamicBitsetAndEqualTest)
{
    // B1  B2  R
    // 0 & 0 = 0
    // 1 & 0 = 0
    // 0 & 1 = 0
    // 1 & 1 = 1
    auto bitset_1 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_1.set(1);
    bitset_1.set(3);

    auto bitset_2 = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset_2.set(2);
    bitset_2.set(3);

    bitset_1 &= bitset_2;
    EXPECT_EQ(bitset_1.blocks_.size(), 1);
    EXPECT_FALSE(bitset_1.default_bit_value_);
    EXPECT_FALSE(bitset_1.get(0));
    EXPECT_FALSE(bitset_1.get(1));
    EXPECT_FALSE(bitset_1.get(2));
    EXPECT_TRUE(bitset_1.get(3));
}

/**
 * Lookup
 */

TEST(CistaTests, CistaDualDynamicBitsetGetTest)
{
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>();
    bitset.set(42);

    // Test Builder
    EXPECT_TRUE(bitset.get(42));
    EXPECT_FALSE(bitset.get(64));
}

TEST(CistaTests, CistaDualDynamicBitsetNextSetBitTest)
{
    size_t num_bits = 10;
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>(num_bits);

    // Test next_set_bit
    bitset.set(5);
    EXPECT_EQ(bitset.next_set_bit(0), 5);
}

/**
 * Iterators
 */

TEST(CistaTests, CistaDualDynamicBitsetIteratorTest)
{
    size_t num_bits = 200;
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>(num_bits, false);
    bitset.set(0);
    bitset.set(2);
    bitset.set(4);
    bitset.set(99);

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
    EXPECT_EQ(it, end);
}

TEST(CistaTests, CistaDualDynamicBitsetIterator2Test)
{
    size_t num_bits = 0;
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>(num_bits, false);

    auto it = bitset.begin();
    auto end = bitset.end();
    EXPECT_EQ(it, end);
}

TEST(CistaTests, CistaDualDynamicBitsetIterator3Test)
{
    // Set the last bit within the size of the bitset.
    auto bitset = cista::raw::dual_dynamic_bitset<uint64_t>(63, false);
    bitset.set(63);
    bitset.set(127);
    EXPECT_EQ(bitset.count(), 2);
}

}
