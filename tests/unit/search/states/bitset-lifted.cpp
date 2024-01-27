#include <mimir/search/states/bitset-lifted.hpp>
#include <mimir/buffer/byte_stream_utils.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchStatesBitsetLiftedTest) {
    // Build a state.
    auto builder = Builder<StateDispatcher<BitsetStateTag, LiftedTag>>();
    builder.set_id(5);
    builder.finish();
    EXPECT_NE(builder.get_buffer_pointer(), nullptr);
    EXPECT_EQ(builder.get_size(), 20);

    // View the data generated in the builder.
    auto view = View<StateDispatcher<BitsetStateTag, LiftedTag>>(builder.get_buffer_pointer());
    EXPECT_EQ(view.get_id(), 5);
    EXPECT_EQ(view.get_size(), 20);
}

}
