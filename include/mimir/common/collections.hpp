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

#ifndef MIMIR_COMMON_COLLECTIONS_HPP_
#define MIMIR_COMMON_COLLECTIONS_HPP_

#include <unordered_set>
#include <vector>

namespace mimir
{

/// @brief Return true iff left vector is a subset of right vector
template<typename T>
bool is_subseteq(const std::vector<T>& left, const std::vector<T>& right)
{
    const auto right_set = std::unordered_set<T>(right.begin(), right.end());
    for (const auto& element : left)
    {
        if (!right_set.count(element))
        {
            return false;
        }
    }
    return true;
}

/// @brief Return true iff left vector is disjoint of right vector
template<typename T>
bool are_disjoint(const std::vector<T>& left, const std::vector<T>& right)
{
    const auto right_set = std::unordered_set<T>(right.begin(), right.end());
    for (const auto& element : left)
    {
        if (right_set.count(element))
        {
            return false;
        }
    }
    return true;
}

/// @brief Return true iff all elements in the vector are unique.
template<typename T>
bool is_all_unique(const std::vector<T>& vec)
{
    auto set = std::unordered_set<T>(vec.begin(), vec.end());
    return vec.size() == set.size();
}

/// @brief Return a set representation
template<typename T>
std::unordered_set<T> to_set(const std::vector<T>& vec)
{
    return std::unordered_set<T>(vec.begin(), vec.end());
}

}

#endif
