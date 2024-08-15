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

#ifndef MIMIR_COMMON_EQUAL_TO_HPP_
#define MIMIR_COMMON_EQUAL_TO_HPP_

#include "mimir/common/concepts.hpp"

#include <functional>
#include <span>

namespace mimir
{

/// @brief Compare two sized ranges for equality.
/// @tparam R1, R2
/// @param lhs
/// @param rhs
/// @return true if ranges are equal, false otherwise
template<std::ranges::sized_range R1, std::ranges::sized_range R2>
bool operator==(const R1& lhs, const R2& rhs)
{
    return std::ranges::size(lhs) == std::ranges::size(rhs) && std::ranges::equal(lhs, rhs);
}

}

#endif
