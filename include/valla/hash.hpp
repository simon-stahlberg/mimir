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

#ifndef VALLA_INCLUDE_HASH_HPP_
#define VALLA_INCLUDE_HASH_HPP_

#include "valla/slot.hpp"
#include "valla/uint64tcoder.hpp"

#include <absl/hash/hash.h>
#include <concepts>
#include <utility>

namespace valla
{

template<typename T>
struct Hash
{
    size_t operator()(const T& el) const { return std::hash<T> {}(el); }
};

template<std::unsigned_integral I>
struct Hash<Slot<I>>
{
    size_t operator()(const Slot<I>& el) const { return absl::HashOf(el.i1, el.i2); }

    template<IsUint64tCodable U = Slot<I>>
    bool operator()(uint64_t el) const
    {
        return Hash<uint64_t> {}(el);
    }
};

}

#endif