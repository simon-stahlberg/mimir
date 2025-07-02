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

// Code
// https://github.com/kampersanda/bonsais
// https://github.com/simongog/sdsl-lite/tree/master
// https://github.com/tudocomp/tudocomp
// https://github.com/Poyias/mBonsai

// Papers:
// https://arxiv.org/pdf/1906.06015
// https://citeseerx.ist.psu.edu/document?repid=rep1&type=pdf&doi=b7620871f53d759a15cf9e820584b89f984f74c1
// https://link.springer.com/article/10.1007/s00453-022-00996-y

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

using RawSlot = uint64_t;

/// @brief Pack two uint32_t into a uint64_t.
inline RawSlot make_slot(Index lhs, Index rhs) { return RawSlot(lhs) << 32 | rhs; }

inline Index first(RawSlot slot) { return Index(slot >> 32); }

inline Index second(RawSlot slot) { return Index(slot) & (Index(-1)); }

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

struct SlotHash
{
    size_t operator()(Slot el) const { return fmix64((uint64_t(el.i1) << 32) | el.i2); }
};

struct RawSlotHash
{
    size_t operator()(RawSlot el) const { return fmix64(el); }
};

}

#endif