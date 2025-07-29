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

#ifndef VALLA_INCLUDE_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_HASH_ID_MAP_HPP_

#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace valla
{

/// @brief `ctrl_t` implements the control byte in a Swiss table.
enum class ctrl_t : int8_t
{
    kEmpty = -128,   // 0b10000000
    kDeleted = -2,   // 0b11111110
    kSentinel = -1,  // 0b11111111
};

inline std::ostream& operator<<(std::ostream& out, const std::vector<ctrl_t>& vec)
{
    out << "[";
    for (const auto x : vec)
    {
        out << static_cast<int32_t>(x) << ", ";
    }
    out << "]";

    return out;
}

alignas(16) inline static const __m128i kEmptyPattern = _mm_set1_epi8(static_cast<signed char>(ctrl_t::kEmpty));

static constexpr double MAX_LOAD_FACTOR = static_cast<double>(7) / 8;

/// @brief `HashIDMap implements a hash ID map with open addressing in a Swiss table format where the position of a key implicitly becomes the index.
/// @tparam Derived is the derived class that must implement the rehash logic in rehash_impl.
/// @tparam Key is the key.
/// @tparam Hash is the hash functor for a key.
/// @tparam EqualTo is the equality comparison functor for a key.
/// @tparam InitialCapacity is the initial capacity, which must be a multiplicative of 16.
template<typename Derived,
         typename Key,
         std::unsigned_integral I,
         typename Hash = Hasher<Key>,
         typename EqualTo = std::equal_to<Key>,
         size_t InitialCapacity = 1024>
class HashIDMap
{
private:
    static_assert(InitialCapacity % 16 == 0, "InitialCapacity must be a multiple of 16.");

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    std::vector<Key> m_slots;
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

public:
    HashIDMap() : m_slots(), m_controls(), m_size(0), m_capacity(InitialCapacity), m_hash(), m_equal_to()
    {
        m_slots.resize(m_capacity);

        // Sentinel-padded rolling buffer
        m_controls.reserve(m_capacity + 15);
        m_controls.resize(m_capacity, ctrl_t::kEmpty);
        m_controls.resize(m_capacity + 15, ctrl_t::kSentinel);
    }

    bool has_capacity_for(size_t amount) const { return (static_cast<double>(size() + amount) / capacity()) <= MAX_LOAD_FACTOR; }

    I insert(const Key& slot)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");

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

    const Key& operator[](I pos) const { return m_slots[pos]; }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    double load_factor() const { return static_cast<double>(size()) / capacity(); }
    constexpr double max_load_factor() const { return MAX_LOAD_FACTOR; }
    const Statistics& statistics() const { return m_statistics; }
};

/// @brief `TreeHashIDMap` implements a HashIDMap for chains of perfectly balanced binary trees with DFS style rehash policy.
template<std::unsigned_integral I, typename Hash = Hasher<Slot<I>>, typename EqualTo = std::equal_to<Slot<I>>, size_t InitialCapacity = 1024>
class TreeHashIDMap : public HashIDMap<TreeHashIDMap<I, Hash, EqualTo, InitialCapacity>, Slot<I>, I, Hash, EqualTo, InitialCapacity>
{
private:
    IndexedHashSet<Slot<I>, I> m_roots;  ///< Dynamic hash ID maps require stable mapping for root nodes.
    std::vector<bool> m_stable_leaves;

    struct RehashData
    {
        size_t capacity;
        size_t size;
        std::vector<Slot<I>> slots;
        std::vector<ctrl_t> controls;

        explicit RehashData(size_t capacity) : capacity(capacity), size(0), slots(capacity), controls()
        {
            // Sentinel-padded rolling buffer
            controls.reserve(capacity + 15);
            controls.resize(capacity, ctrl_t::kEmpty);
            controls.resize(capacity + 15, ctrl_t::kSentinel);
        }

        bool has_capacity_for(size_t amount) const { return (static_cast<double>(size + amount) / capacity) <= MAX_LOAD_FACTOR; }
    };

    I insert(Slot<I> slot, RehashData& tmp)
    {
        assert(this->size() < tmp.capacity && "Insert failed. Rehashing to higher capacity is required.");

        size_t h = this->m_hash(slot);
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

                if (this->m_equal_to(tmp.slots[idx], slot))
                {
                    this->m_statistics.m_sum_probe_lengths += offset;
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
                this->m_statistics.m_sum_probe_lengths += offset;
                size_t idx = (i + offset) & mask;

                if (tmp.controls[idx] == ctrl_t::kEmpty)
                {
                    assert(tmp.controls[idx] == ctrl_t::kEmpty && "Unexpected overwrite!");

                    tmp.slots[idx] = slot;
                    tmp.controls[idx] = ctrl;
                    ++tmp.size;
                    ++this->m_statistics.m_num_probes;
                    return idx;
                }
            }

            // Else probe further
            i = (i + 16) & mask;
            this->m_statistics.m_sum_probe_lengths += 16;
        }
    }

    I rehash_recursively(I unstable_index, I size, RehashData& tmp)
    {
        /* Base case 1: skipped node creation */
        if (size == 1)
            return unstable_index;

        /* Note: caching relocation is expensive to cache because the tree structure depends on size. */

        assert(is_within_bounds(this->m_slots, unstable_index));
        const auto& slot = this->m_slots[unstable_index];

        /* Divide */
        assert(size >= 2);
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        I i1 = rehash_recursively(slot.i1, mid, tmp);
        I i2 = rehash_recursively(slot.i2, size - mid, tmp);

        return insert(Slot<I>(i1, i2), tmp);
    }

    /// @brief Depth-first rehash policy for a HashIDMap that stores a collection of perfectly balanced binary trees.
    /// @param new_capacity is the capacity after rehash.
    bool rehash_impl(size_t new_capacity)
    {
        auto tmp = RehashData(new_capacity);

        /* Relocate trees underlying the stable indices */
        m_roots.m_uniqueness.clear();

        auto backup_unstable_indices = std::vector<I> { 0 };

        // Relocate remaining roots.
        for (I stable_index = 1; stable_index < this->m_roots.size(); ++stable_index)
        {
            Slot root = this->m_roots.m_slots[stable_index];
            assert(root.i2 > 0);  // Ensure nonempty.

            backup_unstable_indices.push_back(root.i1);

            if (!tmp.has_capacity_for(root.i2))
            {
                /* Rollback rehash */
                for (I stable_index_2 = 1; stable_index_2 < backup_unstable_indices.size(); ++stable_index_2)
                {
                    this->m_roots.m_slots[stable_index].i1 = backup_unstable_indices[stable_index_2];
                }
                m_roots.m_uniqueness.clear();
                for (I stable_index_2 = 1; stable_index_2 < this->m_roots.size(); ++stable_index_2)
                {
                    this->m_roots.m_uniqueness.emplace(stable_index);
                }
                return false;
            }

            I unstable_index = rehash_recursively(root.i1, root.i2, tmp);

            this->m_roots.m_slots[stable_index] = Slot(unstable_index, root.i2);
            this->m_roots.m_uniqueness.emplace(stable_index);
        }

        if (tmp.size > new_capacity)
            throw std::runtime_error("Encountered insufficient capacity during rehash due to changed structural sharing.");

        this->m_capacity = new_capacity;
        this->m_size = tmp.size;
        std::swap(this->m_slots, tmp.slots);
        std::swap(this->m_controls, tmp.controls);

        return true;
    }

    using Base = HashIDMap<TreeHashIDMap<I, Hash, EqualTo, InitialCapacity>, Slot<I>, I, Hash, EqualTo, InitialCapacity>;

    friend Base;

public:
    explicit TreeHashIDMap() : Base(), m_roots()
    {
        this->m_roots.insert(get_empty_slot<I>());  // root representing empty sequence
        this->m_stable_leaves.push_back(false);
    }

    void rehash(double factor = 2.)
    {
        using clock = std::chrono::high_resolution_clock;

        auto start = clock::now();  // Start timing

        ++this->m_statistics.m_num_rehashes;

        size_t new_capacity = this->m_capacity;

        while (true)
        {
            new_capacity *= factor;

            if (rehash_impl(new_capacity))
                break;
        }

        auto end = clock::now();
        this->m_statistics.m_total_rehash_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }

    I insert_root(const Slot<I>& slot) { return m_roots.insert(slot); }

    I insert_internal(const Slot<I>& slot) { return Base::insert(slot); }

    const Slot<I>& lookup_root(I pos) const { return m_roots[pos]; }

    const Slot<I>& lookup_internal(I pos) const { return this->m_slots[pos]; }

    size_t num_internals() const { return Base::size(); }
    size_t num_roots() const { return m_roots.size(); }
    size_t num_slots() const { return num_internals() + num_roots(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_roots.mem_usage();
        usage += this->m_slots.capacity() * sizeof(Slot<I>);
        usage += this->m_controls.capacity() * sizeof(ctrl_t);
        return usage;
    }

    friend std::ostream& operator<<(std::ostream& os, const TreeHashIDMap& el)
    {
        os << el.m_slots << std::endl;
        return os;
    }
};
}

#endif
