/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_BUFFERING_UTILS_HPP_
#define MIMIR_BUFFERING_UTILS_HPP_

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace mimir::buffering
{
template<std::unsigned_integral T>
T read_uint(const uint8_t* buffer, size_t start_bit, size_t end_bit)
{
    assert(start_bit < end_bit);
    const auto num_bits = end_bit - start_bit;
    assert(num_bits < sizeof(T) * 8);

    auto result = T(0);
    for (size_t i = 0; i < num_bits; ++i)
    {
        const auto bit_pos = start_bit + i;
        const auto index = bit_pos / 8;
        const auto offset = bit_pos % 8;
        const bool has_bit = (buffer[index] >> offset) & 1;
        result |= (static_cast<T>(has_bit) << i);
    }

    return result;
}

template<std::unsigned_integral T>
void write_uint(uint8_t* buffer, size_t start_bit, size_t end_bit, T value)
{
    assert(start_bit < end_bit);
    const auto num_bits = end_bit - start_bit;
    assert(num_bits < sizeof(T) * 8);

    for (size_t i = 0; i < num_bits; ++i)
    {
        const auto bit_pos = start_bit + i;
        const auto index = bit_pos / 8;
        const auto offset = bit_pos % 8;

        const bool bit = (value >> i) & 1;
        const auto mask = uint8_t(1) << offset;
        if (bit)
            buffer[index] |= mask;
        else
            buffer[index] &= ~mask;
    }
}

}

#endif
