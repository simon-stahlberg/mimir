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

#ifndef VALLA_INCLUDE_EQUAL_TO_HPP_
#define VALLA_INCLUDE_EQUAL_TO_HPP_

#include "valla/uint64tcoder.hpp"

#include <cstdint>

namespace valla
{

template<typename T>
struct EqualTo
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }

    template<IsUint64tCodable U = T>
    bool operator()(uint64_t lhs, uint64_t rhs) const
    {
        return lhs == rhs;
    }
};

}

#endif