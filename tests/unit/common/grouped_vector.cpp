#include "mimir/common/grouped_vector.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, CommonIndexGroupedVectorTest)
{
    using ElementType = std::pair<int, int>;
    auto vec = std::vector<ElementType> { { 0, 2 }, { 0, 2 }, { 2, 0 }, { 2, 3 } };
    auto group_boundary_checker = [](const ElementType& l, const ElementType& r)
    {
        if (l.first == r.first)
        {
            return l.second != r.second;
        }
        return l.first != r.first;
    };
    auto group_index_retriever = [](const ElementType& e) { return static_cast<size_t>(e.first); };
    auto index_grouped_vector = IndexGroupedVector<ElementType>::create(vec, group_boundary_checker, group_index_retriever, 4);

    EXPECT_EQ(index_grouped_vector.size(), 4);
    EXPECT_EQ(index_grouped_vector[0].size(), 2);
    EXPECT_EQ(index_grouped_vector[1].size(), 0);
    EXPECT_EQ(index_grouped_vector[2].size(), 2);
    EXPECT_EQ(index_grouped_vector[3].size(), 0);
}

}
