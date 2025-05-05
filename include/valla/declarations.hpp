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

#ifndef VALLA_INCLUDE_DECLARATIONS_HPP_
#define VALLA_INCLUDE_DECLARATIONS_HPP_

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace valla
{
// Observe: there is no sentinel value requires in constrast to several other tree compression data structures.

using Index = uint32_t;  ///< Enough space for 4,294,967,295 indices
using Slot = uint64_t;   ///< A slot is made up of two indices.

static constexpr const Slot EMPTY_ROOT_SLOT = Slot(0);  ///< represents the empty state.

/// @brief Pack two uint32_t into a uint64_t.
inline Slot make_slot(Index lhs, Index rhs) { return Slot(lhs) << 32 | rhs; }

/// @brief Unpack two uint32_t from a uint64_t.
inline std::pair<Index, Index> read_slot(Slot slot) { return { Index(slot >> 32), slot & (Index(-1)) }; }

inline Index read_pos(Slot slot, size_t pos)
{
    assert(pos < 2);
    return Index((slot >> ((1 - pos) * 32)));
}

using State = std::vector<Index>;
using RootIndices = std::vector<Index>;

}

#endif