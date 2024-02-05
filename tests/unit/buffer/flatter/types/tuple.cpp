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
    builder.finish();
    EXPECT_EQ(builder.get_size(), 6);
}




}
