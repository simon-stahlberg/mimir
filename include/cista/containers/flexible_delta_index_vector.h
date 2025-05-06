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
struct basic_flexible_delta_index_vector
{
private:
    template<std::unsigned_integral T>
    static inline constexpr uint8_t get_bit_width()
    {
        return std::numeric_limits<T>::digits;
    }

    inline size_t get_block_index(size_t pos) const { return pos >> elements_per_block_log2_; }

    inline size_t get_element_index(size_t pos) const { return pos & (elements_per_block_ - 1); }

    inline IndexType extract_value_at_position(size_t pos) const
    {
        if (bit_width_ == get_bit_width<IndexType>())  // Fast evaluate
        {
            return blocks_[pos];
        }

        const size_t block_index = get_block_index(pos);
        const size_t element_index = get_element_index(pos);
        const IndexType mask = (static_cast<IndexType>(1) << bit_width_) - 1;
        const size_t shift = (element_index << bit_width_log2_);

        return (blocks_[block_index] >> shift) & mask;
    }

public:
    using value_type = IndexType;

    basic_flexible_delta_index_vector() : bit_width_(0), bit_width_log2_(0), elements_per_block_(0), elements_per_block_log2_(0), size_(0), blocks_() {}

    basic_flexible_delta_index_vector(size_t size) :
        bit_width_(0),
        bit_width_log2_(0),
        elements_per_block_(0),
        elements_per_block_log2_(0),
        size_(size),
        blocks_(size)
    {
    }

    basic_flexible_delta_index_vector(size_t size, IndexType default_value) :
        bit_width_(0),
        bit_width_log2_(0),
        elements_per_block_(0),
        elements_per_block_log2_(0),
        size_(size),
        blocks_(size, default_value)
    {
    }

    basic_flexible_delta_index_vector(std::initializer_list<IndexType> init_list) :
        bit_width_(0),
        bit_width_log2_(0),
        elements_per_block_(0),
        elements_per_block_log2_(0),
        size_(init_list.size()),
        blocks_()
    {
        blocks_.reserve(size_);
        blocks_.insert(blocks_.end(), init_list.begin(), init_list.end());
    }

    /// @brief 0 is sentinel value for uncompressed!
    /// @return
    bool is_compressed() const { return bit_width_ != 0; }

    /**
     * Iterators
     */

    class const_iterator
    {
    private:
        const IndexType* blocks_;
        size_t size_;
        size_t pos_;
        IndexType mask_;
        IndexType value_;
        uint8_t bit_width_;
        uint8_t bit_width_log2_;
        uint8_t elements_per_block_;
        uint8_t elements_per_block_log2_;

        size_t get_block_index(size_t pos) const { return pos >> elements_per_block_log2_; }

        size_t get_element_index(size_t pos) const { return pos & (elements_per_block_ - 1); }

        IndexType extract_value_at_position(size_t pos) const
        {
            const size_t block_index = get_block_index(pos);
            const size_t element_index = get_element_index(pos);
            const size_t shift = (element_index << bit_width_log2_);

            return (blocks_[block_index] >> shift) & mask_;
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = IndexType;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        constexpr const_iterator() :
            blocks_(nullptr),
            size_(0),
            pos_(0),
            mask_(0),
            value_(0),
            bit_width_(0),
            bit_width_log2_(0),
            elements_per_block_(0),
            elements_per_block_log2_(0)
        {
        }
        constexpr const_iterator(const IndexType* blocks,
                                 size_t size,
                                 uint8_t bit_width,
                                 uint8_t bit_width_log2,
                                 uint8_t elements_per_block,
                                 uint8_t elements_per_block_log2,
                                 bool begin) :
            blocks_(blocks),
            size_(size),
            pos_(begin ? 0 : size),
            mask_((std::numeric_limits<IndexType>::max() >> (get_bit_width<IndexType>() - bit_width))),
            value_(0),
            bit_width_(bit_width),
            bit_width_log2_(bit_width_log2),
            elements_per_block_(elements_per_block),
            elements_per_block_log2_(elements_per_block_log2)
        {
            assert(bit_width_ > 0);

            if (begin && size > 0)
            {
                value_ = extract_value_at_position(0);
            }
        }

        constexpr value_type operator*() const
        {
            assert(pos_ < size_ && "Iterator out of bounds");

            return value_;
        }
        constexpr const_iterator& operator++()
        {
            value_ += extract_value_at_position(++pos_);
            return *this;
        }
        constexpr const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        constexpr const_iterator operator+(difference_type n) const
        {
            const_iterator tmp = *this;
            tmp += n;
            return tmp;
        }

        constexpr const_iterator& operator+=(difference_type n)
        {
            pos_ += n;
            return *this;
        }
        constexpr bool operator==(const const_iterator& other) const { return pos_ == other.pos_; }
        constexpr bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    auto uncompressed_begin()
    {
        assert(!is_compressed());
        return blocks_.begin();
    }
    auto uncompressed_end()
    {
        assert(!is_compressed());
        return blocks_.end();
    }
    auto uncompressed_begin() const
    {
        assert(!is_compressed());
        return blocks_.begin();
    }
    auto uncompressed_end() const
    {
        assert(!is_compressed());
        return blocks_.end();
    }
    auto uncompressed_range()
    {
        assert(!is_compressed());
        return std::ranges::subrange(uncompressed_begin(), uncompressed_end());
    }
    auto uncompressed_range() const
    {
        assert(!is_compressed());
        return std::ranges::subrange(uncompressed_begin(), uncompressed_end());
    }

    const_iterator compressed_begin()
    {
        assert(is_compressed());
        return const_iterator(blocks_.data(), size_, bit_width_, bit_width_log2_, elements_per_block_, elements_per_block_log2_, true);
    }
    const_iterator compressed_end()
    {
        assert(is_compressed());
        return const_iterator(blocks_.data(), size_, bit_width_, bit_width_log2_, elements_per_block_, elements_per_block_log2_, false);
    }
    const_iterator compressed_begin() const
    {
        assert(is_compressed());
        return const_iterator(blocks_.data(), size_, bit_width_, bit_width_log2_, elements_per_block_, elements_per_block_log2_, true);
    }
    const_iterator compressed_end() const
    {
        assert(is_compressed());
        return const_iterator(blocks_.data(), size_, bit_width_, bit_width_log2_, elements_per_block_, elements_per_block_log2_, false);
    }
    auto compressed_range()
    {
        assert(is_compressed());
        return std::ranges::subrange(compressed_begin(), compressed_end());
    }
    auto compressed_range() const
    {
        assert(is_compressed());
        return std::ranges::subrange(compressed_begin(), compressed_end());
    }

    /**
     * Modifiers
     */

    void push_back(IndexType value)
    {
        assert(!is_compressed());
        ++size_;
        blocks_.push_back(value);
    }

    void resize(size_t size)
    {
        assert(!is_compressed());
        size_ = size;
        blocks_.resize(size);
    }

    void resize(size_t size, IndexType init)
    {
        assert(!is_compressed());
        size_ = size;
        blocks_.resize(size, init);
    }

    void clear()
    {
        bit_width_ = 0;
        bit_width_log2_ = 0;
        elements_per_block_ = 0;
        elements_per_block_log2_ = 0;
        size_ = 0;
        blocks_.clear();
    }

    /**
     * Accessors
     */

    IndexType& operator[](size_t pos)
    {
        assert(!is_compressed());
        return blocks_[pos];
    }

    IndexType operator[](size_t pos) const
    {
        assert(!is_compressed());
        return extract_value_at_position(pos);
    }

    IndexType& at(size_t pos)
    {
        assert(!is_compressed());
        if (pos >= size_)
        {
            throw std::out_of_range("Index out of bounds in const operator[]");
        }

        return blocks_[pos];
    }

    IndexType at(size_t pos) const
    {
        assert(!is_compressed());
        if (pos >= size_)
        {
            throw std::out_of_range("Index out of bounds in const operator[]");
        }

        return extract_value_at_position(pos);
    }

    /**
     * Compression
     */

    void initialize_compressed_format()
    {
        assert(!is_compressed());

        IndexType max_delta = blocks_.empty() ? IndexType(0) : blocks_[0];
        for (size_t i = 1; i < blocks_.size(); ++i)
        {
            assert(blocks_[i] >= blocks_[i - 1]);
            max_delta = std::max(max_delta, static_cast<IndexType>(blocks_[i] - blocks_[i - 1]));
        }

        if (max_delta == 0)
        {
            bit_width_ = 1;
            bit_width_log2_ = 0;
            elements_per_block_ = get_bit_width<IndexType>();
            elements_per_block_log2_ = std::bit_width(elements_per_block_) - 1;
        }
        else
        {
            bit_width_ = std::bit_ceil(static_cast<IndexType>(std::bit_width(max_delta)));
            bit_width_log2_ = std::bit_width(bit_width_) - 1;
            elements_per_block_ = get_bit_width<IndexType>() >> bit_width_log2_;
            elements_per_block_log2_ = std::bit_width(elements_per_block_) - 1;
        }

        // std::cout << "max_delta: " << max_delta << "\n"
        //           << "bit_width_: " << static_cast<size_t>(bit_width_) << "\n"
        //           << "bit_width_log2_: " << static_cast<size_t>(bit_width_log2_) << "\n"
        //           << "elements_per_block_: " << static_cast<size_t>(elements_per_block_) << "\n"
        //           << "elements_per_block_log2_: " << static_cast<size_t>(elements_per_block_log2_) << std::endl;
    }

    void compress()
    {
        initialize_compressed_format();

        // Now compress blocks_
        const IndexType mask =
            std::numeric_limits<IndexType>::max() >> (get_bit_width<IndexType>() - bit_width_);  ///< Mask for extracting the required number of bits
        size_t shift_bits = 0;                                                                   ///< Tracks the bit position for packing smaller values
        size_t packed_index = 0;                                                                 ///< New position in blocks_ for writing packed data
        IndexType packed_buffer = 0;                                                             ///< Buffer for packing smaller values
        size_t element_index = 0;                                                                ///< the index of an element in the packed_buffer
        [[maybe_unused]] size_t element_count = 0;                                               ///< just for assertion.

        IndexType prev = 0;
        for (IndexType block : blocks_)
        {
            IndexType delta = block - prev;
            prev = block;

            ++element_count;

            // Mask the current block to ensure it's within the bit width
            IndexType value = delta & mask;

            // Add the value into the current packed buffer
            assert(shift_bits < sizeof(IndexType) * 8);
            packed_buffer |= value << shift_bits;
            shift_bits += bit_width_;

            // Write the buffer if its full
            if (++element_index == elements_per_block_)
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
    uint8_t bit_width_log2() const { return bit_width_log2_; }
    uint8_t elements_per_block() const { return elements_per_block_; };
    uint8_t elements_per_block_log2() const { return elements_per_block_log2_; };

    IndexType size() const { return size_; }
    bool empty() const { return size_ == 0; }
    const cista::basic_vector<IndexType, Ptr>& blocks() const { return blocks_; }

    /**
     * Members
     */

    uint8_t bit_width_;
    uint8_t bit_width_log2_;
    uint8_t elements_per_block_;
    uint8_t elements_per_block_log2_;
    IndexType size_;
    cista::basic_vector<IndexType, Ptr> blocks_ {};
};

template<std::unsigned_integral IndexType, template<typename> typename Ptr>
constexpr bool operator==(const basic_flexible_delta_index_vector<IndexType, Ptr>& lhs, const basic_flexible_delta_index_vector<IndexType, Ptr>& rhs)
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
using flexible_delta_index_vector = basic_flexible_delta_index_vector<IndexType, ptr>;
}

namespace offset
{
template<std::unsigned_integral IndexType>
using flexible_delta_index_vector = basic_flexible_delta_index_vector<IndexType, ptr>;
}

}  // namespace cista
