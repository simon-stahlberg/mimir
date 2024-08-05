/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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
