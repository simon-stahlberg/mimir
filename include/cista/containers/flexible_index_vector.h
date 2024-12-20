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

/// @brief `FlexibleIndexVectorDataType` encodes the data type used after compression inside of a `basic_flexible_index_vector`.
enum class FlexibleIndexVectorDataType
{
    UINT8_T = 1,
    UINT16_T = 2,
    UINT32_T = 4,
    UINT64_T = 8,
};

template<std::unsigned_integral IndexType, template<typename> typename Ptr>
struct basic_flexible_index_vector
{
private:
    void ensure_uncompressed() const
    {
        if (type_ != max_type)
        {
            throw std::runtime_error("Operation not supported after compression");
        }
    }

    template<typename T>
    static constexpr FlexibleIndexVectorDataType get_max_type()
    {
        if constexpr (std::is_same_v<T, uint8_t>)
            return FlexibleIndexVectorDataType::UINT8_T;
        else if constexpr (std::is_same_v<T, uint16_t>)
            return FlexibleIndexVectorDataType::UINT16_T;
        else if constexpr (std::is_same_v<T, uint32_t>)
            return FlexibleIndexVectorDataType::UINT32_T;
        else if constexpr (std::is_same_v<T, uint64_t>)
            return FlexibleIndexVectorDataType::UINT64_T;
        else
            static_assert(std::is_unsigned_v<T>, "Unsupported IndexType. Must be an unsigned integral type.");
    }

    template<typename T>
    static constexpr size_t get_bit_width()
    {
        if constexpr (std::is_same_v<T, uint8_t>)
            return 8;
        else if constexpr (std::is_same_v<T, uint16_t>)
            return 16;
        else if constexpr (std::is_same_v<T, uint32_t>)
            return 32;
        else if constexpr (std::is_same_v<T, uint64_t>)
            return 64;
        else
            static_assert(std::is_unsigned_v<T>, "Unsupported IndexType. Must be an unsigned integral type.");
    }

public:
    using value_type = IndexType;

    static constexpr const FlexibleIndexVectorDataType max_type = get_max_type<IndexType>();

    basic_flexible_index_vector() : type_(max_type), size_(0), blocks_() {}

    basic_flexible_index_vector(size_t size) : type_(max_type), size_(size), blocks_(size) {}

    basic_flexible_index_vector(size_t size, IndexType default_value) : type_(max_type), size_(size), blocks_(size, default_value) {}

    basic_flexible_index_vector(std::initializer_list<IndexType> init_list) : type_(max_type), size_(init_list.size()), blocks_()
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
        FlexibleIndexVectorDataType type_;
        size_t size_;
        const cista::basic_vector<IndexType, Ptr>* blocks_;

        size_t pos_;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = IndexType;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        constexpr const_iterator() {}
        constexpr const_iterator(FlexibleIndexVectorDataType type, size_t size, const cista::basic_vector<IndexType, Ptr>& blocks, bool begin) :
            type_(type),
            size_(size),
            blocks_(&blocks),
            pos_(begin ? 0 : size_)
        {
        }

        constexpr value_type operator*() const
        {
            assert(pos_ < size_ && "Iterator out of bounds");

            size_t type_bit_width = static_cast<size_t>(type_) * 8;
            size_t elements_per_block = sizeof(IndexType) / static_cast<size_t>(type_);
            size_t block_index = pos_ / elements_per_block;
            size_t element_offset = pos_ % elements_per_block;

            const uint64_t& block = (*blocks_)[block_index];
            return (block >> (element_offset * type_bit_width)) & ((1ULL << type_bit_width) - 1);
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

    const_iterator begin() const { return const_iterator(type_, size_, blocks_, true); }
    const_iterator end() const { return const_iterator(type_, size_, blocks_, false); }
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
        type_ = max_type;
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

    IndexType operator[](size_t pos) const
    {
        size_t type_bit_width = static_cast<size_t>(type_) * 8;
        size_t elements_per_block = sizeof(IndexType) / static_cast<size_t>(type_);
        size_t block_index = pos / elements_per_block;
        size_t element_offset = pos % elements_per_block;

        const uint64_t& block = blocks_[block_index];
        return (block >> (element_offset * type_bit_width)) & ((1ULL << type_bit_width) - 1);
    }

    IndexType at(size_t pos) const
    {
        if (pos >= size_)
        {
            throw std::out_of_range("Index out of bounds in const operator[]");
        }

        size_t type_bit_width = static_cast<size_t>(type_) * 8;
        size_t elements_per_block = sizeof(IndexType) / static_cast<size_t>(type_);
        size_t block_index = pos / elements_per_block;
        size_t element_offset = pos % elements_per_block;

        const uint64_t& block = blocks_[block_index];
        return (block >> (element_offset * type_bit_width)) & ((1ULL << type_bit_width) - 1);
    }

    /**
     * Compression
     */

    FlexibleIndexVectorDataType determine_type() const
    {
        ensure_uncompressed();

        FlexibleIndexVectorDataType detected_type = FlexibleIndexVectorDataType::UINT8_T;

        for (const auto& block : blocks_)
        {
            if (block > std::numeric_limits<uint32_t>::max())
            {
                return FlexibleIndexVectorDataType::UINT64_T;  // Early exit, as it's the largest type
            }
            else if (block > std::numeric_limits<uint16_t>::max())
            {
                detected_type = FlexibleIndexVectorDataType::UINT32_T;
            }
            else if (block > std::numeric_limits<uint8_t>::max())
            {
                detected_type = FlexibleIndexVectorDataType::UINT16_T;
            }
        }

        return detected_type;
    }

    void compress()
    {
        type_ = determine_type();

        // No compression needed for max type
        if (type_ == max_type)
        {
            return;
        }

        // Now compress blocks_
        size_t type_bit_width = static_cast<size_t>(type_) * 8;
        uint64_t mask = (1ULL << type_bit_width) - 1;
        size_t shift_bits = 0;        // Tracks the bit position for packing smaller types
        size_t new_index = 0;         // New position in blocks_ for writing packed data
        uint64_t current_packed = 0;  // Buffer for packing smaller values

        for (uint64_t block : blocks_)
        {
            uint64_t value = block & mask;

            current_packed |= value << shift_bits;
            shift_bits += type_bit_width;

            if (shift_bits == get_bit_width<IndexType>())
            {
                blocks_[new_index++] = current_packed;
                current_packed = 0;
                shift_bits = 0;
            }
        }

        // Write any remaining data that was not flushed
        if (shift_bits > 0)
        {
            blocks_[new_index++] = current_packed;
        }

        // Resize the blocks_ to discard unused space
        blocks_.resize(new_index);
    }

    /**
     * Getters
     */

    FlexibleIndexVectorDataType type() const { return type_; }
    IndexType size() const { return size_; }
    const cista::basic_vector<IndexType, Ptr>& blocks() const { return blocks_; }

    /**
     * Members
     */

    FlexibleIndexVectorDataType type_;
    IndexType size_;
    cista::basic_vector<IndexType, Ptr> blocks_ {};
};

template<std::unsigned_integral IndexType, template<typename> typename Ptr>
constexpr bool operator==(const basic_flexible_index_vector<IndexType, Ptr>& lhs, const basic_flexible_index_vector<IndexType, Ptr>& rhs)
{
    return (lhs.type() == rhs.type()) && (lhs.size() == rhs.size()) && (lhs.blocks() == rhs.blocks());
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
