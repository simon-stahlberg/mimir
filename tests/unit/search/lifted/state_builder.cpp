#include <mimir/search/config.hpp>
#include <mimir/search/lifted/state_builder.hpp>
#include <mimir/search/lifted/state_view.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedStateBuilderTest) {
    // Build a state.
    auto builder = Builder<State<LiftedTag>>();
    builder.set_id(5);
    builder.finish();
    EXPECT_NE(builder.get_buffer().get_data(), nullptr);
    EXPECT_EQ(builder.get_buffer().get_size(), 8);

    // View the data generated in the builder.
    auto view = View<State<LiftedTag>>(builder.get_buffer().get_data());
    EXPECT_EQ(view.get_id(), 5);
}

}
