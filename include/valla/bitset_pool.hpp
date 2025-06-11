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
class Bitset;

class Bitset
{
private:
    uint64_t* m_blocks;
    uint32_t m_num_bits;
    Index m_index;

public:
    Bitset();
    Bitset(uint64_t* blocks, uint32_t num_bits, Index index);

    bool get(size_t bit) const;

    void set(size_t bit);

    uint64_t* get_blocks() const;
    uint32_t get_num_bits() const;
    uint32_t get_num_blocks() const;
    Index get_index() const;
};

static_assert(sizeof(Bitset) == 16);
static_assert(sizeof(Bitset*) == 8);

struct BitsetHash
{
    size_t operator()(const Bitset& el) const;
};

struct BitsetEqualTo
{
    bool operator()(const Bitset& lhs, const Bitset& rhs) const;
};

struct DerefBitsetHash
{
    size_t operator()(const Bitset* el) const;
};

struct DerefBitsetEqualTo
{
    bool operator()(const Bitset* lhs, const Bitset* rhs) const;
};

class BitsetPool
{
private:
    std::vector<std::vector<uint64_t>> m_segments;
    size_t m_segment;
    size_t m_offset;
    size_t m_size;

    size_t m_last_allocated_num_blocks;

    static constexpr const size_t INITIAL_SEGMENT_SIZE = 1024;

    void resize_to_fit(size_t num_blocks);

public:
    BitsetPool();

    Bitset allocate(uint32_t num_bits);

    void pop_allocation();

    std::vector<uint64_t>& get_segment(size_t segment);

    const std::vector<uint64_t>& get_segment(size_t segment) const;

    size_t size() const;
};

class BitsetRepository
{
private:
    std::vector<std::vector<Bitset>> m_segments;
    size_t m_segment;
    size_t m_offset;
    size_t m_size;
    size_t m_capacity;

    absl::flat_hash_set<const Bitset*, DerefBitsetHash, DerefBitsetEqualTo> m_uniqueness;

    static constexpr const size_t INITIAL_SEGMENT_SIZE = 1024;

    void resize_to_fit();

    const Bitset* m_empty_bitset;

public:
    explicit BitsetRepository(BitsetPool& pool);

    const Bitset* get_empty_bitset() const;

    auto insert(Bitset bitset);

    size_t size() const;
};

/**
 * Bitset
 */

inline Bitset::Bitset() : m_blocks(nullptr), m_num_bits(0), m_index(0) {}

inline Bitset::Bitset(uint64_t* blocks, uint32_t num_bits, Index index) : m_blocks(blocks), m_num_bits(num_bits), m_index(index) {}

inline bool Bitset::get(size_t bit) const
{
    assert(bit < get_num_bits());

    size_t block_index = bit / 64;
    size_t bit_index = bit % 64;

    return m_blocks[block_index] & (uint64_t(1) << bit_index);
}

inline void Bitset::set(size_t bit)
{
    assert(bit < get_num_bits());

    size_t block_index = bit / 64;
    size_t bit_index = bit % 64;

    m_blocks[block_index] |= (uint64_t(1) << bit_index);
}

inline uint64_t* Bitset::get_blocks() const { return m_blocks; }

inline uint32_t Bitset::get_num_bits() const { return m_num_bits; }

inline uint32_t Bitset::get_num_blocks() const { return (m_num_bits + 63) / 64; }

inline Index Bitset::get_index() const { return m_index; }

/**
 * BitsetHash
 */

inline size_t BitsetHash::operator()(const Bitset& el) const
{
    size_t seed = el.get_num_bits();
    for (size_t i = 0; i < el.get_num_blocks(); ++i)
    {
        valla::hash_combine(seed, el.get_blocks()[i]);
    }
    return seed;
}

/**
 * BitsetEqualTo
 */

inline bool BitsetEqualTo::operator()(const Bitset& lhs, const Bitset& rhs) const
{
    if (lhs.get_num_bits() != rhs.get_num_bits())
        return false;
    return std::equal(lhs.get_blocks(), lhs.get_blocks() + lhs.get_num_blocks(), rhs.get_blocks());
}

/**
 * DerefBitsetHash
 */

inline size_t DerefBitsetHash::operator()(const Bitset* el) const { return BitsetHash {}(*el); }

/**
 * DerefBitsetEqualTo
 */

inline bool DerefBitsetEqualTo::operator()(const Bitset* lhs, const Bitset* rhs) const { return BitsetEqualTo {}(*lhs, *rhs); }

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
    m_size(0),
    m_last_allocated_num_blocks(0)
{
}

inline Bitset BitsetPool::allocate(uint32_t num_bits)
{
    const auto num_blocks = (num_bits + 63) / 64;

    resize_to_fit(num_blocks);

    auto view = Bitset(m_segments.back().data() + m_offset, num_bits, m_size++);
    m_offset += num_blocks;
    m_last_allocated_num_blocks = num_blocks;

    return view;
}

inline void BitsetPool::pop_allocation()
{
    assert(m_offset >= m_last_allocated_num_blocks);

    auto& segment = m_segments.back();
    std::fill(segment.begin() + m_offset - m_last_allocated_num_blocks, segment.begin() + m_offset, uint64_t(0));
    m_offset -= m_last_allocated_num_blocks;
    --m_size;
    m_last_allocated_num_blocks = 0;
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

inline size_t BitsetPool::size() const { return m_size; }

/**
 * BitsetRepository
 */

inline const Bitset* BitsetRepository::get_empty_bitset() const { return m_empty_bitset; }

inline void BitsetRepository::resize_to_fit()
{
    if (m_segments.back().size() == m_offset)
    {
        const auto segment_size = m_segments.back().size() * 2;
        m_segments.push_back(std::vector<Bitset>(segment_size));
        ++m_segment;
        m_offset = 0;
        m_capacity += segment_size;
    }
}

inline auto BitsetRepository::insert(Bitset bitset)
{
    resize_to_fit();

    const auto& element = m_segments[m_segment][m_offset] = bitset;

    const auto result = m_uniqueness.emplace(&element);

    if (result.second)
    {
        ++m_offset;
        ++m_size;
    }

    return result;
}

inline BitsetRepository::BitsetRepository(BitsetPool& pool) :
    m_segments(1, std::vector<Bitset>(INITIAL_SEGMENT_SIZE, Bitset())),
    m_segment(0),
    m_offset(0),
    m_size(0),
    m_capacity(1),
    m_empty_bitset(*insert(pool.allocate(0)).first)
{
}

}

#endif
