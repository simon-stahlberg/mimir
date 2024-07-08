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
    auto index_grouped_vector = IndexGroupedVector<ElementType>::create(vec, 4, group_boundary_checker, group_index_retriever);

    EXPECT_EQ(index_grouped_vector.get_groups_begin(), (std::vector<size_t> { 0, 2, 2, 4, 4 }));
}

}
