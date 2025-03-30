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

#include <boost/container/small_vector.hpp>
#include <loki/loki.hpp>
#include <unordered_set>
#include <vector>

namespace mimir
{

/// @brief Check whether all elements in the vector are unique.
/// @tparam T the vector value type.
/// @param vec the vector.
/// @return true iff all element in the vector are unique, i.e., do not occur more than once, and false otherwise.
template<typename T, typename Hash = loki::Hash<T>>
bool is_all_unique(const std::vector<T>& vec)
{
    auto set = std::unordered_set<T, Hash>(vec.begin(), vec.end());
    return vec.size() == set.size();
}

/// @brief Uniquify elements in a vector of elements.
template<typename T, typename Hash = loki::Hash<T>>
extern std::vector<T> uniquify_elements(const std::vector<T>& vec)
{
    std::unordered_set<T, Hash> set(vec.begin(), vec.end());
    return std::vector<T>(set.begin(), set.end());
}

template<typename Container>
inline bool is_within_bounds(const Container& container, size_t index)
{
    return index < container.size();
}

}

#endif
