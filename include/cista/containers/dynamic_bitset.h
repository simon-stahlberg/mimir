#pragma once

#include "cista/bit_counting.h"
#include "cista/containers/vector.h"
#include "cista/hash.h"

#include <bit>
#include <cassert>
#include <cinttypes>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

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

    static constexpr size_t get_index(size_t position) noexcept { return position / block_size; }

    static constexpr size_t get_offset(size_t position) noexcept { return position % block_size; }

    static constexpr size_t get_lsb_position(Block n) noexcept
    {
        assert(n != 0);
        const Block v = n & (-n);
        return std::bit_width(v) - 1;  // bit_width uses more efficient specialized cpu instructions
    }

    /**
     * Constructors
     */

    constexpr basic_dynamic_bitset() noexcept = default;

    constexpr basic_dynamic_bitset(size_t num_bits) : default_bit_value_(false), blocks_((num_bits / block_size) + 1) {}

    constexpr basic_dynamic_bitset(size_t num_bits, bool default_bit_value) : default_bit_value_(default_bit_value), blocks_((num_bits / block_size) + 1) {}

    /**
     * Iterators
     */

    class const_iterator
    {
    private:
        // Underlying data
        const Block* m_blocks;
        size_t m_num_blocks;

        // The current block to be worked on
        size_t m_block_index;
        size_t m_bit_index;
        size_t m_cur_block;

        // The iterator positions
        size_t m_end_pos;
        size_t m_pos;

        constexpr void next_set_bit()
        {
            assert(m_pos != m_end_pos);
            do
            {
                // Advance position
                ++m_pos;
                ++m_bit_index;
                if (m_bit_index == block_size)
                {
                    ++m_block_index;
                    if (m_block_index == m_num_blocks)
                    {
                        // Reached end of blocks, set m_pos to end
                        m_pos += block_size - 1;
                        break;
                    }
                    m_bit_index = 0;
                    m_cur_block = m_blocks[m_block_index];
                }

                if (m_cur_block)
                {
                    // If there are set bits in the current value
                    const auto lsb_position = get_lsb_position(m_cur_block);
                    m_bit_index += lsb_position;
                    m_pos += lsb_position;
                    // shift by + 1 to advance further
                    m_cur_block >>= lsb_position + 1;
                    break;
                }
                else
                {
                    // Skip the remaining bits, point to last position in the current block
                    m_pos += block_size - m_bit_index - 1;
                    ++m_block_index;
                    m_bit_index = -1;
                    // Fetch next data block or zeroes
                    m_cur_block = m_block_index < m_num_blocks ? m_blocks[m_block_index] : block_zeroes;
                }
            } while (m_pos < m_end_pos);
        }

    public:
        using difference_type = int;
        using value_type = size_t;
        using pointer = size_t*;
        using reference = size_t&;
        using iterator_category = std::forward_iterator_tag;

        constexpr const_iterator() : m_blocks(nullptr), m_num_blocks(0), m_block_index(0), m_bit_index(-1) {}
        constexpr const_iterator(bool default_bit_value, const Block* blocks, size_t num_blocks, bool begin) :
            m_blocks(blocks),
            m_num_blocks(num_blocks),
            m_block_index(0),
            m_bit_index(-1),
            m_cur_block(num_blocks > 0 ? m_blocks[m_block_index] : block_zeroes),
            m_end_pos((m_num_blocks + 1) * block_size - 1),
            m_pos(begin ? -1 : m_end_pos)
        {
            if (default_bit_value)
            {
                throw std::runtime_error("Cannot iterate over infinite set.");
            }
            if (begin && m_pos != m_end_pos)
            {
                // Operator::block_msb_one
                if ((num_blocks > 0) && ((m_blocks[0] & 1) == 0))
                {
                    // The first bit is not set, so we need to find it
                    next_set_bit();
                }
                else if (num_blocks == 0)
                {
                    m_pos = m_end_pos;
                }
                else
                {
                    // The first bit is set, advance by 1 manually
                    m_pos = 0;
                    m_bit_index = 0;
                    m_cur_block >>= 1;
                }
            }
        }
        constexpr size_t operator*() const
        {
            // Do not allow interpreting end as position.
            assert(m_pos < m_end_pos);
            return m_pos;
        }
        constexpr const_iterator& operator++()
        {
            next_set_bit();
            return *this;
        }
        constexpr const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        constexpr bool operator==(const const_iterator& other) const { return m_pos == other.m_pos; }
        constexpr bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    constexpr const_iterator begin() const { return const_iterator(default_bit_value_, blocks_.data(), blocks_.size(), true); }

    constexpr const_iterator end() const { return const_iterator(default_bit_value_, blocks_.data(), blocks_.size(), false); }

    /**
     * Utility
     */

    /// @brief Shrink the bitset to minimum number of blocks to represent its bits.
    constexpr void shrink_to_fit()
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

        blocks_.resize(last_non_default_block_index + 1);
    }

    constexpr void resize_to_fit(const basic_dynamic_bitset& other)
    {
        if (blocks_.size() < other.blocks_.size())
        {
            blocks_.resize(other.blocks_.size(), default_bit_value_ ? block_ones : block_zeroes);
        }
    }

    constexpr std::size_t next_set_bit(std::size_t position) const
    {
        std::size_t index = get_index(position);
        std::size_t offset = get_offset(position);

        for (auto iter = blocks_.begin() + index; iter < blocks_.end(); ++iter)
        {
            // Shift so that we start checking from the offset
            const Block value = *iter >> offset;

            if (value)
            {
                // If there are set bits in the current value
                const auto lsb_position = get_lsb_position(value);
                return index * block_size + offset + lsb_position;
            }

            // Reset offset for the next value
            offset = 0;
        }

        return no_position;
    }

    /**
     * Bit fiddeling
     */

    constexpr bool get(std::size_t position) const
    {
        const std::size_t index = get_index(position);

        if (index < blocks_.size())
        {
            return (blocks_[index] & (static_cast<Block>(1) << get_offset(position))) != 0;
        }
        else
        {
            return default_bit_value_;
        }
    }

    /// @brief Set a bit at a specific position
    /// @param position
    constexpr void set(std::size_t position)
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
    constexpr void unset(std::size_t position)
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
    constexpr void unset_all() { blocks_.clear(); }

    /**
     * Bitset operations
     */

    constexpr basic_dynamic_bitset& operator~()
    {
        default_bit_value_ = !default_bit_value_;

        for (Block& value : blocks_)
        {
            value = ~value;
        }

        return *this;
    }

    constexpr basic_dynamic_bitset operator|(const basic_dynamic_bitset& other) const
    {
        auto result = basic_dynamic_bitset(*this);
        result |= other;

        return result;
    }

    constexpr basic_dynamic_bitset& operator|=(const basic_dynamic_bitset& other)
    {
        // Update default bit value
        default_bit_value_ |= other.default_bit_value_;

        // Update blocks
        resize_to_fit(other);
        // Other blocks might still be smaller which is fine
        assert(other.blocks_.size() <= blocks_.size());

        auto it = blocks_.begin();
        auto other_it = other.blocks_.begin();
        // Since other is potentially smaller, it acts as termination conditions
        while (other_it != other.blocks_.end())
        {
            *it |= *other_it;
            ++it;
            ++other_it;
        }

        return *this;
    }

    constexpr basic_dynamic_bitset operator&(const basic_dynamic_bitset& other) const
    {
        auto result = basic_dynamic_bitset(*this);
        result &= other;

        return result;
    }

    constexpr basic_dynamic_bitset& operator&=(const basic_dynamic_bitset& other)
    {
        // Update default bit value
        default_bit_value_ &= other.default_bit_value_;

        // Update blocks
        resize_to_fit(other);
        // Other blocks might still be smaller which is fine
        assert(other.blocks_.size() <= blocks_.size());

        auto it = blocks_.begin();
        auto other_it = other.blocks_.begin();
        while (other_it != other.blocks_.end())
        {
            *it &= *other_it;
            ++it;
            ++other_it;
        }
        // Shrink to size of other since those bits should become default valued
        blocks_.resize(other.blocks_.size());

        return *this;
    }

    constexpr basic_dynamic_bitset operator-(const basic_dynamic_bitset& other) const
    {
        auto result = basic_dynamic_bitset(*this);
        result -= other;

        return result;
    }

    constexpr basic_dynamic_bitset& operator-=(const basic_dynamic_bitset& other)
    {
        // Update default bit value
        default_bit_value_ &= !other.default_bit_value_;

        // Update blocks
        resize_to_fit(other);
        // Other blocks might still be smaller which is fine
        assert(other.blocks_.size() <= blocks_.size());

        auto it = blocks_.begin();
        auto other_it = other.blocks_.begin();
        while (other_it != other.blocks_.end())
        {
            *it &= ~(*other_it);
            ++it;
            ++other_it;
        }
        // The remaining blocks stay the same.

        return *this;
    }

    constexpr bool is_superseteq(const basic_dynamic_bitset& other) const
    {
        if (other.default_bit_value_ && !default_bit_value_)
        {
            // blocks has finitely many and other blocks has infinitely many set bits.
            // Hence blocks cannot be a superseteq of other_blocks.
            return false;
        }

        std::size_t common_size = std::min(blocks_.size(), other.blocks_.size());

        for (std::size_t index = 0; index < common_size; ++index)
        {
            if ((blocks_[index] & other.blocks_[index]) != other.blocks_[index])
            {
                // There exists a set bit in other block that is not set in block.
                return false;
            }
        }

        if (other.blocks_.size() <= blocks_.size())
        {
            // blocks can only contain additional set bits
            return true;
        }

        if (default_bit_value_)
        {
            return true;
        }

        for (std::size_t index = common_size; index < other.blocks_.size(); ++index)
        {
            if (other.blocks_[index])
            {
                // other_block contains additional set bits
                return false;
            }
        }

        return true;
    }

    constexpr bool are_disjoint(const basic_dynamic_bitset& other) const
    {
        if (default_bit_value_ && other.default_bit_value_)
        {
            // blocks and other blocks have infinitely many set bits after finite sized explicit bitsets.
            // Hence blocks and other_blocks cannot be disjoint.
            return false;
        }

        std::size_t common_size = std::min(blocks_.size(), other.blocks_.size());

        for (std::size_t index = 0; index < common_size; ++index)
        {
            if ((blocks_[index] & other.blocks_[index]) > 0)
            {
                // block and other_block have set bits in common
                return false;
            }
        }

        if (default_bit_value_ && !other.default_bit_value_)
        {
            for (std::size_t index = common_size; index < other.blocks_.size(); ++index)
            {
                if (other.blocks_[index] > 0)
                {
                    // other_blocks has a set bit in common with blocks in the infinite part.
                    return false;
                }
            }
        }

        if (!default_bit_value_ && other.default_bit_value_)
        {
            for (std::size_t index = common_size; index < blocks_.size(); ++index)
            {
                if (blocks_[index] > 0)
                {
                    // blocks has a set bit in common with other_blocks in the infinite part.
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * Accessors
     */

    constexpr size_t count() const
    {
        size_t count = 0;
        for (auto it = begin(); it != end(); ++it)
        {
            ++count;
        }
        return count;
    }

    bool default_bit_value_;
    cista::basic_vector<Block, Ptr> blocks_ {};
};

template<typename Block, template<typename> typename Ptr>
constexpr bool operator==(const basic_dynamic_bitset<Block, Ptr>& lhs, const basic_dynamic_bitset<Block, Ptr>& rhs)
{
    using Type = basic_dynamic_bitset<Block, Ptr>;
    if (&lhs != &rhs)
    {
        std::size_t common_size = std::min(lhs.blocks_.size(), rhs.blocks_.size());
        if (std::memcmp(lhs.blocks_.data(), rhs.blocks_.data(), common_size * sizeof(Block)) != 0)
            return false;

        std::size_t max_size = std::max(lhs.blocks_.size(), rhs.blocks_.size());

        for (std::size_t index = common_size; index < max_size; ++index)
        {
            auto this_value = index < lhs.blocks_.size() ? lhs.blocks_[index] : (lhs.default_bit_value_ ? Type::block_ones : Type::block_zeroes);
            auto other_value = index < rhs.blocks_.size() ? rhs.blocks_[index] : (rhs.default_bit_value_ ? Type::block_ones : Type::block_zeroes);

            if (this_value != other_value)
            {
                return false;
            }
        }

        return true;
    }

    return true;
}

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
