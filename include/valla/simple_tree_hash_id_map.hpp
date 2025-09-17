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

#ifndef VALLA_INCLUDE_SIMPLE_TREE_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_SIMPLE_TREE_HASH_ID_MAP_HPP_

#include "valla/concepts.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/hash.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/simple_hash_id_map.hpp"
#include "valla/statistics.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace valla
{

/// @brief `SimpleTreeHashIDMap` implements a HashIDMap for chains of perfectly balanced binary trees with DFS style rehash policy.
template<std::unsigned_integral I, IsStableIndexedHashSet RootSet, typename Hash = Hash<Slot<I>>, typename EqualTo = EqualTo<Slot<I>>>
    requires std::same_as<typename RootSet::value_type, Slot<I>> && std::same_as<typename RootSet::index_type, I>
class SimpleTreeHashIDMap : public SimpleHashIDMap<Slot<I>, I, Hash, EqualTo>
{
private:
    using Base = SimpleHashIDMap<Slot<I>, I, Hash, EqualTo>;

public:
    using value_type = Slot<I>;
    using index_type = I;

    using Base::capacity;
    using Base::growth_info;
    using Base::size;
    using Base::statistics;

private:
    RootSet m_roots;

    I relocate_recursively(I unstable_index, I size, SimpleTreeHashIDMap& tmp)
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
        I i1 = relocate_recursively(slot.i1, mid, tmp);
        I i2 = relocate_recursively(slot.i2, size - mid, tmp);

        return tmp.insert_internal(Slot<I>(i1, i2));
    }

    /// @brief Depth-first rehash policy for a HashIDMap that stores a collection of perfectly balanced binary trees.
    /// @param new_capacity is the capacity after rehash.
    bool rehash_impl(size_t new_capacity)
    {
        auto tmp = SimpleTreeHashIDMap(new_capacity);

        /* Relocate trees */
        for (I stable_index = 1; stable_index < this->m_roots.size(); ++stable_index)  // root 0 was already created
        {
            Slot root = this->m_roots.lookup(stable_index);

            if (tmp.growth_info().growth_left() < 2 * root.i2)
                return false;

            tmp.insert_root(Slot(relocate_recursively(root.i1, root.i2, tmp), root.i2));
        }

        std::swap(*this, tmp);

        return true;
    }

public:
    SimpleTreeHashIDMap(size_t capacity = absl::container_internal::Group::kWidth, Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) :
        Base(capacity, hash, equal_to),
        m_roots()
    {
        this->m_roots.insert(Slot<I>());  // root representing empty sequence
    }

    // Moveable but not copieable
    SimpleTreeHashIDMap(const SimpleTreeHashIDMap&) = delete;
    SimpleTreeHashIDMap& operator=(const SimpleTreeHashIDMap&) = delete;
    SimpleTreeHashIDMap(SimpleTreeHashIDMap&&) = default;
    SimpleTreeHashIDMap& operator=(SimpleTreeHashIDMap&&) = default;

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

    template<std::ranges::forward_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, I>
    void resize_to_fit(Range sequence)
    {
        const auto size = static_cast<I>(std::distance(sequence.begin(), sequence.end()));

        while (growth_info().growth_left() < 2 * size)
            rehash();
    }

    I insert_root(const Slot<I>& slot) { return m_roots.insert(slot); }

    I insert_internal(const Slot<I>& slot) { return Base::insert(slot); }

    Slot<I> lookup_root(I pos) const { return m_roots.lookup(pos); }

    Slot<I> lookup_internal(I pos) const { return this->m_slots[pos]; }

    size_t num_internals() const { return Base::size(); }
    size_t num_roots() const { return m_roots.size(); }
    size_t num_slots() const { return num_internals() + num_roots(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_roots.mem_usage();
        usage += this->m_slots.capacity() * sizeof(Slot<I>);
        usage += this->m_controls.capacity() * sizeof(absl::container_internal::ctrl_t);
        return usage;
    }

    friend std::ostream& operator<<(std::ostream& os, const SimpleTreeHashIDMap& el)
    {
        os << el.m_slots << std::endl;
        return os;
    }
};
}

#endif
