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

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

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

template<std::unsigned_integral T>
struct VarUint
{
    T value;

    static constexpr size_t num_size_bits = std::bit_width(static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::digits));

    friend bool operator==(VarUint lhs, VarUint rhs) noexcept { return lhs.value == rhs.value; }
    friend bool operator!=(VarUint lhs, VarUint rhs) noexcept { return !(lhs == rhs); }
};

template<std::unsigned_integral T>
struct AllocateByteSizePrefix
{
};

template<std::unsigned_integral T>
struct WriteByteSizePrefix
{
};

struct WriteZeroPadding
{
};

class StreamBufferWriter
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
    StreamBufferWriter() : buffer(), bit(0) {}

    void reset() { bit = 0; }

    template<std::unsigned_integral T>
    StreamBufferWriter& operator<<(AllocateByteSizePrefix<T>)
    {
        assert(bit == 0);  // Ensure prefix.

        resize_to_fit(std::numeric_limits<T>::digits);
        bit += std::numeric_limits<T>::digits;

        return *this;
    }

    template<std::unsigned_integral T>
    StreamBufferWriter& operator<<(WriteByteSizePrefix<T> value)
    {
        write_uint<T>(buffer.data(), 0, std::numeric_limits<T>::digits, bit / 8);

        return *this;
    }

    template<std::unsigned_integral T>
    StreamBufferWriter& operator<<(VarUint<T> element)
    {
        size_t num_bits = std::bit_width(element.value);
        resize_to_fit(bit + VarUint<T>::num_size_bits + num_bits);
        write_uint<T>(buffer.data(), bit, bit + VarUint<T>::num_size_bits, num_bits);
        bit += VarUint<T>::num_size_bits;
        write_uint<T>(buffer.data(), bit, bit + num_bits, element.value);
        bit += num_bits;

        return *this;
    }

    template<std::unsigned_integral T>
    StreamBufferWriter& operator<<(T value)
    {
        resize_to_fit(bit + std::numeric_limits<T>::digits);
        write_uint<T>(buffer.data(), bit, bit + std::numeric_limits<T>::digits, value);
        bit += std::numeric_limits<T>::digits;

        return *this;
    }

    StreamBufferWriter& operator<<(WriteZeroPadding)
    {
        size_t used_bits = bit % 8;
        if (used_bits != 0)  // avoid clearing a completely used final byte.
        {
            buffer.back() &= uint8_t((1 << used_bits) - 1);
        }

        return *this;
    }

    const std::vector<uint8_t>& get_buffer() const { return buffer; }
    const size_t get_bit() const { return bit; }
};

template<std::unsigned_integral T>
struct ReadByteSizePrefix
{
    T size;
};

class StreamBufferReader
{
private:
    const uint8_t* buffer;
    size_t bit;

public:
    StreamBufferReader(const uint8_t* buffer) : buffer(buffer), bit(0) {}

    template<std::unsigned_integral T>
    StreamBufferReader& operator>>(ReadByteSizePrefix<T>& prefix)
    {
        prefix.size = buffering::read_uint<T>(buffer, bit, bit + std::numeric_limits<T>::digits);
        bit += std::numeric_limits<T>::digits;

        return *this;
    }

    template<std::unsigned_integral T>
    StreamBufferReader& operator>>(VarUint<T>& value)
    {
        size_t num_bits = buffering::read_uint<size_t>(buffer, bit, bit + VarUint<T>::num_size_bits);
        bit += VarUint<T>::num_size_bits;
        value = VarUint<T>(buffering::read_uint<T>(buffer, bit, bit + num_bits));
        bit += num_bits;

        return *this;
    }

    template<std::unsigned_integral T>
    StreamBufferReader& operator>>(T& value)
    {
        value = buffering::read_uint<T>(buffer, bit, bit + std::numeric_limits<T>::digits);
        bit += std::numeric_limits<T>::digits;

        return *this;
    }
};
}

#endif
