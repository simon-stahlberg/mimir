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

#ifndef VALLA_INCLUDE_COMPACT_HASH_HPP_
#define VALLA_INCLUDE_COMPACT_HASH_HPP_

#include "valla/slot.hpp"
#include "valla/uint64tcoder.hpp"

#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <utility>

namespace valla
{

/**
 * Bijective hash function h : [b] -> [b] where [b] represents the set {0,...,2^b-1}.
 */

// Precompute masks for b = 0..64
template<std::unsigned_integral T>
consteval auto make_masks()
{
    constexpr uint8_t Bits = std::numeric_limits<T>::digits;

    std::array<T, Bits + 1> m {};
    m[0] = T { 0 };
    for (int b = 1; b < Bits; ++b)
    {
        m[b] = (T(1) << b) - T(1);
    }
    m[Bits] = ~T { 0 };
    return m;
}

template<std::unsigned_integral T>
inline constexpr auto MASKS = make_masks<T>();

template<std::unsigned_integral T>
constexpr T mask(uint8_t b)
{
    constexpr uint8_t Bits = std::numeric_limits<T>::digits;
    assert(b >= 1 && b <= Bits);
    return MASKS<T>[b];
}

template<std::unsigned_integral T>
constexpr T CONSTANT = 0x9E3779B97F4A7C15ULL;

static_assert((CONSTANT<uint64_t> & 1ULL) == 1ULL, "CONSTANT must be odd");

// constexpr 6-step inverse modulo 2^64
template<std::unsigned_integral T>
constexpr T inv_mod_2pow(T a)
{
    T x = T { 1 };
    x *= T(2) - a * x;
    x *= T(2) - a * x;
    x *= T(2) - a * x;
    x *= T(2) - a * x;
    x *= T(2) - a * x;
    x *= T(2) - a * x;
    return x;
}

template<std::unsigned_integral T>
constexpr T CONSTANT_INV = inv_mod_2pow(CONSTANT<T>);

static_assert((CONSTANT<uint64_t> * CONSTANT_INV<uint64_t>) == 1ULL, "CONSTANT_INV must satisfy CONSTANT*CONSTANT_INV == 1 mod 2^64");

template<std::unsigned_integral T>
consteval auto make_masked_inverses()
{
    constexpr uint8_t Bits = std::numeric_limits<T>::digits;
    std::array<T, Bits + 1> a {};
    a[0] = T { 0 };
    for (int b = 1; b < Bits; ++b)
    {
        const T mb = (T { 1 } << b) - T { 1 };
        a[b] = CONSTANT_INV<T> & mb;
    }
    a[Bits] = CONSTANT_INV<T> & ~T { 0 };
    return a;
}

template<std::unsigned_integral T>
inline constexpr auto MASKS_INV = make_masked_inverses<T>();

template<std::unsigned_integral T>
constexpr T mask_inv(uint8_t b)
{
    constexpr uint8_t Bits = std::numeric_limits<T>::digits;
    assert(b >= 1 && b <= Bits);
    return MASKS_INV<T>[b];
}

template<IsUint64tCodable T>
struct CompactHash
{
    T hash(T x, uint8_t b) const { return (x * CONSTANT<T>) &mask<T>(b); }

    T invert_hash(T h, uint8_t b) const { return (h * mask_inv<T>(b)) & mask<T>(b); };
};

}

#endif