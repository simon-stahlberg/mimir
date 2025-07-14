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

#include <absl/container/flat_hash_set.h>
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
using IndexList = std::vector<Index>;

using DoubleList = std::vector<double>;

/**
 * Slot
 */

template<typename T>
struct Slot
{
    T i1;
    T i2;

    constexpr Slot() : i1(0), i2(0) {}
    constexpr Slot(T i1, T i2) : i1(i1), i2(i2) {}

    constexpr friend bool operator==(const Slot& lhs, const Slot& rhs) { return lhs.i1 == rhs.i1 && lhs.i2 == rhs.i2; }

    friend std::ostream& operator<<(std::ostream& os, const Slot& slot)
    {
        os << "<" << slot.i1 << ", " << slot.i2 << ">";
        return os;
    }
};

using IndexSlot = Slot<Index>;
using DoubleSlot = Slot<double>;

static constexpr const Slot EMPTY_ROOT_SLOT = IndexSlot();  ///< represents the empty state.

/**
 * Iterator
 */

struct Entry
{
    Index m_index;
    Index m_size;

    Entry(Index index, Index size) : m_index(index), m_size(size) {}
};

inline void copy_object(const std::vector<Entry>& src, std::vector<Entry>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

inline void copy_object(const std::vector<double>& src, std::vector<double>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

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

inline std::ostream& operator<<(std::ostream& out, const std::vector<uint8_t>& vec)
{
    out << "[";
    for (const auto x : vec)
    {
        out << static_cast<uint32_t>(x) << ", ";
    }
    out << "]";

    return out;
}

/**
 * Hashing
 */

template<typename T>
struct SlotHash
{
    size_t operator()(Slot<T> el) const { return absl::HashOf(el.i1, el.i2); }
};

// Instead of additionally storing the size in the unstable_to_stable mapping,
// we reference to stable_to_unstable to access this piece of information.
template<typename T>
struct IndexReferencedSlotHash
{
    const std::vector<Slot<T>>& stable_to_unstable;

    IndexReferencedSlotHash(const std::vector<Slot<T>>& stable_to_unstable) : stable_to_unstable(stable_to_unstable) {}

    size_t operator()(Index el) const
    {
        assert(el < stable_to_unstable.size());
        return SlotHash<T> {}(stable_to_unstable[el]);
    }
};

template<typename T>
struct IndexReferencedSlotEqualTo
{
    const std::vector<Slot<T>>& stable_to_unstable;

    IndexReferencedSlotEqualTo(const std::vector<Slot<T>>& stable_to_unstable) : stable_to_unstable(stable_to_unstable) {}

    size_t operator()(Index lhs, Index rhs) const
    {
        assert(lhs < stable_to_unstable.size());
        assert(rhs < stable_to_unstable.size());
        return stable_to_unstable[lhs] == stable_to_unstable[rhs];
    }
};

}

#endif