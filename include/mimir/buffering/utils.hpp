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
#include <tuple>

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

template<std::unsigned_integral... Ts>
struct UintWriter
{
private:
    uint8_t* buffer;

public:
    UintWriter(uint8_t* buffer) : buffer(buffer) {}

    void write(Ts... values)
    {
        size_t bit = 0;
        ((
             [&]
             {
                 using T = decltype(values);
                 size_t num_bits = std::bit_width(values);
                 write_uint<T>(buffer, bit, bit + 6, num_bits);
                 bit += 6;
                 write_uint<T>(buffer, bit, bit + num_bits, values);
                 bit += num_bits;
             }()),
         ...);

        // Set trailing bits in the last uint8_t to 0.
        size_t used_bytes = (bit + 7) / 8;
        size_t trailing_bits = 8 - (bit % 8);
        if (trailing_bits != 8)
        {
            buffer[used_bytes - 1] &= uint8_t(-1) << trailing_bits;
        }
    }
};

template<std::unsigned_integral... Ts>
struct UintReader
{
private:
    const uint8_t* buffer;

public:
    UintReader(const uint8_t* buffer) : buffer(buffer) {}

    std::tuple<Ts...> read() const
    {
        std::tuple<Ts...> result;
        size_t bit = 0;

        std::apply(
            [this, &bit](auto&&... fields)
            {
                (
                    [&](auto&& field)
                    {
                        using T = std::remove_cvref_t<decltype(field)>;
                        size_t num_bits = read_uint<size_t>(buffer, bit, bit + 6);
                        bit += 6;
                        field = read_uint<T>(buffer, bit, bit + num_bits);
                        bit += num_bits;
                    }(fields),
                    ...);
            },
            result);

        return result;
    }

    size_t buffer_size() const
    {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>)
        {
            size_t bit = 0;
            ((
                 [&]
                 {
                     size_t num_bits = read_uint<size_t>(buffer, bit, bit + 6);
                     bit += 6 + num_bits;
                 }(),
                 static_cast<void>(Is)),
             ...);

            return (bit + 7) / 8;
        }(std::make_index_sequence<sizeof...(Ts)> {});
    }
};
}

#endif
