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

#ifndef VALLA_INCLUDE_OSTREAM_HPP_
#define VALLA_INCLUDE_OSTREAM_HPP_

#include <cstdint>
#include <ostream>
#include <vector>

namespace valla
{

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

}

#endif