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

class BufferWriter
{
private:
    std::vector<uint8_t> buffer;
    size_t bit;

    void resize_to_fit(size_t num_bits)
    {
        size_t required_bytes = (num_bits + 7) / 8;

        if (buffer.size() < required_bytes)
        {
            buffer.resize(required_bytes);
        }
    }

public:
    BufferWriter() : buffer(), bit(0) {}

    void reset()
    {
        buffer.clear();
        bit = 0;
    }

    template<std::unsigned_integral T>
    void write(T value)
    {
        size_t num_bits = std::bit_width(value);
        resize_to_fit(bit + 6 + num_bits);
        write_uint<T>(buffer.data(), bit, bit + 6, num_bits);
        bit += 6;
        write_uint<T>(buffer.data(), bit, bit + num_bits, value);
        bit += num_bits;
    }

    void write_zero_padding()
    {
        size_t used_bits = bit % 8;
        if (used_bits != 0)  // avoid clearing a completely used final byte.
        {
            buffer.back() &= uint8_t((1 << used_bits) - 1);
        }
    }

    const std::vector<uint8_t>& get_buffer() const { return buffer; }
};

class BufferReader
{
private:
    const uint8_t* buffer;

public:
    BufferReader(const uint8_t* buffer) : buffer(buffer) {}

    template<std::unsigned_integral T>
    std::pair<T, size_t> read(size_t bit) const
    {
        size_t num_bits = read_uint<size_t>(buffer, bit, bit + 6);
        bit += 6;
        T value = read_uint<T>(buffer, bit, bit + num_bits);
        bit += num_bits;
        return std::make_pair(value, bit);
    }
};
}

#endif
