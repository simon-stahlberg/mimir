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

#include "mimir/common/itertools.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, AlgorithmsItertoolsCartesianSetTest)
{
    auto vectors = std::vector<std::vector<int>> { { 1, 2 }, { 2, 3, 4 } };
    auto result = std::vector<std::vector<int>> {};
    for (const auto& combination : create_cartesian_product_generator(vectors))
    {
        result.push_back(combination);
    }
    EXPECT_EQ(result, (std::vector<std::vector<int>> { { 1, 2 }, { 2, 2 }, { 1, 3 }, { 2, 3 }, { 1, 4 }, { 2, 4 } }));
}
}