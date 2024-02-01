#include <mimir/buffer/wrappers/bitset.hpp>

#include <gtest/gtest.h>

#include <string>


namespace mimir::tests
{

TEST(MimirTests, BufferWrapperBitsetDefaultConstructorTest) {
    // Test default constructor bitset
    auto bitset = Bitset<uint64_t>();
    EXPECT_FALSE(bitset.get_default_bit_value());
    EXPECT_EQ(bitset.get_data().size(), 0);
    EXPECT_EQ(bitset.next_set_bit(0), bitset.no_position);

    // Test resize
    bitset.set(100);
    EXPECT_EQ(bitset.get_data().size(), 2);
    EXPECT_TRUE(bitset.get(100));
    EXPECT_FALSE(bitset.get(0));
    EXPECT_FALSE(bitset.get(101));

    // Test no resize out of bounds
    EXPECT_FALSE(bitset.get(1000));
    EXPECT_EQ(bitset.get_data().size(), 2);
}


TEST(MimirTests, BufferWrapperBitsetConstructorSizeTest) {
    // Test size constructor
    auto bitset = Bitset<uint64_t>(10);
    EXPECT_FALSE(bitset.get_default_bit_value());
    EXPECT_EQ(bitset.get_data().size(), 1);
    EXPECT_EQ(bitset.next_set_bit(0), bitset.no_position);
}


TEST(MimirTests, BufferWrapperBitsetConstructorSizeAndDefaultBitTest) {
    // Test size constructor
    auto bitset = Bitset<uint64_t>(10, true);
    EXPECT_TRUE(bitset.get_default_bit_value());
    EXPECT_EQ(bitset.get_data().size(), 1);
    EXPECT_EQ(bitset.next_set_bit(0), 0);
}

}
