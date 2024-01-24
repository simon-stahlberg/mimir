#include <mimir/search/states/bitset-grounded.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchStatesBitsetGroundedTest) {
    // Build a state.
    auto builder = Builder<WrappedStateTag<BitsetStateTag, GroundedTag>>();
    builder.set_id(5);
    builder.finish();
    EXPECT_NE(builder.get_buffer().get_data(), nullptr);
    EXPECT_EQ(builder.get_buffer().get_size(), 8);

    // View the data generated in the builder.
    auto view = View<WrappedStateTag<BitsetStateTag, GroundedTag>>(builder.get_buffer().get_data());
    EXPECT_EQ(view.get_id(), 5);
}

}
