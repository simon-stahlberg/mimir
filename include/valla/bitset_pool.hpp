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
    uint32_t m_segment;
    uint32_t m_offset;

public:
    BitsetView() : m_segment(-1), m_offset(-1) {}
    BitsetView(uint32_t segment, uint32_t offset);

    bool get(size_t bit, const BitsetPool& pool) const;

    void set(size_t bit, BitsetPool& pool);

    uint32_t get_segment() const;
    uint32_t get_offset() const;
};

static_assert(sizeof(BitsetView) == 8);

class BitsetConstView
{
private:
    uint32_t m_segment;
    uint32_t m_offset;

public:
    BitsetConstView() : m_segment(-1), m_offset(-1) {}
    BitsetConstView(uint32_t segment, uint32_t offset);

    /// @brief Implicit conversion from mutable BitsetView.
    /// @param view
    BitsetConstView(BitsetView view);

    bool get(size_t bit, const BitsetPool& pool) const;

    uint32_t get_segment() const;
    uint32_t get_offset() const;
};

static_assert(sizeof(BitsetView) == 8);

class BitsetPool
{
private:
    std::vector<std::vector<uint64_t>> m_segments;
    size_t m_segment;
    size_t m_offset;

    static constexpr const size_t MAX_SEGMENT_SIZE = std::numeric_limits<uint32_t>::max() >> 6;

    static constexpr const size_t INITIAL_SEGMENT_SIZE = 1024;

    void resize_to_fit(size_t num_bits);

public:
    BitsetPool();

    BitsetView allocate(size_t num_bits);

    std::vector<uint64_t>& get_segment(size_t segment);

    const std::vector<uint64_t>& get_segment(size_t segment) const;
};

/**
 * BitsetView
 */

inline BitsetView::BitsetView(uint32_t segment, uint32_t offset) : m_segment(segment), m_offset(offset) {}

inline bool BitsetView::get(size_t bit, const BitsetPool& pool) const
{
    const auto& segment = pool.get_segment(m_segment);

    size_t offset = m_offset + bit;
    size_t block_index = offset / 64;
    size_t bit_index = offset % 64;

    return segment[block_index] & (uint64_t(1) << bit_index);
}

inline void BitsetView::set(size_t bit, BitsetPool& pool)
{
    auto& segment = pool.get_segment(m_segment);

    size_t offset = m_offset + bit;
    size_t block_index = offset / 64;
    size_t bit_index = offset % 64;

    segment[block_index] |= (uint64_t(1) << bit_index);
}

inline uint32_t BitsetView::get_segment() const { return m_segment; }

inline uint32_t BitsetView::get_offset() const { return m_offset; }

/**
 * BitsetConstView
 */

inline BitsetConstView::BitsetConstView(uint32_t segment, uint32_t offset) : m_segment(segment), m_offset(offset) {}

inline BitsetConstView::BitsetConstView(BitsetView view) : m_segment(view.get_segment()), m_offset(view.get_offset()) {}

inline bool BitsetConstView::get(size_t bit, const BitsetPool& pool) const
{
    const auto& segment = pool.get_segment(m_segment);

    size_t offset = m_offset + bit;
    size_t block_index = offset / 64;
    size_t bit_index = offset % 64;

    return segment[block_index] & (uint64_t(1) << bit_index);
}

inline uint32_t BitsetConstView::get_segment() const { return m_segment; }

inline uint32_t BitsetConstView::get_offset() const { return m_offset; }

/**
 * BitsetPool
 */

inline void BitsetPool::resize_to_fit(size_t num_bits)
{
    const auto remaining_bits = (m_segments.back().size() * 64) - m_offset;

    if (remaining_bits < num_bits)
    {
        const auto needed_blocks = (num_bits + 63) / 64;
        const auto new_segment_size = std::min(std::max(m_segments.back().size() * 2, needed_blocks), MAX_SEGMENT_SIZE);

        m_segments.push_back(std::vector<uint64_t>(new_segment_size, uint64_t(0)));
        ++m_segment;
        m_offset = 0;
    }
}

inline BitsetPool::BitsetPool() : m_segments(1, std::vector<uint64_t>(INITIAL_SEGMENT_SIZE, uint64_t(0))), m_segment(0), m_offset(0) {}

inline BitsetView BitsetPool::allocate(size_t num_bits)
{
    resize_to_fit(num_bits);

    auto view = BitsetView(m_segment, m_offset);
    m_offset += num_bits;

    return view;
}

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
