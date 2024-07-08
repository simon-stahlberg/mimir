#include "mimir/common/grouped_vector.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, CommonIndexGroupedVectorTest)
{
    using ElementType = std::pair<int, int>;
    auto vec = std::vector<ElementType> { { 2, 3 }, { 0, 2 }, { 2, 0 }, { 0, 2 } };
    auto sort_comparator = [](const ElementType& l, const ElementType& r)
    {
        if (l.first == r.first)
        {
            return l.second < r.second;
        }
        return l.first < r.first;
    };
    auto group_comparator = sort_comparator;
    auto group_retriever = [](const ElementType& e) { return static_cast<size_t>(e.first); };
    auto index_grouped_vector = IndexGroupedVector<ElementType>::create(vec, 4, sort_comparator, group_comparator, group_retriever);

    EXPECT_EQ(index_grouped_vector.get_vector(), (std::vector<ElementType> { { 0, 2 }, { 0, 2 }, { 2, 0 }, { 2, 3 } }));
    EXPECT_EQ(index_grouped_vector.get_groups_begin(), (std::vector<size_t> { 0, 2, 2, 4, 4 }));
}

}
