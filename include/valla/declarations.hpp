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

using Slot = uint64_t;  ///< A slot is made up of two indices.

static constexpr const Slot EMPTY_ROOT_SLOT = Slot(0);  ///< represents the empty state.

/// @brief Pack two uint32_t into a uint64_t.
inline Slot make_slot(Index lhs, Index rhs) { return Slot(lhs) << 32 | rhs; }

/// @brief Unpack two uint32_t from a uint64_t.
inline std::pair<Index, Index> read_slot(Slot slot) { return { Index(slot >> 32), slot & (Index(-1)) }; }

inline Index first(Slot slot) { return Index(slot >> 32); }

inline Index second(Slot slot) { return Index(slot) & (Index(-1)); }

inline Index read_pos(Slot slot, size_t pos)
{
    assert(pos < 2);
    return Index((slot >> ((1 - pos) * 32)));
}

using IndexList = std::vector<Index>;
using DoubleList = std::vector<double>;

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

template<typename T>
inline void hash_combine(size_t& seed, const T& value)
{
    seed ^= std::hash<std::decay_t<T>> {}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline uint64_t cantor_pair(uint64_t a, uint64_t b) { return (((a + b) * (a + b + 1)) >> 1) + b; }

/* Source: https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp */
inline uint64_t fmix64(uint64_t k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccd;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53;
    k ^= k >> 33;

    return k;
}

template<typename S>
struct SlotHash
{
    size_t operator()(S el) const { return el; }
};

template<>
struct SlotHash<Slot>
{
    size_t operator()(Slot el) const { return cantor_pair(first(el), second(el)); }
};

constexpr Slot SlotSentinel = std::numeric_limits<Slot>::max();

}

#endif