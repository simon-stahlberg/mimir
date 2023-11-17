/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../../include/mimir/algorithms/murmurhash3.hpp"
#include "../../include/mimir/formalism/bitset.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

namespace mimir::formalism
{
    std::size_t get_lsb_position(std::size_t n)
    {
        assert(n != 0);
        const std::size_t v = n & (-n);
        return static_cast<std::size_t>(log2(v));
    }

    void Bitset::resize_to_fit(const Bitset& other)
    {
        if (data.size() < other.data.size())
        {
            data.resize(other.data.size(), default_bit_value ? block_ones : block_zeroes);
        }
    }

    Bitset::Bitset(std::size_t size) : data(size / (sizeof(std::size_t) * 8) + 1, block_zeroes) {}

    Bitset::Bitset(std::size_t size, bool default_bit_value) :
        data(size / (sizeof(std::size_t) * 8) + 1, default_bit_value ? block_ones : block_zeroes),
        default_bit_value(default_bit_value)
    {
    }

    Bitset::Bitset(const Bitset& other) : data(other.data), default_bit_value(other.default_bit_value) {}

    Bitset::Bitset(Bitset&& other) noexcept : data(std::move(other.data)), default_bit_value(other.default_bit_value) {}

    Bitset& Bitset::operator=(const Bitset& other)
    {
        if (this != &other)
        {
            data = other.data;
            default_bit_value = other.default_bit_value;
        }

        return *this;
    }

    // Move assignment operator
    Bitset& Bitset::operator=(Bitset&& other) noexcept
    {
        if (this != &other)
        {
            data = std::move(other.data);
            default_bit_value = other.default_bit_value;
        }

        return *this;
    }

    void Bitset::set(std::size_t position)
    {
        const std::size_t index = position / block_size;   // Find the index in the vector
        const std::size_t offset = position % block_size;  // Find the offset within the std::size_t

        if (index >= data.size())
        {
            data.resize(index + 1, default_bit_value ? block_ones : block_zeroes);
        }

        data[index] |= (static_cast<std::size_t>(1) << offset);  // Set the bit at the offset
    }

    void Bitset::unset(std::size_t position)
    {
        const std::size_t index = position / block_size;   // Find the index in the vector
        const std::size_t offset = position % block_size;  // Find the offset within the std::size_t

        if (index >= data.size())
        {
            data.resize(index + 1, default_bit_value ? block_ones : block_zeroes);
        }

        data[index] &= ~(static_cast<std::size_t>(1) << offset);  // Set the bit at the offset
    }

    bool Bitset::get(std::size_t position) const
    {
        const std::size_t index = position / block_size;

        if (index < data.size())
        {
            const std::size_t offset = position % block_size;
            return (data[index] & (static_cast<std::size_t>(1) << offset)) != 0;
        }
        else
        {
            return default_bit_value;
        }
    }

    std::size_t Bitset::next_set_bit(std::size_t position) const
    {
        std::size_t index = position / block_size;
        std::size_t offset = position % block_size;

        while (index < data.size())
        {
            // Shift so that we start checking from the offset
            const std::size_t value = data[index] >> offset;

            if (value)
            {
                // If there are set bits in the current value
                const auto lsb_position = get_lsb_position(value);
                return index * block_size + offset + lsb_position;
            }

            // Reset offset for the next value
            offset = 0;
            index++;
        }

        return no_position;
    }

    Bitset Bitset::operator|(const Bitset& other) const
    {
        Bitset result = *this;
        result.resize_to_fit(other);

        for (std::size_t i = 0; i < other.data.size(); ++i)
        {
            result.data[i] |= other.data[i];
        }

        return result;
    }

    Bitset Bitset::operator&(const Bitset& other) const
    {
        Bitset result = *this;
        result.resize_to_fit(other);

        for (std::size_t i = 0; i < other.data.size(); ++i)
        {
            result.data[i] &= other.data[i];
        }

        return result;
    }

    Bitset Bitset::operator~() const
    {
        Bitset result = *this;
        result.default_bit_value = !default_bit_value;

        for (std::size_t& value : result.data)
        {
            value = ~value;
        }

        return result;
    }

    Bitset& Bitset::operator|=(const Bitset& other)
    {
        this->resize_to_fit(other);

        for (std::size_t i = 0; i < other.data.size(); ++i)
        {
            this->data[i] |= other.data[i];
        }

        return *this;
    }

    Bitset& Bitset::operator&=(const Bitset& other)
    {
        this->resize_to_fit(other);

        for (std::size_t i = 0; i < other.data.size(); ++i)
        {
            this->data[i] &= other.data[i];
        }

        return *this;
    }

    bool Bitset::operator<(const Bitset& other) const
    {
        std::size_t common_size = std::min(data.size(), other.data.size());

        for (std::size_t index = 0; index < common_size; ++index)
        {
            if (data[index] < other.data[index])
            {
                return true;
            }
        }

        std::size_t max_size = std::max(data.size(), other.data.size());

        for (std::size_t index = common_size; index < max_size; ++index)
        {
            std::size_t this_value = index < data.size() ? data[index] : (default_bit_value ? block_ones : block_zeroes);
            std::size_t other_value = index < other.data.size() ? other.data[index] : (other.default_bit_value ? block_ones : block_zeroes);

            if (this_value < other_value)
            {
                return true;
            }
        }

        return false;
    }

    bool Bitset::operator==(const Bitset& other) const
    {
        std::size_t common_size = std::min(data.size(), other.data.size());

        for (std::size_t index = 0; index < common_size; ++index)
        {
            if (data[index] != other.data[index])
            {
                return false;
            }
        }

        std::size_t max_size = std::max(data.size(), other.data.size());

        for (std::size_t index = common_size; index < max_size; ++index)
        {
            std::size_t this_value = index < data.size() ? data[index] : (default_bit_value ? block_ones : block_zeroes);
            std::size_t other_value = index < other.data.size() ? other.data[index] : (other.default_bit_value ? block_ones : block_zeroes);

            if (this_value != other_value)
            {
                return false;
            }
        }

        return true;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Bitset>::operator()(const mimir::formalism::Bitset& bitset) const
    {
        const auto default_block = bitset.default_bit_value ? bitset.block_ones : bitset.block_zeroes;
        const auto seed = static_cast<uint32_t>(default_block);

        // Find the last block that differs from the default block
        auto last_relevant_index = static_cast<int64_t>(bitset.data.size()) - 1;
        for (; (last_relevant_index >= 0) && (bitset.data[last_relevant_index] == default_block); --last_relevant_index) {}
        const auto length = static_cast<std::size_t>(last_relevant_index + 1) * sizeof(std::size_t);

        // Compute a hash value up to and including this block
        int64_t hash[2];
        MurmurHash3_x64_128(&bitset.data[0], length, seed, hash);
        return static_cast<std::size_t>(hash[0] + 0x9e3779b9 + (hash[1] << 6) + (hash[1] >> 2));
    }

    bool less<mimir::formalism::Bitset>::operator()(const mimir::formalism::Bitset& left_bitset, const mimir::formalism::Bitset& right_bitset) const
    {
        return left_bitset < right_bitset;
    }

    bool equal_to<mimir::formalism::Bitset>::operator()(const mimir::formalism::Bitset& left_bitset, const mimir::formalism::Bitset& right_bitset) const
    {
        return left_bitset == right_bitset;
    }
}  // namespace std
