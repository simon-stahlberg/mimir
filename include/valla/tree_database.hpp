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

#ifndef VALLA_INCLUDE_TREE_DATABASE_HPP_
#define VALLA_INCLUDE_TREE_DATABASE_HPP_

#include "valla/declarations.hpp"
#include "valla/details/unique_object_pool.hpp"
#include "valla/indexed_hash_set.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <immintrin.h>  // for SSE2, AVX2, etc.
#include <vector>

namespace valla::tdb
{

struct Entry
{
    Index m_index;
    Index m_size;

    Entry(Index index, Index size) : m_index(index), m_size(size) {}
};

inline void copy(const std::vector<Entry>& src, std::vector<Entry>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

static thread_local UniqueObjectPool<std::vector<Entry>> s_stack_pool = UniqueObjectPool<std::vector<Entry>> {};

/**
 * `TreeDatabase` implements a tree databse as a hash id map with open addressing and bounded bucket size, i.e.,
 * a mapping from slots to implicit integer ids where implicit means that the ids are assigned internally by the data structure itself.
 * `TreeDatabase` saves 250% memory for internal nodes compared to the naive implementation of tree compression.
 * It follows an analysis of compact hashing.
 *
 * Compact hashing explained and an example showing its low benefit.
 *
 * Use a bijective hash function h : uint64_t -> uint64_t.
 * For a key k, compute h(k), and define:
 *   - i = h(k) % m          // the initial bucket index
 *   - q = h(k) / m          // the quotient to store
 *
 * Instead of storing the full key k (64 bits), the hash table stores only q.
 * During lookup or insertion, h(k) can be reconstructed as:
 *      h(k) = q * m + i
 * and the original key can be recovered via:
 *      k = h⁻¹(h(k))
 *
 * This reduces storage per entry from 64 bits (or 2*64+32=160 bits for the naive implementation) to ceil(log2(q)) bits.
 *
 * Example:
 * Consider a tree database with:
 *   - total_slots = 2^24 (~17M entries)
 *   - bucket_size = 2^8 (256 entries per bucket)
 *   - load_factor = 0.5
 *
 * Then the number of buckets is:
 *     m = total_slots / (bucket_size * (1 / load_factor)
 *       = 2^24 / (2^8 * (1 / 0.5))
 *       = 2^24 / 2^9
 *       = 2^15
 *
 * Since h(k) is 64 bits, the maximum possible value of q is:
 *     q_max = 2^64 / m = 2^64 / 2^15 = 2^49
 *
 * So the stored quotient fits in 49 bits. This is a ~24% reduction from 64 bits,
 * and storage shrinks further as m increases (i.e., as the table grows).
 */
template<typename Hash = std::hash<Slot>, typename EqualTo = std::equal_to<Slot>, size_t InitialCapacity = 1024>
class TreeDatabase
{
private:
    static_assert(InitialCapacity % 16 == 0, "InitialCapacity must be a multiple of 16.");

    static constexpr Index INDEX_SENTINEL = std::numeric_limits<Index>::max();  ///< used to indicate insertion failure to trigger a rehash.

    static constexpr double MAX_LOAD_FACTOR = static_cast<double>(7) / 8;

    enum class ctrl_t : int8_t
    {
        kEmpty = -128,   // 0b10000000
        kDeleted = -2,   // 0b11111110
        kSentinel = -1,  // 0b11111111
    };

    std::vector<Index> m_stable_to_unstable;
    absl::flat_hash_map<Index, Index> m_unstable_to_stable;

    std::vector<Slot> m_slots;
    std::vector<ctrl_t> m_controls;
    size_t m_size;
    size_t m_capacity;

    Hash m_hash;
    EqualTo m_equal_to;

    struct Statistics
    {
        size_t m_num_rehashes = 0;
        std::chrono::milliseconds m_total_rehash_time = std::chrono::milliseconds::zero();
        size_t m_num_probes = 0;
        size_t m_sum_probe_lengths = 0;
    };

    Statistics m_statistics;

    struct RehashData
    {
        size_t capacity;
        std::vector<Slot> slots;
        std::vector<ctrl_t> controls;

        explicit RehashData(size_t capacity) : capacity(capacity), slots(capacity), controls()
        {
            // Sentinel-padded rolling buffer
            controls.reserve(capacity + 15);
            controls.resize(capacity, ctrl_t::kEmpty);
            controls.resize(capacity + 15, ctrl_t::kSentinel);
        }
    };

    alignas(16) inline static const __m128i kEmptyPattern = _mm_set1_epi8(static_cast<signed char>(ctrl_t::kEmpty));

    Index insert(Slot slot, RehashData& tmp)
    {
        size_t h = m_hash(slot);
        size_t mask = (tmp.capacity - 1);
        size_t i = h & mask;
        ctrl_t ctrl = static_cast<ctrl_t>(h >> 57);
        assert(static_cast<int8_t>(ctrl) >= 0);

        while (true)
        {
            assert(i < tmp.capacity);
            assert(i + 15 < tmp.controls.size());

            // Load 16 control bytes
            __m128i ctrl_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&tmp.controls[i]));
            __m128i broadcast_ctrl = _mm_set1_epi8(static_cast<signed char>(ctrl));

            // Compare against ctrl byte
            __m128i cmp_ctrl = _mm_cmpeq_epi8(ctrl_block, broadcast_ctrl);
            int mask_ctrl = _mm_movemask_epi8(cmp_ctrl);

            // Check if slot exists
            while (mask_ctrl != 0)
            {
                int offset = __builtin_ctz(mask_ctrl);
                size_t idx = (i + offset) & mask;

                if (m_equal_to(tmp.slots[idx], slot))
                {
                    m_statistics.m_sum_probe_lengths += offset;
                    return idx;
                }

                mask_ctrl &= mask_ctrl - 1;  // Clear the lowest set bit
            }

            // Compare against kEmpty
            __m128i cmp_empty = _mm_cmpeq_epi8(ctrl_block, kEmptyPattern);
            int mask_empty = _mm_movemask_epi8(cmp_empty);

            // Second: insert into first empty slot if found
            if (mask_empty != 0)
            {
                int offset = __builtin_ctz(mask_empty);
                m_statistics.m_sum_probe_lengths += offset;
                size_t idx = (i + offset) & mask;

                if (tmp.controls[idx] == ctrl_t::kEmpty)
                {
                    assert(tmp.controls[idx] == ctrl_t::kEmpty && "Unexpected overwrite!");

                    tmp.slots[idx] = slot;
                    tmp.controls[idx] = ctrl;
                    ++m_statistics.m_num_probes;
                    return idx;
                }
            }

            // Else probe further
            i = (i + 16) & mask;
            m_statistics.m_sum_probe_lengths += 16;
        }
    }

    Index rehash_recursively(Index unstable_index, size_t size, RehashData& tmp)
    {
        /* Base case 1: skipped node creation */
        if (size == 1)
            return unstable_index;

        /* Note: caching relocation is expensive to cache because the tree structure depends on size. */

        const auto& slot = m_slots[unstable_index];

        /* Base case 3: rellocate slot */
        if (size == 2)
            return insert(slot, tmp);

        /* Divide */
        assert(size >= 2);
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        Index i1 = rehash_recursively(slot.i1, mid, tmp);
        Index i2 = rehash_recursively(slot.i2, size - mid, tmp);

        return insert(Slot(i1, i2), tmp);
    }

    void rehash(double factor = 2.)
    {
        using clock = std::chrono::high_resolution_clock;

        auto start = clock::now();  // Start timing

        ++m_statistics.m_num_rehashes;

        size_t new_capacity = factor * m_capacity;

        auto tmp = RehashData(new_capacity);

        // Relocate trees underlying the stable indices
        m_unstable_to_stable.clear();

        // Skip the empty root.
        for (Index stable_index = 1; stable_index < m_stable_to_unstable.size(); ++stable_index)
        {
            Index unstable_index = m_stable_to_unstable[stable_index];

            const Slot& root = m_slots[unstable_index];

            assert(root.i2 > 0);  // Ensure nonempty.

            unstable_index = rehash_recursively(root.i1, root.i2, tmp);
            unstable_index = insert(Slot(unstable_index, root.i2), tmp);

            m_unstable_to_stable.emplace(unstable_index, stable_index);
            m_stable_to_unstable[stable_index] = unstable_index;
        }

        m_capacity = new_capacity;
        std::swap(m_slots, tmp.slots);
        std::swap(m_controls, tmp.controls);

        auto end = clock::now();
        m_statistics.m_total_rehash_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }

    Index insert(Slot slot)
    {
        size_t h = m_hash(slot);
        size_t mask = (m_capacity - 1);
        size_t i = h & mask;
        ctrl_t ctrl = static_cast<ctrl_t>(h >> 57);
        assert(static_cast<int8_t>(ctrl) >= 0);

        while (true)
        {
            assert(i < m_capacity);
            assert(i + 15 < m_controls.size());

            // Load 16 control bytes
            __m128i ctrl_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&m_controls[i]));
            __m128i broadcast_ctrl = _mm_set1_epi8(static_cast<signed char>(ctrl));

            // Compare against ctrl byte
            __m128i cmp_ctrl = _mm_cmpeq_epi8(ctrl_block, broadcast_ctrl);
            int mask_ctrl = _mm_movemask_epi8(cmp_ctrl);

            // Check if slot exists
            while (mask_ctrl != 0)
            {
                int offset = __builtin_ctz(mask_ctrl);
                size_t idx = (i + offset) & mask;

                if (m_equal_to(m_slots[idx], slot))
                {
                    m_statistics.m_sum_probe_lengths += offset;
                    return idx;
                }

                mask_ctrl &= mask_ctrl - 1;  // Clear the lowest set bit
            }

            // Compare against kEmpty
            __m128i cmp_empty = _mm_cmpeq_epi8(ctrl_block, kEmptyPattern);
            int mask_empty = _mm_movemask_epi8(cmp_empty);

            // Second: insert into first empty slot if found
            if (mask_empty != 0)
            {
                int offset = __builtin_ctz(mask_empty);
                m_statistics.m_sum_probe_lengths += offset;
                size_t idx = (i + offset) & mask;

                if (m_controls[idx] == ctrl_t::kEmpty)
                {
                    assert(m_controls[idx] == ctrl_t::kEmpty && "Unexpected overwrite!");

                    m_slots[idx] = slot;
                    m_controls[idx] = ctrl;
                    ++m_size;
                    ++m_statistics.m_num_probes;

                    return idx;
                }
            }

            // Else probe further
            i = (i + 16) & mask;
            m_statistics.m_sum_probe_lengths += 16;
        }
    }

    template<std::input_iterator Iterator>
        requires std::same_as<std::iter_value_t<Iterator>, Index>
    inline Index insert_recursively(Iterator it, Iterator end, size_t size)
    {
        /* Base cases */
        if (size == 1)
            return *it;  ///< Skip node creation

        if (size == 2)
            return insert(Slot(*it, *(it + 1)));

        /* Divide */
        assert(size >= 2);
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        const auto mid_it = it + mid;
        Index i1 = insert_recursively(it, mid_it, mid);
        Index i2 = insert_recursively(mid_it, end, size - mid);

        return insert(Slot(i1, i2));
    }

public:
    TreeDatabase() : m_stable_to_unstable(), m_unstable_to_stable(), m_slots(), m_controls(), m_size(0), m_capacity(InitialCapacity), m_hash(), m_equal_to()
    {
        m_slots.resize(m_capacity);

        // Sentinel-padded rolling buffer
        m_controls.reserve(m_capacity + 15);
        m_controls.resize(m_capacity, ctrl_t::kEmpty);
        m_controls.resize(m_capacity + 15, ctrl_t::kSentinel);

        m_stable_to_unstable.push_back(0);  // dummy
    }

    template<std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, Index>
    Index insert(const Range& range)
    {
        assert(std::is_sorted(range.begin(), range.end()));

        // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
        const auto size = static_cast<Index>(std::distance(range.begin(), range.end()));

        if (size == 0)  ///< Special case for empty range.
            return 0;

        if ((static_cast<double>(m_size + 2 * size) / m_capacity) >= MAX_LOAD_FACTOR)
            rehash(2.0);

        Index unstable_index = insert(Slot(insert_recursively(range.begin(), range.end(), size), size));

        auto result = m_unstable_to_stable.emplace(unstable_index, m_stable_to_unstable.size());
        if (result.second)
            m_stable_to_unstable.push_back(unstable_index);

        return result.first->second;
    }

    /**
     * ConstIterator
     */

    class const_iterator
    {
    private:
        const TreeDatabase* m_db;
        UniqueObjectPoolPtr<std::vector<Entry>> m_stack;
        Index m_value;

        static constexpr const Index END_POS = Index(-1);

        const TreeDatabase& db() const
        {
            assert(m_db);
            return *m_db;
        }

        void advance()
        {
            while (!m_stack->empty())
            {
                auto entry = m_stack->back();
                m_stack->pop_back();

                if (entry.m_size == 1)
                {
                    m_value = entry.m_index;
                    return;
                }

                const auto slot = db().m_slots[entry.m_index];

                assert(entry.m_size >= 0);
                Index mid = std::bit_floor(entry.m_size - 1);

                // Emplace i2 first to ensure i1 is visited first in dfs.
                m_stack->emplace_back(slot.i2, entry.m_size - mid);
                m_stack->emplace_back(slot.i1, mid);
            }

            m_value = END_POS;
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = Index;
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;

        const_iterator() : m_db(nullptr), m_stack(), m_value(END_POS) {}
        const_iterator(const const_iterator& other) : m_db(other.m_db), m_stack(other.m_stack.clone()), m_value(other.m_value) {}
        const_iterator& operator=(const const_iterator& other)
        {
            if (*this != other)
            {
                m_db = other.m_db;
                m_stack = other.m_stack.clone();
                m_value = other.m_value;
            }
            return *this;
        }
        const_iterator(const_iterator&& other) = default;
        const_iterator& operator=(const_iterator&& other) = default;
        const_iterator(const TreeDatabase& db, Index stable_index, bool begin) : m_db(&db), m_stack(), m_value(END_POS)
        {
            assert(m_db);

            if (begin && stable_index > 0)
            {
                m_stack = s_stack_pool.get_or_allocate();
                m_stack->clear();

                Index unstable_index = db.m_stable_to_unstable[stable_index];

                const auto& root = db.m_slots[unstable_index];

                assert(root.i2 > 0);  // Ensure nonempty

                m_stack->emplace_back(root.i1, root.i2);
                advance();
            }
        }
        value_type operator*() const { return m_value; }
        const_iterator& operator++()
        {
            advance();
            return *this;
        }
        const_iterator operator++(int)
        {
            auto it = *this;
            ++it;
            return it;
        }
        bool operator==(const const_iterator& other) const { return m_value == other.m_value; }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    const_iterator begin(Index stable_index) const { return const_iterator(*this, stable_index, true); }
    const_iterator end() const { return const_iterator(); }

    Index empty_stable_index() const { return 0; }
    size_t num_roots() const { return m_stable_to_unstable.size(); }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    double load_factor() const { return static_cast<double>(m_size) / m_capacity; };
    const Statistics& statistics() const { return m_statistics; }
};
}

#endif
