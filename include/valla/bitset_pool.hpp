/*
 * Copyright (C) 2025 Dominik Drexler
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

#ifndef VALLA_INCLUDE_BITSET_POOL_HPP_
#define VALLA_INCLUDE_BITSET_POOL_HPP_

#include "valla/declarations.hpp"

#include <absl/container/flat_hash_set.h>
#include <bitset>
#include <limits>
#include <vector>

namespace valla
{
class BitsetPool;
class BitsetView;
class BitsetConstView;

class BitsetView
{
private:
    uint64_t* m_blocks;
    size_t m_num_blocks;

public:
    BitsetView();
    BitsetView(uint64_t* blocks, size_t num_blocks);

    bool get(size_t bit) const;

    void set(size_t bit);

    uint64_t* get_blocks() const;
    size_t get_num_blocks() const;
};

static_assert(sizeof(BitsetView) == 16);

class BitsetConstView
{
private:
    const uint64_t* m_blocks;
    size_t m_num_blocks;

public:
    BitsetConstView();
    BitsetConstView(const uint64_t* blocks, size_t num_blocks);

    /// @brief Implicit conversion from mutable BitsetView.
    /// @param view
    BitsetConstView(BitsetView view);

    bool get(size_t bit) const;

    const uint64_t* get_blocks() const;
    size_t get_num_blocks() const;
};

static_assert(sizeof(BitsetView) == 16);

struct BitsetHash
{
private:
    size_t m_num_blocks;

public:
    size_t operator()(BitsetView el) const;

    size_t operator()(BitsetConstView el) const;

    template<typename Bitset>
    size_t hash(Bitset el) const;
};

struct BitsetEqualTo
{
private:
    size_t m_common_num_blocks;

public:
    bool operator()(BitsetView lhs, BitsetView rhs) const;

    bool operator()(BitsetConstView lhs, BitsetConstView rhs) const;

    template<typename Bitset>
    size_t equal_to(Bitset lhs, Bitset rhs) const;
};

class BitsetPool
{
private:
    std::vector<std::vector<uint64_t>> m_segments;
    size_t m_segment;
    size_t m_offset;

    size_t m_last_allocated_num_blocks;

    absl::flat_hash_set<BitsetConstView, BitsetHash, BitsetEqualTo> m_uniqueness;

    static constexpr const size_t MAX_SEGMENT_SIZE = std::numeric_limits<uint32_t>::max() >> 6;

    static constexpr const size_t INITIAL_SEGMENT_SIZE = 1024;

    void resize_to_fit(size_t num_blocks);

public:
    BitsetPool();

    BitsetView allocate(size_t num_blocks);

    void pop_allocation();

    auto insert(BitsetConstView view);

    std::vector<uint64_t>& get_segment(size_t segment);

    const std::vector<uint64_t>& get_segment(size_t segment) const;
};

/**
 * BitsetView
 */

inline BitsetView::BitsetView() : m_blocks(nullptr), m_num_blocks(0) {}

inline BitsetView::BitsetView(uint64_t* blocks, size_t num_blocks) : m_blocks(blocks), m_num_blocks(num_blocks) {}

inline bool BitsetView::get(size_t bit) const
{
    size_t block_index = bit / 64;
    size_t bit_index = bit % 64;

    return m_blocks[block_index] & (uint64_t(1) << bit_index);
}

inline void BitsetView::set(size_t bit)
{
    size_t block_index = bit / 64;
    size_t bit_index = bit % 64;

    m_blocks[block_index] |= (uint64_t(1) << bit_index);
}

inline uint64_t* BitsetView::get_blocks() const { return m_blocks; }

inline size_t BitsetView::get_num_blocks() const { return m_num_blocks; }

/**
 * BitsetConstView
 */

inline BitsetConstView::BitsetConstView() : m_blocks(nullptr), m_num_blocks(0) {}

inline BitsetConstView::BitsetConstView(const uint64_t* blocks, size_t num_blocks) : m_blocks(blocks), m_num_blocks(num_blocks) {}

inline BitsetConstView::BitsetConstView(BitsetView view) : m_blocks(view.get_blocks()), m_num_blocks(view.get_num_blocks()) {}

inline bool BitsetConstView::get(size_t bit) const
{
    size_t block_index = bit / 64;
    size_t bit_index = bit % 64;

    return m_blocks[block_index] & (uint64_t(1) << bit_index);
}

inline const uint64_t* BitsetConstView::get_blocks() const { return m_blocks; }

inline size_t BitsetConstView::get_num_blocks() const { return m_num_blocks; }

/**
 * BitsetHash
 */

inline size_t BitsetHash::operator()(BitsetView el) const { return hash(el); }

inline size_t BitsetHash::operator()(BitsetConstView el) const { return hash(el); }

template<typename Bitset>
size_t BitsetHash::hash(Bitset el) const
{
    size_t seed = el.get_num_blocks();
    for (size_t i = 0; i < el.get_num_blocks(); ++i)
    {
        valla::hash_combine(seed, el.get_blocks()[i]);
    }
    return seed;
}

/**
 * BitsetEqualTo
 */

inline bool BitsetEqualTo::operator()(BitsetView lhs, BitsetView rhs) const { return equal_to(lhs, rhs); }

inline bool BitsetEqualTo::operator()(BitsetConstView lhs, BitsetConstView rhs) const { return equal_to(lhs, rhs); }

template<typename Bitset>
size_t BitsetEqualTo::equal_to(Bitset lhs, Bitset rhs) const
{
    if (lhs.get_num_blocks() != rhs.get_num_blocks())
        return false;
    return std::equal(lhs.get_blocks(), lhs.get_blocks() + lhs.get_num_blocks(), rhs.get_blocks(), rhs.get_blocks() + rhs.get_num_blocks());
}

/**
 * BitsetPool
 */

inline void BitsetPool::resize_to_fit(size_t num_blocks)
{
    const auto remaining_blocks = m_segments.back().size() - m_offset;

    if (remaining_blocks < num_blocks)
    {
        const auto new_segment_size = std::max(m_segments.back().size() * 2, num_blocks);

        m_segments.push_back(std::vector<uint64_t>(new_segment_size, uint64_t(0)));
        ++m_segment;
        m_offset = 0;
    }
}

inline BitsetPool::BitsetPool() :
    m_segments(1, std::vector<uint64_t>(INITIAL_SEGMENT_SIZE, uint64_t(0))),
    m_segment(0),
    m_offset(0),
    m_last_allocated_num_blocks(0)
{
}

inline BitsetView BitsetPool::allocate(size_t num_blocks)
{
    resize_to_fit(num_blocks);

    auto view = BitsetView(m_segments.back().data() + m_offset, num_blocks);
    m_offset += num_blocks;
    m_last_allocated_num_blocks = num_blocks;

    return view;
}

inline void BitsetPool::pop_allocation()
{
    m_offset -= m_last_allocated_num_blocks;
    m_last_allocated_num_blocks = 0;
}

inline auto BitsetPool::insert(BitsetConstView view) { return m_uniqueness.insert(view); }

inline std::vector<uint64_t>& BitsetPool::get_segment(size_t segment)
{
    assert(segment < m_segments.size());
    return m_segments[segment];
}

inline const std::vector<uint64_t>& BitsetPool::get_segment(size_t segment) const
{
    assert(segment < m_segments.size());
    return m_segments[segment];
}
}

#endif
