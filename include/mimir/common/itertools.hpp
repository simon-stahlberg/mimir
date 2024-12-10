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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_ITERTOOLS_HPP_
#define MIMIR_COMMON_ITERTOOLS_HPP_

#include "mimir/algorithms/generator.hpp"

#include <iostream>
#include <unordered_set>
#include <vector>

namespace mimir
{

/**
 * Cartesian set.
 */

template<typename T>
mimir::generator<const std::vector<T>&> create_cartesian_product_generator(const std::vector<std::vector<T>>& vectors)
{
    // Store the result to be yielded.
    auto result = std::vector<T>(vectors.size());

    // Handle edge case: if any vector is empty, the Cartesian product is empty.
    for (const auto& vec : vectors)
    {
        if (vec.empty())
            co_return;
    }

    // Initialize the first result.
    for (size_t i = 0; i < vectors.size(); ++i)
    {
        result[i] = vectors[i].front();
    }
    co_yield result;

    // Initialize the current indices
    auto indices = std::vector<size_t>(vectors.size(), 0);

    while (true)
    {
        // increment right most index, apply carry over, update result, and yield it

        // Increment indices in bit-flip style
        for (size_t i = 0; i < vectors.size(); ++i)
        {
            if (++indices[i] < vectors[i].size())
            {
                result[i] = vectors[i][indices[i]];
                break;  // No carry-over; proceed with the next result
            }
            indices[i] = 0;  // Reset current index and carry over to the next
            result[i] = vectors[i][0];

            if (i == vectors.size() - 1)
            {
                co_return;  // Terminate when all indices are reset
            }
        }

        co_yield result;
    }
}

template<typename T>
size_t get_size_cartesian_product(const std::vector<std::vector<T>>& vectors)
{
    auto result = size_t { 1 };
    for (const auto& vec : vectors)
    {
        if (vec.empty())
        {
            return 0;
        }
        result *= vec.size();
    }
    return result;
}

}

#endif
