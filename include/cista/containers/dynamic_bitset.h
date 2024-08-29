#pragma once

#include "cista/bit_counting.h"
#include "cista/containers/vector.h"

#include <cassert>
#include <cinttypes>
#include <iosfwd>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace cista
{

template<typename Block, template<typename> typename Ptr>
struct basic_dynamic_bitset
{
    using block_type = Block;

    static constexpr std::size_t block_size = sizeof(Block) * 8;
    // 000...
    static constexpr Block block_zeroes = 0;
    // 111...
    static constexpr Block block_ones = Block(-1);
    // 100...
    static constexpr Block block_msb_one = Block(1) << (block_size - 1);
    // 011...
    static constexpr Block block_msb_zero = block_ones & (~block_msb_one);
    // 111...
    static constexpr std::size_t no_position = std::size_t(-1);

    constexpr basic_dynamic_bitset() noexcept = default;

    constexpr basic_dynamic_bitset(size_t num_bits) : default_bit_value_(false), blocks_((num_bits / block_size) + 1) {}

    constexpr basic_dynamic_bitset(size_t num_bits, bool default_bit_value) : default_bit_value_(default_bit_value), blocks_((num_bits / block_size) + 1) {}

    static constexpr size_t get_index(size_t position) noexcept { return position / block_size; }

    static constexpr size_t get_offset(size_t position) noexcept { return position % block_size; }

    /// @brief Shrink the bitset to minimum number of blocks to represent its bits.
    void shrink_to_fit()
    {
        int32_t last_non_default_block_index = blocks_.size() - 1;

        const Block default_block = default_bit_value_ ? block_ones : block_zeroes;

        for (; last_non_default_block_index >= 0; --last_non_default_block_index)
        {
            if (blocks_[last_non_default_block_index] != default_block)
            {
                break;
            }
        }

        blocks_.resize(last_non_default_block_index);
    }

    /// @brief Set a bit at a specific position
    /// @param position
    void set(std::size_t position)
    {
        const std::size_t index = get_index(position);
        const std::size_t offset = get_offset(position);

        if (index >= blocks_.size())
        {
            blocks_.resize(index + 1, default_bit_value_ ? block_ones : block_zeroes);
        }

        blocks_[index] |= (static_cast<Block>(1) << offset);  // Set the bit at the offset
    }

    /// @brief Unset a bit at a specific position
    /// @param position
    void unset(std::size_t position)
    {
        const std::size_t index = get_index(position);
        const std::size_t offset = get_offset(position);

        if (index >= blocks_.size())
        {
            blocks_.resize(index + 1, default_bit_value_ ? block_ones : block_zeroes);
        }

        blocks_[index] &= ~(static_cast<Block>(1) << offset);  // Set the bit at the offset
    }

    /// @brief Unset all bits and shrink its size to represent the bits
    void unset_all() { blocks_.clear(); }

    bool default_bit_value_;
    cista::basic_vector<Block, Ptr> blocks_ {};
};

namespace raw
{
template<typename Block>
using dynamic_bitset = basic_dynamic_bitset<Block, ptr>;
}

namespace offset
{
template<typename Block>
using dynamic_bitset = basic_dynamic_bitset<Block, ptr>;
}

}  // namespace cista
