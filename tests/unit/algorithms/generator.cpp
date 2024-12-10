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

#include "mimir/algorithms/generator.hpp"

#include <gtest/gtest.h>
#include <string>

namespace mimir::tests
{

mimir::generator<int> fibonacci()
{
    int i = 0;
    int j = 1;
    while (true)
    {
        co_yield i = std::exchange(j, i + j);
    }
}

TEST(MimirTests, AlgorithmsGeneratorTest)
{
    auto result = std::vector<int> {};
    for (const auto& f : fibonacci())
    {
        result.push_back(f);
        if (result.size() == 10)
        {
            break;
        }
    }
    EXPECT_EQ(result, (std::vector<int> { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55 }));
}
}