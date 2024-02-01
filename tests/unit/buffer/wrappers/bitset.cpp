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
}


TEST(MimirTests, BufferWrapperBitsetConstructorSizeTest) {
    // Test size constructor
    size_t num_bits = 10;
    auto bitset = Bitset<uint64_t>(num_bits);
    EXPECT_FALSE(bitset.get_default_bit_value());
    EXPECT_EQ(bitset.get_data().size(), 1);
    for (size_t i = 0; i < num_bits; ++i) EXPECT_FALSE(bitset.get(i));
    EXPECT_FALSE(bitset.get(num_bits));
}


TEST(MimirTests, BufferWrapperBitsetConstructorSizeAndDefaultBitTest) {
    // Test size constructor
    size_t num_bits = 10;
    auto bitset = Bitset<uint64_t>(num_bits, true);
    EXPECT_TRUE(bitset.get_default_bit_value());
    EXPECT_EQ(bitset.get_data().size(), 1);
    for (size_t i = 0; i < num_bits; ++i) EXPECT_TRUE(bitset.get(i));
    EXPECT_TRUE(bitset.get(num_bits));
}


TEST(MimirTests, BufferWrapperBitsetResizeTest) {
    auto bitset = Bitset<uint64_t>();

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


TEST(MimirTests, BufferWrapperBitsetNextSetBitTest) {
    size_t num_bits = 10;
    auto bitset = Bitset<uint64_t>(num_bits);

    // Test next_set_bit
    bitset.set(5);
    EXPECT_EQ(bitset.next_set_bit(0), 5);
}


TEST(MimirTests, BufferWrapperBitsetOrTest) {
    // Test operator|
    // 1. Test with same default_bit_value=false
    size_t num_bits_1 = 4;
    auto bitset_1 = Bitset<uint64_t>(num_bits_1, false);
    bitset_1.set(1);
    bitset_1.set(3);

    size_t num_bits_2 = 4;
    auto bitset_2 = Bitset<uint64_t>(num_bits_2, false);
    bitset_2.set(2);
    bitset_2.set(3);

    const auto bitset_r = bitset_1 | bitset_2;
    EXPECT_EQ(bitset_r.get_data().size(), 1);
    EXPECT_FALSE(bitset_r.get_default_bit_value());
    EXPECT_FALSE(bitset_r.get(0));
    EXPECT_TRUE(bitset_r.get(1));
    EXPECT_TRUE(bitset_r.get(2));
    EXPECT_TRUE(bitset_r.get(3));
    EXPECT_FALSE(bitset_r.get(4));
}


}
