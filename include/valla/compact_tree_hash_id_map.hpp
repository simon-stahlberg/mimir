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

#ifndef VALLA_INCLUDE_COMPACT_TREE_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_COMPACT_TREE_HASH_ID_MAP_HPP_

#include "valla/bijective_hash.hpp"
#include "valla/compact_hash_id_map.hpp"
#include "valla/compact_utils.hpp"
#include "valla/concepts.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/statistics.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace valla
{
/// @brief `CompactTreeHashIDMap` implements a CompactHashIDMap for chains of perfectly balanced binary trees with DFS style rehash policy.
template<std::unsigned_integral I, IsStableIndexedHashSet RootSet, typename Hash = BijectiveHash<uint64_t>, typename EqualTo = EqualTo<Slot<I>>>
    requires std::same_as<typename RootSet::value_type, Slot<I>> && std::same_as<typename RootSet::index_type, I>
class CompactTreeHashIDMap : public compact_hash_id_map<Slot<I>, Hash, EqualTo>
{
private:
    using Base = compact_hash_id_map<Slot<I>, Hash, EqualTo>;

public:
    using value_type = Slot<I>;
    using index_type = I;

private:
    RootSet m_roots;

    I relocate_recursively(I unstable_index, I size, CompactTreeHashIDMap& tmp)
    {
        /* Base case 1: skipped node creation */
        if (size == I { 1 })
            return unstable_index;

        /* Note: caching relocation is expensive to cache because the tree structure depends on size. */

        assert(is_within_bounds(this->m_slots, unstable_index));
        const auto& slot = this->operator[](unstable_index);

        /* Divide */
        assert(size >= I { 2 });

        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        I i1 = relocate_recursively(slot.i1, mid, tmp);
        I i2 = relocate_recursively(slot.i2, size - mid, tmp);

        return tmp.insert_internal(Slot<I>(i1, i2));
    }

    /// @brief Depth-first rehash policy for a CompactHashIDMap that stores a collection of perfectly balanced binary trees.
    /// @param new_capacity is the capacity after rehash.
    bool rehash_impl(size_t new_capacity)
    {
        auto tmp = CompactTreeHashIDMap(new_capacity,
                                        std::max(this->width(), static_cast<uint8_t>(2 * std::bit_width(new_capacity - 1))),
                                        this->m_hash,
                                        this->m_equal_to);

        /* Relocate trees */
        for (I stable_index = I { 1 }; stable_index < this->m_roots.size(); ++stable_index)  // root 0 was already created
        {
            Slot root = this->m_roots.lookup(stable_index);

            if (tmp.growth_info().growth_left() < 2 * root.i2)
                return false;

            tmp.insert_root(Slot(relocate_recursively(root.i1, root.i2, tmp), root.i2));
        }

        tmp.m_statistics += this->statistics();

        std::swap(*this, tmp);

        return true;
    }

    void rehash()
    {
        using clock = std::chrono::high_resolution_clock;

        auto start = clock::now();  // Start timing

        this->m_statistics.increment_num_rehashes();

        size_t new_capacity = this->capacity();

        while (true)
        {
            new_capacity *= 2;

            if (rehash_impl(new_capacity))
                break;
        }

        this->m_statistics.increase_total_rehash_time(std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start));
    }

    void resize_width(uint8_t new_width)
    {
        auto tmp = CompactTreeHashIDMap(capacity(), new_width, this->m_hash, this->m_equal_to);

        /* Relocate trees */
        for (I stable_index = I { 1 }; stable_index < this->m_roots.size(); ++stable_index)  // root 0 was already created
        {
            Slot root = this->m_roots.lookup(stable_index);

            tmp.insert_root(Slot(relocate_recursively(root.i1, root.i2, tmp), root.i2));
        }

        tmp.m_statistics += this->statistics();

        std::swap(*this, tmp);
    }

public:
    using Base::capacity;
    using Base::growth_info;
    using Base::size;
    using Base::statistics;

    CompactTreeHashIDMap(size_t capacity = absl::container_internal::Group::kWidth, uint8_t width = 1, Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) :
        Base(capacity, width, hash, equal_to),
        m_roots()
    {
        this->m_roots.insert(Slot<I>());  // root representing empty sequence
    }

    // Moveable but not copieable
    CompactTreeHashIDMap(const CompactTreeHashIDMap&) = delete;
    CompactTreeHashIDMap& operator=(const CompactTreeHashIDMap&) = delete;
    CompactTreeHashIDMap(CompactTreeHashIDMap&&) = default;
    CompactTreeHashIDMap& operator=(CompactTreeHashIDMap&&) = default;

    template<std::ranges::forward_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, I>
    void resize_to_fit(Range sequence)
    {
        DEBUG_LOG("Started resizing to fit: capacity=" << this->capacity() << " width=" << static_cast<int>(this->width()));

        const auto [size, width] = compute_required_size_and_width(sequence, this->capacity());

        while (growth_info().growth_left() < size)
            rehash();

        if (width > this->m_width)
            resize_width(width);

        DEBUG_LOG("Finished resizing to fit: capacity=" << this->capacity() << " width=" << static_cast<int>(this->width()));
    }

    std::pair<uint64_t, bool> insert(const Slot<I>& key)
    {
        assert(Uint64tCoder<Slot<I>>::width(key) <= this->width());
        assert(this->m_growth_info.growth_left() > 0);

        return this->insert_impl(key);
    }

    I insert_root(const Slot<I>& slot) { return m_roots.insert(slot); }
    I insert_internal(const Slot<I>& slot) { return insert(slot).first; }

    Slot<I> lookup_root(I pos) const { return m_roots.lookup(pos); }
    Slot<I> lookup_internal(I pos) const { return this->operator[](pos); }

    size_t num_internals() const { return Base::size(); }
    size_t num_roots() const { return m_roots.size(); }
    size_t num_slots() const { return num_internals() + num_roots(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += Base::mem_usage();
        usage += m_roots.mem_usage();
        return usage;
    }

    friend std::ostream& operator<<(std::ostream& os, const CompactTreeHashIDMap& el)
    {
        os << el.m_slots << std::endl;
        return os;
    }
};
}

#endif
