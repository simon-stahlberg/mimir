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
template<typename Hash = std::hash<Slot>, typename EqualTo = std::equal_to<Slot>, size_t BucketSize = 8>
class TreeDatabase
{
private:
    static_assert(BucketSize != 0 && (BucketSize & (BucketSize - 1)) == 0 && "BucketSize must be a power of two.");
    static_assert(BucketSize <= 128 && "Bucket size must fit into uint8_t");

    static constexpr Index INDEX_SENTINEL = std::numeric_limits<Index>::max();  ///< used to indicate insertion failure to trigger a rehash.

    static constexpr double MAX_LOAD_FACTOR = 0.7;

    IndexedHashSet m_roots;

    std::vector<Slot> m_bucket_data;
    std::vector<uint8_t> m_bucket_sizes;
    size_t m_num_buckets;
    size_t m_size;
    size_t m_capacity;

    Hash m_hash;
    EqualTo m_equal_to;

    struct Statistics
    {
        size_t m_num_rehashes = 0;
        size_t m_max_num_subsequent_rehashes = 1;
        std::chrono::milliseconds m_total_rehash_time = std::chrono::milliseconds::zero();
    };

    Statistics m_statistics;

    struct RehashData
    {
        size_t num_buckets;
        size_t capacity;
        IndexedHashSet roots;
        std::vector<Slot> bucket_data;
        std::vector<uint8_t> bucket_sizes;

        RehashData(size_t num_buckets, size_t capacity) :
            num_buckets(num_buckets),
            capacity(capacity),
            roots(),
            bucket_data(capacity),
            bucket_sizes(num_buckets, 0)
        {
        }
    };

    Index insert(Slot slot, RehashData& tmp)
    {
        // The Power of Two Choices in Randomized Load Balancing:
        // https://www.eecs.harvard.edu/~michaelm/postscripts/mythesis.pdf?utm_source=chatgpt.com
        size_t h = m_hash(slot);
        size_t h1 = h % tmp.num_buckets;
        size_t h2 = std::rotl(h, 23) % tmp.num_buckets;
        size_t offset1 = BucketSize * h1;
        size_t offset2 = BucketSize * h2;

        for (size_t i = 0; i < tmp.bucket_sizes[h1]; ++i)
        {
            Index unstable_index = offset1 + i;

            if (m_equal_to(tmp.bucket_data[unstable_index], slot))
                return unstable_index;
        }

        for (size_t i = 0; i < tmp.bucket_sizes[h2]; ++i)
        {
            Index unstable_index = offset2 + i;

            if (m_equal_to(tmp.bucket_data[unstable_index], slot))
                return unstable_index;
        }

        if (tmp.bucket_sizes[h1] > tmp.bucket_sizes[h2])
        {
            std::swap(h1, h2);
            std::swap(offset1, offset2);
        }

        if (tmp.bucket_sizes[h1] == BucketSize)
        {
            assert(tmp.bucket_sizes[h2] == BucketSize);
            return INDEX_SENTINEL;
        }

        Index unstable_index = offset1 + tmp.bucket_sizes[h1]++;

        tmp.bucket_data[unstable_index] = slot;

        return unstable_index;
    }

    Index rehash_recursively(Index unstable_index, size_t size, RehashData& tmp)
    {
        /* Base case 1: skipped node creation */
        if (size == 1)
            return unstable_index;

        /* Note: caching relocation is expensive to cache because the tree structure depends on size. */

        const auto& slot = m_bucket_data[unstable_index];

        /* Base case 3: rellocate slot */
        if (size == 2)
        {
            return insert(slot, tmp);
        }

        /* Divide */
        assert(size >= 2);
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        Index i1 = rehash_recursively(slot.i1, mid, tmp);
        if (i1 == INDEX_SENTINEL)
            return i1;
        Index i2 = rehash_recursively(slot.i2, size - mid, tmp);
        if (i2 == INDEX_SENTINEL)
            return i2;

        return insert(Slot(i1, i2), tmp);
    }

    void rehash(double factor = 2.)
    {
        using clock = std::chrono::high_resolution_clock;

        size_t num_subsequent_rehashes = 0;

        auto start = clock::now();  // Start timing

        while (true)
        {
            ++num_subsequent_rehashes;
            ++m_statistics.m_num_rehashes;

            std::cout << "Start rehash with load factor: " << load_factor() << std::endl;
            size_t new_num_buckets = factor * m_num_buckets;
            size_t new_capacity = factor * m_capacity;

            auto tmp = RehashData(new_num_buckets, new_capacity);

            bool rehash_success = true;

            // Relocate empty root
            tmp.roots.insert(Slot(0, 0));

            // Relocate remaining roots
            for (Index stable_index = 1; stable_index < m_roots.size(); ++stable_index)
            {
                const Slot& root = m_roots[stable_index];

                Index unstable_index = rehash_recursively(root.i1, root.i2, tmp);

                if (unstable_index == INDEX_SENTINEL)
                {
                    rehash_success = false;
                    break;
                }

                tmp.roots.insert(Slot(unstable_index, root.i2));
            }

            if (!rehash_success)
            {
                factor *= 2;
                continue;
            }
            m_statistics.m_max_num_subsequent_rehashes = std::max(m_statistics.m_max_num_subsequent_rehashes, num_subsequent_rehashes);

            m_num_buckets = new_num_buckets;
            m_capacity = new_capacity;
            std::swap(m_roots, tmp.roots);
            std::swap(m_bucket_data, tmp.bucket_data);
            std::swap(m_bucket_sizes, tmp.bucket_sizes);
            auto end = clock::now();
            m_statistics.m_total_rehash_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Finish rehash with load factor: " << load_factor() << std::endl;
            return;
        }
    }

    Index insert(Slot slot)
    {
        // if (load_factor() >= MAX_LOAD_FACTOR)
        //     return INDEX_SENTINEL;

        // The Power of Two Choices in Randomized Load Balancing:
        // https://www.eecs.harvard.edu/~michaelm/postscripts/mythesis.pdf?utm_source=chatgpt.com
        size_t h = m_hash(slot);
        size_t h1 = h % m_num_buckets;
        size_t h2 = std::rotl(h, 23) % m_num_buckets;
        size_t offset1 = BucketSize * h1;
        size_t offset2 = BucketSize * h2;

        for (size_t i = 0; i < m_bucket_sizes[h1]; ++i)
        {
            Index unstable_index = offset1 + i;

            if (m_equal_to(m_bucket_data[unstable_index], slot))
                return unstable_index;
        }

        for (size_t i = 0; i < m_bucket_sizes[h2]; ++i)
        {
            Index unstable_index = offset2 + i;

            if (m_equal_to(m_bucket_data[unstable_index], slot))
                return unstable_index;
        }

        if (m_bucket_sizes[h1] > m_bucket_sizes[h2])
        {
            std::swap(h1, h2);
            std::swap(offset1, offset2);
        }

        if (m_bucket_sizes[h1] == BucketSize)
        {
            assert(m_bucket_sizes[h2] == BucketSize);
            return INDEX_SENTINEL;
        }

        Index unstable_index = offset1 + m_bucket_sizes[h1]++;

        m_bucket_data[unstable_index] = slot;
        ++m_size;

        return unstable_index;
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
        if (i1 == INDEX_SENTINEL)
            return i1;
        Index i2 = insert_recursively(mid_it, end, size - mid);
        if (i2 == INDEX_SENTINEL)
            return i2;

        return insert(Slot(i1, i2));
    }

public:
    TreeDatabase(size_t num_buckets = 1024) :
        m_roots(),
        m_bucket_data(),
        m_bucket_sizes(),
        m_num_buckets(num_buckets),
        m_size(0),
        m_capacity(num_buckets * BucketSize),
        m_hash(),
        m_equal_to()
    {
        m_bucket_data.resize(m_capacity);
        m_bucket_sizes.resize(m_num_buckets, 0);

        m_roots.insert(Slot(0, 0));
    }

    template<std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, Index>
    Index insert(const Range& range)
    {
        assert(std::is_sorted(range.begin(), range.end()));

        // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
        const auto size = static_cast<Index>(std::distance(range.begin(), range.end()));

        if (size == 0)  ///< Special case for empty range.
            return 0;   ///< 0 marks the empty range.

        double factor = 1.0;

        while (true)
        {
            Index unstable_index = insert_recursively(range.begin(), range.end(), size);

            if (unstable_index == INDEX_SENTINEL)
            {
                factor *= 2;
                rehash(factor);
                continue;
            }

            return m_roots.insert(Slot(unstable_index, size)).first->second;
        }
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

                const auto slot = db().m_bucket_data[entry.m_index];

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

            if (begin)
            {
                m_stack = s_stack_pool.get_or_allocate();
                m_stack->clear();

                const auto& root = db.m_roots[stable_index];

                if (root.i2 > 0)  ///< Push to stack iff there are elements.
                {
                    m_stack->emplace_back(root.i1, root.i2);
                    advance();
                }
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

    size_t num_roots() const { return m_roots.size(); }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t num_buckets() const { return m_num_buckets; }
    double load_factor() const { return static_cast<double>(m_size) / m_capacity; };
    const Statistics& statistics() const { return m_statistics; }
};
}

#endif