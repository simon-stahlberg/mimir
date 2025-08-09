/*
 * Copyright (C) 2025 Dominik Drexler
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

#ifndef VALLA_INCLUDE_UTILS_HPP_
#define VALLA_INCLUDE_UTILS_HPP_

#include <concepts>
#include <cstddef>

namespace valla
{

template<typename Container>
constexpr inline bool is_within_bounds(const Container& container, size_t index)
{
    return index < container.size();
}

template<std::unsigned_integral T>
constexpr inline bool is_power_of_two(T n)
{
    return n != 0 && (n & (n - 1)) == 0;
}

}

#endif