#pragma once

#include "cista/bit_counting.h"
#include "cista/containers/vector.h"
#include "cista/hash.h"

#include <bit>
#include <cassert>
#include <cinttypes>
#include <concepts>
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

template<std::unsigned_integral IndexType, template<typename> typename Ptr>
struct basic_flexible_index_vector
{
private:
    template<std::unsigned_integral T>
    static constexpr uint8_t get_bit_width()
    {
        return std::numeric_limits<T>::digits;
    }

    void ensure_uncompressed() const
    {
        if (bit_width_ != get_bit_width<IndexType>())
        {
            throw std::runtime_error("Operation not supported after compression");
        }
    }

    IndexType extract_value_at_position(size_t pos) const
    {
        // Calculate number of bits needed for each element
        const size_t type_bit_width = static_cast<size_t>(bit_width_);
        const size_t block_bit_width = get_bit_width<IndexType>();  // Number of bits in each block (e.g., 64 bits for uint64_t)

        // Calculate the bit position of the element within the compressed data
        const size_t bit_position = pos * type_bit_width;
        const size_t block_index = bit_position / block_bit_width;  // Index of the block containing the element
        const size_t bit_offset = bit_position % block_bit_width;   // Offset within the block

        // Calculate mask
        const IndexType mask = (1ULL << bit_width_) - 1;

        // Extract the element from the block
        const IndexType block = blocks_[block_index];
        IndexType value = (block >> bit_offset) & mask;

        // Handle elements spanning multiple blocks
        size_t remaining_bits = std::max(bit_offset + type_bit_width, block_bit_width) - block_bit_width;
        if (remaining_bits > 0)
        {
            // Calculate mask for bits in the next block.
            const IndexType remaining_mask = (1ULL << remaining_bits) - 1;

            // Extract remaining bits from the next block and shift accordingly.
            const IndexType next_block = blocks_[block_index + 1];
            value |= ((next_block & remaining_mask) << (type_bit_width - remaining_bits));
        }

        return value;
    }

public:
    using value_type = IndexType;

    basic_flexible_index_vector() : bit_width_(get_bit_width<IndexType>()), size_(0), blocks_() {}

    basic_flexible_index_vector(size_t size) : bit_width_(get_bit_width<IndexType>()), size_(size), blocks_(size) {}

    basic_flexible_index_vector(size_t size, IndexType default_value) : bit_width_(get_bit_width<IndexType>()), size_(size), blocks_(size, default_value) {}

    basic_flexible_index_vector(std::initializer_list<IndexType> init_list) : bit_width_(get_bit_width<IndexType>()), size_(init_list.size()), blocks_()
    {
        blocks_.reserve(size_);
        blocks_.insert(blocks_.end(), init_list.begin(), init_list.end());
    }

    /**
     * Iterators
     */

    class const_iterator
    {
    private:
        uint8_t bit_width_;
        size_t size_;
        const cista::basic_vector<IndexType, Ptr>* blocks_;

        size_t pos_;

        IndexType extract_value_at_position(size_t pos) const
        {
            // Calculate number of bits needed for each element
            const size_t type_bit_width = static_cast<size_t>(bit_width_);
            const size_t block_bit_width = get_bit_width<IndexType>();  // Number of bits in each block (e.g., 64 bits for uint64_t)

            // Calculate the bit position of the element within the compressed data
            const size_t bit_position = pos * type_bit_width;
            const size_t block_index = bit_position / block_bit_width;  // Index of the block containing the element
            const size_t bit_offset = bit_position % block_bit_width;   // Offset within the block

            // Calculate mask
            const IndexType mask = (1ULL << bit_width_) - 1;

            // Extract the element from the block
            const IndexType block = (*blocks_)[block_index];
            IndexType value = (block >> bit_offset) & mask;

            // Handle elements spanning multiple blocks
            size_t remaining_bits = std::max(bit_offset + type_bit_width, block_bit_width) - block_bit_width;
            if (remaining_bits > 0)
            {
                // Calculate mask for bits in the next block.
                const IndexType remaining_mask = (1ULL << remaining_bits) - 1;

                // Extract remaining bits from the next block and shift accordingly.
                const IndexType next_block = (*blocks_)[block_index + 1];
                value |= ((next_block & remaining_mask) << (type_bit_width - remaining_bits));
            }

            return value;
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = IndexType;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        constexpr const_iterator() : bit_width_(0), size_(0), blocks_(nullptr), pos_(0) {}
        constexpr const_iterator(uint8_t bit_width, size_t size, const cista::basic_vector<IndexType, Ptr>& blocks, bool begin) :
            bit_width_(bit_width),
            size_(size),
            blocks_(&blocks),
            pos_(begin ? 0 : size_)
        {
        }

        constexpr value_type operator*() const
        {
            assert(pos_ < size_ && "Iterator out of bounds");

            return extract_value_at_position(pos_);
        }
        constexpr const_iterator& operator++()
        {
            ++pos_;
            return *this;
        }
        constexpr const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        constexpr bool operator==(const const_iterator& other) const { return pos_ == other.pos_; }
        constexpr bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    const_iterator begin() const { return const_iterator(bit_width_, size_, blocks_, true); }
    const_iterator end() const { return const_iterator(bit_width_, size_, blocks_, false); }
    auto begin()
    {
        ensure_uncompressed();
        return blocks_.begin();
    }
    auto end()
    {
        ensure_uncompressed();
        return blocks_.end();
    }

    // We sometimes need const iterators in assertions, we cannot ensured uncompressed here.
    const_iterator cbegin() { return const_iterator(bit_width_, size_, blocks_, true); }
    const_iterator cend() { return const_iterator(bit_width_, size_, blocks_, false); }

    /**
     * Modifiers
     */

    void push_back(IndexType value)
    {
        ensure_uncompressed();
        ++size_;
        blocks_.push_back(value);
    }

    void resize(size_t size)
    {
        ensure_uncompressed();
        size_ = size;
        blocks_.resize(size);
    }

    void resize(size_t size, IndexType init)
    {
        ensure_uncompressed();
        size_ = size;
        blocks_.resize(size, init);
    }

    void clear()
    {
        bit_width_ = get_bit_width<IndexType>();
        size_ = 0;
        blocks_.clear();
    }

    /**
     * Accessors
     */

    IndexType& operator[](size_t pos)
    {
        ensure_uncompressed();
        return blocks_[pos];
    }

    IndexType operator[](size_t pos) const { return extract_value_at_position(pos); }

    IndexType& at(size_t pos)
    {
        if (pos >= size_)
        {
            throw std::out_of_range("Index out of bounds in const operator[]");
        }

        return blocks_[pos];
    }

    IndexType at(size_t pos) const
    {
        if (pos >= size_)
        {
            throw std::out_of_range("Index out of bounds in const operator[]");
        }

        return extract_value_at_position(pos);
    }

    /**
     * Compression
     */

    uint8_t determine_bit_width() const
    {
        ensure_uncompressed();

        uint8_t max_bit_width = 0;

        for (const auto& block : blocks_)
        {
            // Calculate the bit width of the current block
            uint8_t bit_width = block == 0 ? 1 : static_cast<uint8_t>(std::bit_width(block));
            max_bit_width = std::max(max_bit_width, bit_width);
        }

        return max_bit_width;
    }

    void compress()
    {
        if (blocks_.size() <= 1)
        {
            return;  // Already optimal
        }

        bit_width_ = determine_bit_width();

        if (bit_width_ == get_bit_width<IndexType>())
        {
            return;  // Already optimal
        }

        // Now compress blocks_
        const IndexType mask = (1ULL << bit_width_) - 1;  // Mask for extracting the required number of bits
        size_t shift_bits = 0;                            // Tracks the bit position for packing smaller values
        size_t packed_index = 0;                          // New position in blocks_ for writing packed data
        IndexType packed_buffer = 0;                      // Buffer for packing smaller values

        for (IndexType block : blocks_)
        {
            // Mask the current block to ensure it's within the bit width
            IndexType value = block & mask;

            // Add the value into the current packed buffer
            packed_buffer |= value << shift_bits;
            shift_bits += bit_width_;

            // If the packed buffer is full, flush it to the blocks_
            if (shift_bits >= get_bit_width<IndexType>())
            {
                blocks_[packed_index++] = packed_buffer;

                // Calculate the remaining bits beyond the block's capacity
                const size_t overflow_bits = shift_bits - get_bit_width<IndexType>();

                // Start the next packed_buffer with the overflow bits
                packed_buffer = (value >> (bit_width_ - overflow_bits));

                // Reset shift_bits for the next block
                shift_bits = overflow_bits;
            }
        }

        // Write any remaining data that was not flushed
        if (shift_bits > 0)
        {
            blocks_[packed_index++] = packed_buffer;
        }

        // Resize the blocks_ to discard unused space
        blocks_.resize(packed_index);
    }

    /**
     * Getters
     */

    uint8_t bit_width() const { return bit_width_; }
    IndexType size() const { return size_; }
    const cista::basic_vector<IndexType, Ptr>& blocks() const { return blocks_; }

    /**
     * Members
     */

    uint8_t bit_width_;
    IndexType size_;
    cista::basic_vector<IndexType, Ptr> blocks_ {};
};

template<std::unsigned_integral IndexType, template<typename> typename Ptr>
constexpr bool operator==(const basic_flexible_index_vector<IndexType, Ptr>& lhs, const basic_flexible_index_vector<IndexType, Ptr>& rhs)
{
    // TODO: Do we want to allow comparison of compressed and uncompressed?

    // Compare size and bit width first for early exits
    if (lhs.size() != rhs.size() || lhs.bit_width_ != rhs.bit_width_)
    {
        return false;
    }

    // Use std::equal to compare the blocks_ if sizes and bit widths match
    return std::equal(lhs.blocks_.begin(), lhs.blocks_.end(), rhs.blocks_.begin());
}

namespace raw
{
template<std::unsigned_integral IndexType>
using flexible_index_vector = basic_flexible_index_vector<IndexType, ptr>;
}

namespace offset
{
template<std::unsigned_integral IndexType>
using flexible_index_vector = basic_flexible_index_vector<IndexType, ptr>;
}

}  // namespace cista
