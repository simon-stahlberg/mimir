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

    bool is_uncompressed() const { return bit_width_ == get_bit_width<IndexType>(); }

    size_t get_elements_per_block() const { return static_cast<size_t>(get_bit_width<IndexType>() / bit_width_); }

    size_t get_block_index(size_t pos, size_t elements_per_block) const { return pos / elements_per_block; }

    size_t get_element_index(size_t pos, size_t elements_per_block) const { return pos % elements_per_block; }

    IndexType extract_value_at_position(size_t pos) const
    {
        if (bit_width_ == get_bit_width<IndexType>())  // Fast evaluate
        {
            return blocks_[pos];
        }

        const size_t elements_per_block = get_elements_per_block();
        const size_t block_index = get_block_index(pos, elements_per_block);
        const size_t element_index = get_element_index(pos, elements_per_block);
        const size_t shift = (element_index * bit_width_);
        const IndexType mask = ((static_cast<IndexType>(1) << bit_width_) - 1);

        return (blocks_[block_index] >> shift) & mask;
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

        size_t get_elements_per_block() const { return static_cast<size_t>(get_bit_width<IndexType>() / bit_width_); }

        size_t get_block_index(size_t pos, size_t elements_per_block) const { return pos / elements_per_block; }

        size_t get_element_index(size_t pos, size_t elements_per_block) const { return pos % elements_per_block; }

        IndexType extract_value_at_position(size_t pos) const
        {
            if (bit_width_ == get_bit_width<IndexType>())  // Fast evaluate
            {
                return (*blocks_)[pos];
            }

            const size_t elements_per_block = get_elements_per_block();
            const size_t block_index = get_block_index(pos, elements_per_block);
            const size_t element_index = get_element_index(pos, elements_per_block);
            const size_t shift = (element_index * bit_width_);
            const IndexType mask = ((static_cast<IndexType>(1) << bit_width_) - 1);

            return ((*blocks_)[block_index] >> shift) & mask;
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
        assert(is_uncompressed());
        return blocks_.begin();
    }
    auto end()
    {
        assert(is_uncompressed());
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
        assert(is_uncompressed());
        ++size_;
        blocks_.push_back(value);
    }

    void resize(size_t size)
    {
        assert(is_uncompressed());
        size_ = size;
        blocks_.resize(size);
    }

    void resize(size_t size, IndexType init)
    {
        assert(is_uncompressed());
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
        assert(is_uncompressed());
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
        assert(is_uncompressed());

        uint8_t max_bit_width = 0;

        for (const auto& block : blocks_)
        {
            // Calculate the bit width of the current block
            uint8_t bit_width = (block == 0) ? 1 : static_cast<uint8_t>(std::bit_width(block));
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
        const size_t elements_per_block = get_elements_per_block();
        const IndexType mask = (1ULL << bit_width_) - 1;  ///< Mask for extracting the required number of bits
        size_t shift_bits = 0;                            ///< Tracks the bit position for packing smaller values
        size_t packed_index = 0;                          ///< New position in blocks_ for writing packed data
        IndexType packed_buffer = 0;                      ///< Buffer for packing smaller values
        size_t element_index = 0;                         ///< the index of an element in the packed_buffer
        [[maybe_unused]] size_t element_count = 0;        ///< just for assertion.

        for (IndexType block : blocks_)
        {
            ++element_count;

            // Mask the current block to ensure it's within the bit width
            IndexType value = block & mask;

            // Add the value into the current packed buffer
            assert(shift_bits < sizeof(IndexType) * 8);
            packed_buffer |= value << shift_bits;
            shift_bits += bit_width_;

            // Write the buffer if its full
            if (++element_index == elements_per_block)
            {
                assert(packed_index < element_count);
                assert(packed_index < blocks_.size());

                blocks_[packed_index++] = packed_buffer;

                packed_buffer = 0;
                shift_bits = 0;
                element_index = 0;
            }
        }

        // Write any remaining data that was not flushed
        if (shift_bits > 0)
        {
            assert(packed_index < element_count);
            assert(packed_index < blocks_.size());

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
