#include <mimir/search/states.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchStatesBitsetGroundedTest) {
    // Build a state.
    auto builder = Builder<StateDispatcher<BitsetStateTag, GroundedTag>>();
    builder.set_id(5);
    builder.finish();
    EXPECT_NE(builder.get_buffer_pointer(), nullptr);
    EXPECT_EQ(builder.get_size(), 48);

    // View the data generated in the builder.
    auto view = View<StateDispatcher<BitsetStateTag, GroundedTag>>(builder.get_buffer_pointer());
    EXPECT_EQ(view.get_id(), 5);
    EXPECT_EQ(view.get_size(), 48);
}

}
