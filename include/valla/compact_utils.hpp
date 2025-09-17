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

#ifndef VALLA_INCLUDE_COMPACT_UTILS_HPP_
#define VALLA_INCLUDE_COMPACT_UTILS_HPP_

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>
#include <utility>

namespace valla
{
template<std::ranges::forward_range Range>
    requires std::unsigned_integral<std::ranges::range_value_t<Range>>
inline constexpr std::pair<size_t, uint8_t> compute_required_size_and_width(Range sequence, size_t capacity) noexcept
{
    assert(capacity > 0);

    auto size = size_t(0);
    auto max_index = std::numeric_limits<std::ranges::range_value_t<Range>>::min();
    for (const auto index : sequence)
    {
        max_index = std::max(max_index, index);
        ++size;
    }
    auto width = 2 * std::bit_width(std::max(capacity - 1, static_cast<size_t>(max_index)));
    return { size, width };
}

}

#endif