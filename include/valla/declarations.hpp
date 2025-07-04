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
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace valla
{

template<typename Container>
inline bool is_within_bounds(const Container& container, size_t index)
{
    return index < container.size();
}

using Index = uint32_t;  ///< Enough space for 4,294,967,295 indices

/**
 * Integer slot
 */

struct Slot
{
    Index i1;
    Index i2;

    constexpr Slot() : i1(0), i2(0) {}
    constexpr Slot(Index i1, Index i2) : i1(i1), i2(i2) {}

    constexpr friend bool operator==(const Slot& lhs, const Slot& rhs) { return lhs.i1 == rhs.i1 && lhs.i2 == rhs.i2; }

    friend std::ostream& operator<<(std::ostream& os, const Slot& slot)
    {
        os << "<" << slot.i1 << ", " << slot.i2 << ">";
        return os;
    }
};

static_assert(alignof(Slot) == 4);

static constexpr const Slot EMPTY_ROOT_SLOT = Slot(Index(0), Index(0));  ///< represents the empty state.

using IndexList = std::vector<Index>;

/**
 * Printing
 */

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
{
    out << "[";
    for (const auto x : vec)
    {
        out << x << ", ";
    }
    out << "]";

    return out;
}

/**
 * Hashing
 */

struct SlotHash
{
    size_t operator()(Slot el) const { return absl::HashOf(el.i1, el.i2); }
};

// Instead of additionally storing the size in the unstable_to_stable mapping,
// we reference to stable_to_unstable to access this piece of information.
struct IndexReferencedSlotHash
{
    std::reference_wrapper<const std::vector<Slot>> stable_to_unstable;

    IndexReferencedSlotHash(const std::vector<Slot>& stable_to_unstable) : stable_to_unstable(stable_to_unstable) {}

    size_t operator()(Index el) const
    {
        assert(el < stable_to_unstable.get().size());
        return SlotHash {}(stable_to_unstable.get()[el]);
    }
};

struct IndexReferencedSlotEqualTo
{
    std::reference_wrapper<const std::vector<Slot>> stable_to_unstable;

    IndexReferencedSlotEqualTo(const std::vector<Slot>& stable_to_unstable) : stable_to_unstable(stable_to_unstable) {}

    size_t operator()(Index lhs, Index rhs) const
    {
        assert(lhs < stable_to_unstable.get().size());
        assert(rhs < stable_to_unstable.get().size());
        return stable_to_unstable.get()[lhs] == stable_to_unstable.get()[rhs];
    }
};

}

#endif