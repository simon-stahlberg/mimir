#include <mimir/buffer/flatter/types/tuple.hpp>
#include <mimir/buffer/flatter/types/uint16.hpp>

#include <gtest/gtest.h>

#include <string>


namespace mimir::tests
{

TEST(MimirTests, BufferFlatterTypesTupleTest) {
    EXPECT_EQ((Layout<TupleTag<Uint16Tag, Uint16Tag, Uint16Tag>>::alignment), 2);
    EXPECT_EQ((Layout<TupleTag<Uint16Tag, Uint16Tag, Uint16Tag>>::header_size), 6);

    auto builder = Builder<TupleTag<Uint16Tag, Uint16Tag, Uint16Tag>>();
    builder.get<0>().set_uint16(5);
    builder.get<1>().set_uint16(6);
    builder.get<2>().set_uint16(7);
    builder.finish();
    EXPECT_EQ(builder.get_size(), 6);

    auto view = View<TupleTag<Uint16Tag, Uint16Tag, Uint16Tag>>(builder.get_data());
    EXPECT_EQ(view.get<0>().get(), 5);
    EXPECT_EQ(view.get<1>().get(), 6);
    EXPECT_EQ(view.get<2>().get(), 7);
}




}
