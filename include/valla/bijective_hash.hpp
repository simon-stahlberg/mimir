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

#ifndef VALLA_INCLUDE_BIJECTIVE_HASH_HPP_
#define VALLA_INCLUDE_BIJECTIVE_HASH_HPP_

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
 * Bijective hash function h : [w] -> [w] where [w] represents the set {0,...,2^w-1}.
 * Source: https://arxiv.org/pdf/1906.06015
 */

template<std::unsigned_integral U>
struct BijectiveHash
{
    // ---- compile-time helpers ----
    static consteval U inv2pow(U a)
    {
        U x = 1;
        for (int i = 0; i < (sizeof(U) == 8 ? 6 : 5); ++i)
            x *= (U { 2 } - a * x);
        return x;
    }

    static inline constexpr U P = 0x9E3779B97F4A7C15ull;

    static inline constexpr U P_INV64 = inv2pow(P);

    static inline constexpr std::array<U, std::numeric_limits<U>::digits + 1> MASKS = []
    {
        std::array<U, std::numeric_limits<U>::digits + 1> a {};
        a[0] = 0;
        constexpr uint8_t D = std::numeric_limits<U>::digits;
        for (int w = 1; w <= D; ++w)
            a[w] = (w == D) ? ~U { 0 } : ((U { 1 } << w) - U { 1 });
        return a;
    }();

    // choose a > floor(z/2); derive from z to keep it valid for all z
    static inline constexpr uint8_t A(uint8_t z) { return static_cast<uint8_t>((z >> 1) + 1); }

    // ---- API ----
    static inline U hash(U x, uint8_t z) noexcept
    {
        assert(z >= 1 && z <= std::numeric_limits<U>::digits);
        const U m = MASKS[z];
        const U pmod = P & m;

        x &= m;
        x = (x * pmod) & m;         // h2
        x = (x ^ (x >> A(z))) & m;  // h1
        return x;
    }

    static inline U invert_hash(U y, uint8_t z) noexcept
    {
        assert(z >= 1 && z <= std::numeric_limits<U>::digits);
        const U m = MASKS[z];
        const U pinv = P_INV64 & m;

        y &= m;
        y = (y ^ (y >> A(z))) & m;  // h1^{-1} == h1
        y = (y * pinv) & m;         // h2^{-1}
        return y;
    }
};

}

#endif