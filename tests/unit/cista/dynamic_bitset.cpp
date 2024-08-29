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

TEST(CistaTests, CistaDynamicBitsetTest)
{
    namespace data = cista::raw;

    using Bitset = data::dynamic_bitset<uint64_t>;

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
    auto deserialized = cista::deserialize<Bitset>(buf.begin().base(), buf.end().base());
}

TEST(CistaTests, CistaDynamicBitsetDefaultConstructorTest)
{
    // Test default constructor bitset
    auto bitset = cista::raw::dynamic_bitset<uint64_t>();
    EXPECT_FALSE(bitset.default_bit_value_);
    EXPECT_EQ(bitset.blocks_.size(), 0);
}

TEST(CistaTests, CistaDynamicBitsetConstructorSizeTest)
{
    // Test default constructor bitset
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(65);
    EXPECT_FALSE(bitset.default_bit_value_);
    EXPECT_EQ(bitset.blocks_.size(), 2);
}

TEST(CistaTests, CistaDynamicBitsetConstructorSizeValueTest)
{
    // Test size constructor
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(10, true);
    EXPECT_TRUE(bitset.default_bit_value_);
    EXPECT_EQ(bitset.blocks_.size(), 1);
    EXPECT_TRUE(bitset.get(100));
}

// TEST(CistaTests, TypesBitsetConstructorCopyTest)
//{
//     // Test size constructor
//     size_t num_bits = 2;
//     auto builder = Builder<Bitset<uint64_t>>(num_bits);
//     builder.set(1);
//     builder.finish();
//
//     // Test Builder
//     auto copy_builder = Builder<Bitset<uint64_t>>(builder);
//     copy_builder.finish();
//     EXPECT_EQ(builder, copy_builder);
// }
//
// TEST(CistaTests, TypesBitsetViewConstructorTest)
//{
//     size_t num_bits = 2;
//     auto builder = Builder<Bitset<uint64_t>>(num_bits);
//     builder.set(1);
//     builder.set(100);
//     builder.finish();
//
//     // Test ConstView
//     auto view = View<Bitset<uint64_t>>(builder.buffer().data());
//     EXPECT_EQ(builder, view);
// }
//
// TEST(CistaTests, TypesBitsetConstViewConstructorTest)
//{
//     size_t num_bits = 2;
//     auto builder = Builder<Bitset<uint64_t>>(num_bits);
//     builder.set(1);
//     builder.set(100);
//     builder.finish();
//
//     // Test ConstView
//     auto const_view = ConstView<Bitset<uint64_t>>(builder.buffer().data());
//     EXPECT_EQ(builder, const_view);
// }
//
///**
// * Conversion constructors
// */
//
// TEST(CistaTests, TypesBitsetConversionTest)
//{
//    // Test size constructor
//    auto builder = Builder<Bitset<uint64_t>>();
//    builder.set(1);
//    builder.finish();
//
//    // Test View
//    auto view = View<Bitset<uint64_t>>(builder.buffer().data());
//    auto copy_builder_view = Builder<Bitset<uint64_t>>(view);
//    EXPECT_EQ(builder, copy_builder_view);
//
//    // Test ConstView
//    auto const_view = ConstView<Bitset<uint64_t>>(builder.buffer().data());
//    auto copy_builder_const_view = Builder<Bitset<uint64_t>>(const_view);
//    EXPECT_EQ(builder, copy_builder_const_view);
//}
//
///**
// * Conversion assignments
// */
//
// TEST(CistaTests, TypesBitsetAssignmentTest)
//{
//    // Test size constructor
//    size_t num_bits = 2;
//    auto builder = Builder<Bitset<uint64_t>>(num_bits);
//    builder.set(1);
//    builder.finish();
//
//    // Test Builder
//    auto tmp_builder = Builder<Bitset<uint64_t>>();
//    tmp_builder = builder;
//    EXPECT_EQ(builder, tmp_builder);
//
//    // Test View
//    auto view = View<Bitset<uint64_t>>(builder.buffer().data());
//    auto tmp_builder_view = Builder<Bitset<uint64_t>>();
//    EXPECT_NE(builder, tmp_builder_view);
//    tmp_builder_view = view;
//    EXPECT_EQ(builder, tmp_builder_view);
//
//    // Test ConstView
//    builder.finish();
//    auto const_view = ConstView<Bitset<uint64_t>>(builder.buffer().data());
//    auto tmp_builder_const_view = Builder<Bitset<uint64_t>>();
//    EXPECT_NE(builder, tmp_builder_const_view);
//    tmp_builder_const_view = const_view;
//    EXPECT_EQ(builder, const_view);
//}
//
///**
// * Operators
// */
//
// TEST(CistaTests, TypesBitsetEqualTest)
//{
//    using BitsetLayout = Bitset<uint64_t>;
//
//    auto builder1 = Builder<BitsetLayout>();
//    builder1.set(42);
//    builder1.finish();
//
//    auto builder2 = Builder<BitsetLayout>();
//    builder2.set(42);
//    builder2.set(64);
//    builder2.finish();
//
//    auto builder3 = Builder<BitsetLayout>();
//    builder3.set(42);
//    builder3.finish();
//
//    // Test Builder
//    EXPECT_TRUE((builder1 == builder3));
//    EXPECT_EQ(std::hash<Builder<BitsetLayout>>()(builder1), std::hash<Builder<BitsetLayout>>()(builder3));
//    EXPECT_FALSE((builder1 == builder2));
//    EXPECT_NE(std::hash<Builder<BitsetLayout>>()(builder1), std::hash<Builder<BitsetLayout>>()(builder2));
//
//    // Test View
//    auto view1 = View<BitsetLayout>(builder1.buffer().data());
//    auto view2 = View<BitsetLayout>(builder2.buffer().data());
//    auto view3 = View<BitsetLayout>(builder3.buffer().data());
//    EXPECT_EQ(view1, view3);
//    EXPECT_EQ(std::hash<View<BitsetLayout>>()(view1), std::hash<View<BitsetLayout>>()(view3));
//    EXPECT_NE(view1, view2);
//    EXPECT_NE(std::hash<View<BitsetLayout>>()(view1), std::hash<View<BitsetLayout>>()(view2));
//
//    // Test ConstView
//    auto const_view1 = ConstView<BitsetLayout>(builder1.buffer().data());
//    auto const_view2 = ConstView<BitsetLayout>(builder2.buffer().data());
//    auto const_view3 = ConstView<BitsetLayout>(builder3.buffer().data());
//    EXPECT_EQ(const_view1, const_view3);
//    EXPECT_EQ(std::hash<ConstView<BitsetLayout>>()(const_view1), std::hash<ConstView<BitsetLayout>>()(const_view3));
//    EXPECT_NE(const_view1, const_view2);
//    EXPECT_NE(std::hash<ConstView<BitsetLayout>>()(const_view1), std::hash<ConstView<BitsetLayout>>()(const_view2));
//
//    // Test Builder and View
//    EXPECT_EQ(builder1, view1);
//    EXPECT_EQ(builder2, view2);
//    EXPECT_EQ(builder3, view3);
//    EXPECT_EQ(builder1, view3);
//    EXPECT_NE(builder1, view2);
//    // Test View and Builder
//    EXPECT_EQ(view1, builder1);
//    EXPECT_EQ(view2, builder2);
//    EXPECT_EQ(view3, builder3);
//    EXPECT_EQ(view1, builder3);
//    EXPECT_NE(view1, builder2);
//
//    // Test Builder and ConstView
//    EXPECT_EQ(builder1, const_view1);
//    EXPECT_EQ(builder2, const_view2);
//    EXPECT_EQ(builder3, const_view3);
//    EXPECT_EQ(builder1, const_view3);
//    EXPECT_NE(builder1, const_view2);
//    // Test ConstView and Builder
//    EXPECT_EQ(const_view1, builder1);
//    EXPECT_EQ(const_view2, builder2);
//    EXPECT_EQ(const_view3, builder3);
//    EXPECT_EQ(const_view1, builder3);
//    EXPECT_NE(const_view1, builder2);
//
//    // Test View and ConstView
//    EXPECT_EQ(view1, const_view1);
//    EXPECT_EQ(view2, const_view2);
//    EXPECT_EQ(view3, const_view3);
//    EXPECT_EQ(view1, const_view3);
//    EXPECT_NE(view1, const_view2);
//    // Test ConstView and View
//    EXPECT_EQ(const_view1, view1);
//    EXPECT_EQ(const_view2, view2);
//    EXPECT_EQ(const_view3, view3);
//    EXPECT_EQ(const_view1, view3);
//    EXPECT_NE(const_view1, view2);
//}
//
// TEST(CistaTests, TypesBitsetAreDisjointTest)
//{
//    auto bitset_1 = Builder<Bitset<uint64_t>>();
//    bitset_1.set(1);
//
//    auto bitset_2 = Builder<Bitset<uint64_t>>();
//    bitset_2.set(2);
//
//    EXPECT_TRUE(bitset_1.are_disjoint(bitset_2));
//
//    bitset_2.set(1);
//
//    EXPECT_FALSE(bitset_1.are_disjoint(bitset_2));
//}
//
// TEST(CistaTests, TypesBitsetIsSupersetTest)
//{
//    auto bitset_1 = Builder<Bitset<uint64_t>>();
//    bitset_1.set(1);
//
//    auto bitset_2 = Builder<Bitset<uint64_t>>();
//    bitset_2.set(2);
//
//    EXPECT_FALSE(bitset_2.is_superseteq(bitset_1));
//
//    bitset_2.set(1);
//
//    EXPECT_TRUE(bitset_2.is_superseteq(bitset_1));
//}
//
///**
// * Modifiers
// */
//
// TEST(CistaTests, TypesBitsetShrinkToFitTest)
//{
//    auto builder = Builder<Bitset<uint64_t>>(250);
//    builder.set(42);
//    EXPECT_EQ(builder.get_blocks().size(), 4);
//    EXPECT_TRUE(builder.get(42));
//
//    builder.shrink_to_fit();
//    EXPECT_EQ(builder.get_blocks().size(), 1);
//}
//
// TEST(CistaTests, TypesBitsetUnsetAllTest)
//{
//    auto builder = Builder<Bitset<uint64_t>>(250);
//    builder.set(42);
//    EXPECT_EQ(builder.get_blocks().size(), 4);
//
//    builder.unset_all();
//    EXPECT_FALSE(builder.get(42));
//    EXPECT_EQ(builder.get_blocks().size(), 1);
//}
//
// TEST(CistaTests, TypesBitsetSetTest)
//{
//    // Is included in TypesBitsetGetTest
//}
//
// TEST(CistaTests, TypesBitsetNotTest)
//{
//    auto bitset = Builder<Bitset<uint64_t>>();
//    bitset.set(1);
//
//    ~bitset;
//
//    EXPECT_TRUE(bitset.get_default_bit_value());
//    EXPECT_TRUE(bitset.get(0));
//    EXPECT_FALSE(bitset.get(1));
//    EXPECT_TRUE(bitset.get(2));
//}
//
// TEST(CistaTests, TypesBitsetOrTest)
//{
//    // B1  B2  R
//    // 0 & 0 = 0
//    // 1 & 0 = 1
//    // 0 & 1 = 1
//    // 1 & 1 = 1
//    auto bitset_1 = Builder<Bitset<uint64_t>>();
//    bitset_1.set(1);
//    bitset_1.set(3);
//
//    auto bitset_2 = Builder<Bitset<uint64_t>>();
//    bitset_2.set(2);
//    bitset_2.set(3);
//
//    const auto result = bitset_1 | bitset_2;
//    EXPECT_EQ(result.get_blocks().size(), 1);
//    EXPECT_FALSE(result.get_default_bit_value());
//    EXPECT_FALSE(result.get(0));
//    EXPECT_TRUE(result.get(1));
//    EXPECT_TRUE(result.get(2));
//    EXPECT_TRUE(result.get(3));
//}
//
// TEST(CistaTests, TypesBitsetOrEqualTest)
//{
//    // B1  B2  R
//    // 0 & 0 = 0
//    // 1 & 0 = 1
//    // 0 & 1 = 1
//    // 1 & 1 = 1
//    auto bitset_1 = Builder<Bitset<uint64_t>>();
//    bitset_1.set(1);
//    bitset_1.set(3);
//
//    auto bitset_2 = Builder<Bitset<uint64_t>>();
//    bitset_2.set(2);
//    bitset_2.set(3);
//
//    bitset_1 |= bitset_2;
//    EXPECT_EQ(bitset_1.get_blocks().size(), 1);
//    EXPECT_FALSE(bitset_1.get_default_bit_value());
//    EXPECT_FALSE(bitset_1.get(0));
//    EXPECT_TRUE(bitset_1.get(1));
//    EXPECT_TRUE(bitset_1.get(2));
//    EXPECT_TRUE(bitset_1.get(3));
//}
//
// TEST(CistaTests, TypesBitsetAndTest)
//{
//    // B1  B2  R
//    // 0 & 0 = 0
//    // 1 & 0 = 0
//    // 0 & 1 = 0
//    // 1 & 1 = 1
//
//    auto bitset_1 = Builder<Bitset<uint64_t>>();
//    bitset_1.set(1);
//    bitset_1.set(3);
//
//    auto bitset_2 = Builder<Bitset<uint64_t>>();
//    bitset_2.set(2);
//    bitset_2.set(3);
//
//    const auto result = bitset_1 & bitset_2;
//    EXPECT_EQ(result.get_blocks().size(), 1);
//    EXPECT_FALSE(result.get_default_bit_value());
//    EXPECT_FALSE(result.get(0));
//    EXPECT_FALSE(result.get(1));
//    EXPECT_FALSE(result.get(2));
//    EXPECT_TRUE(result.get(3));
//}
//
TEST(CistaTests, CistaDynamicBitsetAndEqualTest)
{
    // B1  B2  R
    // 0 & 0 = 0
    // 1 & 0 = 0
    // 0 & 1 = 0
    // 1 & 1 = 1
    auto bitset_1 = cista::raw::dynamic_bitset<uint64_t>();
    bitset_1.set(1);
    bitset_1.set(3);

    auto bitset_2 = cista::raw::dynamic_bitset<uint64_t>();
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

TEST(CistaTests, CistaDynamicBitsetGetTest)
{
    auto bitset = cista::raw::dynamic_bitset<uint64_t>();
    bitset.set(42);

    // Test Builder
    EXPECT_TRUE(bitset.get(42));
    EXPECT_FALSE(bitset.get(64));
}

TEST(CistaTests, CistaDynamicBitsetNextSetBitTest)
{
    size_t num_bits = 10;
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(num_bits);

    // Test next_set_bit
    bitset.set(5);
    EXPECT_EQ(bitset.next_set_bit(0), 5);
}

/**
 * Iterators
 */

TEST(CistaTests, CistaDynamicBitsetIteratorTest)
{
    size_t num_bits = 200;
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(num_bits, false);
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

TEST(CistaTests, CistaDynamicBitsetIterator2Test)
{
    size_t num_bits = 0;
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(num_bits, false);

    auto it = bitset.begin();
    auto end = bitset.end();
    EXPECT_EQ(it, end);
}

TEST(CistaTests, CistaDynamicBitsetIterator3Test)
{
    // Set the last bit within the size of the bitset.
    auto bitset = cista::raw::dynamic_bitset<uint64_t>(63, false);
    bitset.set(63);
    bitset.set(127);
    EXPECT_EQ(bitset.count(), 2);
}

}
