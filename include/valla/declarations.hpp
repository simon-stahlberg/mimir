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

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#include <cassert>
#include <functional>
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

using Index = uint32_t;  ///< Enough space for 4,294,967,295 indices

/**
 * Integer slot
 */

struct Slot
{
    Index i1;
    Index i2;

    constexpr Slot(Index i1, Index i2) : i1(i1), i2(i2) {}

    constexpr friend bool operator==(const Slot& lhs, const Slot& rhs) { return lhs.i1 == rhs.i1 && lhs.i2 == rhs.i2; }
};

static_assert(alignof(Slot) == 4);

static constexpr const Slot EMPTY_ROOT_SLOT = Slot(Index(0), Index(0));  ///< represents the empty state.

using IndexList = std::vector<Index>;

/**
 * Printing
 */

inline std::ostream& operator<<(std::ostream& out, const IndexList& state)
{
    out << "[";
    for (const auto x : state)
    {
        out << x << ", ";
    }
    out << "]";

    return out;
}

/**
 * Hashing
 */

inline uint64_t cantor_pair(uint64_t a, uint64_t b) { return (((a + b) * (a + b + 1)) >> 1) + b; }

struct SlotHash
{
    size_t operator()(Slot el) const { return cantor_pair(el.i1, el.i2); }
};

}

#endif