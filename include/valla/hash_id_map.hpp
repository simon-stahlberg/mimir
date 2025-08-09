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

#include "valla/concepts.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/hash.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/statistics.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace valla
{
/// @brief `HashIDMap implements a hash ID map with open addressing in a Swiss table format where the position of a key implicitly becomes the index.
/// @tparam Derived is the derived class that must implement the rehash logic in rehash_impl.
/// @tparam Key is the key.
/// @tparam Hash is the hash functor for a key.
/// @tparam EqualTo is the equality comparison functor for a key.
/// @tparam InitialCapacity is the initial capacity.
template<typename Derived,
         typename Key,
         std::unsigned_integral I,
         typename Hash = Hash<Key>,
         typename EqualTo = std::equal_to<Key>,
         size_t InitialCapacity = absl::container_internal::Group::kWidth>
class HashIDMap
{
private:
    static_assert(is_power_of_two(InitialCapacity) && InitialCapacity >= absl::container_internal::Group::kWidth
                  && "InitialCapacity must be a power of two and greater or equal to Group::kWidth for wrap around.");

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    GrowthInfo m_growth_info;
    std::vector<Key> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

    HashIDMap(size_t capacity, Hash hash, EqualTo equal_to) :
        m_growth_info(capacity),
        m_slots(capacity),
        m_controls(capacity + absl::container_internal::NumClonedBytes(), absl::container_internal::ctrl_t::kEmpty),
        m_hash(hash),
        m_equal_to(equal_to)
    {
    }

    explicit HashIDMap(size_t capacity) : HashIDMap(capacity, Hash {}, EqualTo {}) {}

    HashIDMap() : HashIDMap(InitialCapacity, Hash {}, EqualTo {}) {}

    // Moveable but not copieable
    HashIDMap(const HashIDMap&) = delete;
    HashIDMap& operator=(const HashIDMap&) = delete;
    HashIDMap(HashIDMap&&) = default;
    HashIDMap& operator=(HashIDMap&&) = default;

    I insert(const Key& slot)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");

        m_statistics.increment_num_probes();

        size_t h = m_hash(slot);
        absl::container_internal::h2_t h2 = absl::container_internal::H2(h);

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, m_growth_info.mask());

        while (true)
        {
            absl::container_internal::Group group(&m_controls[probe.offset()]);

            for (const auto i : group.Match(h2))
            {
                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);
                assert(is_within_bounds(m_slots, offset));

                if (m_equal_to(m_slots[offset], slot))
                    return offset;
            }

            auto mask_empty = group.MaskEmpty();
            if (mask_empty)
            {
                int i = mask_empty.LowestBitSet();

                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);
                assert(is_within_bounds(m_slots, offset));

                m_slots[offset] = slot;
                m_controls[offset] = static_cast<absl::container_internal::ctrl_t>(h2);
                m_controls[((offset - absl::container_internal::NumClonedBytes()) & m_growth_info.mask()) + absl::container_internal::NumClonedBytes()] =
                    static_cast<absl::container_internal::ctrl_t>(h2);

                m_growth_info.increment_size();

                return offset;
            }

            m_statistics.increase_total_probe_length(absl::container_internal::Group::kWidth);

            probe.next();
        }
    }

    const Key& operator[](I pos) const { return m_slots[pos]; }

    const GrowthInfo& growth_info() const { return m_growth_info; }
    size_t size() const { return m_growth_info.size(); }
    size_t capacity() const { return m_growth_info.capacity(); }
    const HashSetStatistics& statistics() const { return m_statistics; }
};

/// @brief `TreeHashIDMap` implements a HashIDMap for chains of perfectly balanced binary trees with DFS style rehash policy.
template<std::unsigned_integral I,
         typename Hash = Hash<Slot<I>>,
         typename EqualTo = std::equal_to<Slot<I>>,
         IsStableIndexedHashSet RootSet = IndexedHashSet<Slot<I>, I, Hash, EqualTo>,
         size_t InitialCapacity = absl::container_internal::Group::kWidth>
    requires std::same_as<typename RootSet::value_type, Slot<I>> && std::same_as<typename RootSet::index_type, I>
class TreeHashIDMap : public HashIDMap<TreeHashIDMap<I, Hash, EqualTo, RootSet, InitialCapacity>, Slot<I>, I, Hash, EqualTo, InitialCapacity>
{
public:
    using value_type = Slot<I>;
    using index_type = I;

private:
    RootSet m_roots;

    I rehash_recursively(I unstable_index, I size, TreeHashIDMap& tmp)
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

        return tmp.insert(Slot<I>(i1, i2));
    }

    /// @brief Depth-first rehash policy for a HashIDMap that stores a collection of perfectly balanced binary trees.
    /// @param new_capacity is the capacity after rehash.
    bool rehash_impl(size_t new_capacity)
    {
        auto tmp = TreeHashIDMap<I, Hash, EqualTo, RootSet, InitialCapacity>(new_capacity);

        /* Relocate trees */
        for (I stable_index = 1; stable_index < this->m_roots.size(); ++stable_index)  // root 0 was already created
        {
            Slot root = this->m_roots.lookup(stable_index);

            if (tmp.growth_info().growth_left() < 2 * root.i2)
                return false;

            tmp.insert_root(Slot(rehash_recursively(root.i1, root.i2, tmp), root.i2));
        }

        std::swap(*this, tmp);

        return true;
    }

    using Base = HashIDMap<TreeHashIDMap<I, Hash, EqualTo, RootSet, InitialCapacity>, Slot<I>, I, Hash, EqualTo, InitialCapacity>;

    friend Base;

public:
    using Base::capacity;
    using Base::growth_info;
    using Base::size;
    using Base::statistics;

    TreeHashIDMap(size_t capacity, Hash hash, EqualTo equal_to) : Base(capacity, hash, equal_to), m_roots()
    {
        this->m_roots.insert(get_empty_slot<I>());  // root representing empty sequence
    }

    explicit TreeHashIDMap(size_t capacity) : TreeHashIDMap(capacity, Hash {}, EqualTo {}) {}

    TreeHashIDMap() : TreeHashIDMap(InitialCapacity) {}

    // Moveable but not copieable
    TreeHashIDMap(const TreeHashIDMap&) = delete;
    TreeHashIDMap& operator=(const TreeHashIDMap&) = delete;
    TreeHashIDMap(TreeHashIDMap&&) = default;
    TreeHashIDMap& operator=(TreeHashIDMap&&) = default;

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

    I insert_root(const Slot<I>& slot) { return m_roots.insert(slot); }

    I insert_internal(const Slot<I>& slot) { return Base::insert(slot); }

    const Slot<I>& lookup_root(I pos) const { return m_roots.lookup(pos); }

    const Slot<I>& lookup_internal(I pos) const { return this->m_slots[pos]; }

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

    friend std::ostream& operator<<(std::ostream& os, const TreeHashIDMap& el)
    {
        os << el.m_slots << std::endl;
        return os;
    }
};
}

#endif
