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

#ifndef VALLA_INCLUDE_UINT64TCODER_HPP_
#define VALLA_INCLUDE_UINT64TCODER_HPP_

#include "valla/slot.hpp"

#include <concepts>
#include <cstdint>

namespace valla
{

template<typename T>
struct Uint64tCoder
{
};

template<std::unsigned_integral I>
struct Uint64tCoder<Slot<I>>
{
    constexpr static uint8_t bit_width(const Slot<I>& el) { return std::max(1, 2 * std::max(std::bit_width(el.i1), std::bit_width(el.i2))); }

    constexpr static uint64_t to_uint64_t(const Slot<I>& el, uint8_t bit_width)
    {
        uint8_t half = bit_width / 2;
        return (uint64_t(el.i1) << half) | el.i2;
    }

    constexpr static Slot<I> from_uint64_t(uint64_t packed, uint8_t bit_width)
    {
        uint8_t half = bit_width / 2;
        uint64_t mask = (uint64_t(1) << half) - 1;
        I i2 = static_cast<I>(packed & mask);
        I i1 = static_cast<I>(packed >> half);
        return Slot(i1, i2);
    }
};

template<std::unsigned_integral T>
struct Uint64tCoder<T>
{
    constexpr static uint8_t bit_width(const T& el) { return std::max(1, std::bit_width(el)); }

    constexpr static uint64_t to_uint64_t(const T& el, uint8_t) { return static_cast<uint64_t>(el); }

    constexpr static T from_uint64_t(uint64_t packed, uint8_t) { return static_cast<T>(packed); }
};

/// `IsUint64tCodable` is a type that can be encoded from and to an uint64_t.
template<typename T>
concept IsUint64tCodable = requires(T a, uint64_t p, uint8_t b) {
    requires std::is_standard_layout_v<T>;
    requires(sizeof(T) <= sizeof(uint64_t));

    { Uint64tCoder<T>::bit_width(a) } -> std::same_as<uint8_t>;
    { Uint64tCoder<T>::to_uint64_t(a, b) } -> std::same_as<uint64_t>;
    { Uint64tCoder<T>::from_uint64_t(p, b) } -> std::same_as<T>;
};

static_assert(IsUint64tCodable<uint16_t>);
static_assert(IsUint64tCodable<uint32_t>);
static_assert(IsUint64tCodable<uint64_t>);

static_assert(IsUint64tCodable<Slot<uint16_t>>);
static_assert(IsUint64tCodable<Slot<uint32_t>>);
static_assert(!IsUint64tCodable<Slot<uint64_t>>);

}

#endif