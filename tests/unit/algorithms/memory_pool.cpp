#include <mimir/algorithms/memory_pool.hpp>

#include <gtest/gtest.h>

#include <string>


namespace mimir::tests
{

TEST(MimirTests, AlgorithmsMemoryPoolTest) {
    MemoryPool<std::vector<bool>> pool;
    EXPECT_EQ(pool.get_size(), 0);
    auto object_0 = pool.get_or_allocate();
    EXPECT_EQ(pool.get_size(), 1);
    EXPECT_EQ(pool.get_num_free(), 0);
    {
        auto object_1 = pool.get_or_allocate();
        EXPECT_EQ(pool.get_size(), 2);
        EXPECT_EQ(pool.get_num_free(), 0);
        // destructor of object_1 is called
    }
    // pool size remains 2
    EXPECT_EQ(pool.get_size(), 2);
    EXPECT_EQ(pool.get_num_free(), 1);

    // reuse element
    auto object_1 = pool.get_or_allocate();
    EXPECT_EQ(pool.get_size(), 2);
    EXPECT_EQ(pool.get_num_free(), 0);
}



}
