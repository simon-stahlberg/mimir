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

#ifndef VALLA_INCLUDE_INDEXED_HASH_SET_HPP_
#define VALLA_INCLUDE_INDEXED_HASH_SET_HPP_

#include "valla/concepts.hpp"
#include "valla/equal_to.hpp"
#include "valla/hash.hpp"
#include "valla/utils.hpp"

#include <absl/container/flat_hash_set.h>
#include <concepts>
#include <cstddef>
#include <gtl/phmap.hpp>
#include <memory>
#include <mutex>
#include <oneapi/tbb.h>
#include <vector>

namespace valla
{

template<typename T, std::unsigned_integral I, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>>
class IndexedHashSet
{
public:
    using value_type = T;
    using index_type = I;

private:
    struct IndexReferencedHash
    {
        using is_transparent = void;

        std::shared_ptr<const tbb::concurrent_vector<T>> vec;
        Hash hash;

        IndexReferencedHash() : vec(nullptr), hash() {}
        explicit IndexReferencedHash(std::shared_ptr<const tbb::concurrent_vector<T>> vec) : vec(std::move(vec)), hash() {}

        size_t operator()(I el) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, el));
            return hash(vec->operator[](el));
        }
        size_t operator()(const T& el) const { return hash(el); }
    };

    struct IndexReferencedEqualTo
    {
        using is_transparent = void;

        std::shared_ptr<const tbb::concurrent_vector<T>> vec;
        EqualTo equal_to;

        IndexReferencedEqualTo() : vec(nullptr), equal_to() {}
        explicit IndexReferencedEqualTo(std::shared_ptr<const tbb::concurrent_vector<T>> vec) : vec(std::move(vec)), equal_to() {}

        bool operator()(I lhs, I rhs) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, lhs));
            assert(is_within_bounds(*vec, rhs));
            return equal_to(vec->operator[](lhs), vec->operator[](rhs));
        }
        bool operator()(const T& lhs, I rhs) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, rhs));
            return equal_to(lhs, (*vec)[rhs]);
        }
        bool operator()(I lhs, const T& rhs) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, lhs));
            return equal_to((*vec)[lhs], rhs);
        }
        bool operator()(const T& lhs, const T& rhs) const { return equal_to(lhs, rhs); }
    };

    size_t stripe_of(const T& slot) { return Hash {}(slot) & (kStripes - 1); }

public:
    IndexedHashSet() : m_slots(std::make_shared<tbb::concurrent_vector<T>>()), m_uniqueness(0, IndexReferencedHash(m_slots), IndexReferencedEqualTo(m_slots)) {}

    // Moveable but not copieable
    IndexedHashSet(const IndexedHashSet& other) = delete;
    IndexedHashSet& operator=(const IndexedHashSet& other) = delete;
    IndexedHashSet(IndexedHashSet&& other) : m_slots(std::move(other.m_slots)), m_uniqueness(std::move(other.m_uniqueness)) {}
    IndexedHashSet& operator=(IndexedHashSet&& other)
    {
        if (this != &other)
        {
            m_slots = std::move(other.m_slots);
            m_uniqueness = std::move(other.m_uniqueness);
        }
        return *this;
    }

    /// @brief Insert a slot uniquely and return its index.
    /// @param slot
    /// @return
    I insert(T slot)
    {
        assert(m_uniqueness.size() != std::numeric_limits<I>::max() && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        // Lock the stripe associated with the slot, which allows running the following code in parallel for different slots, with low false positive rate.
        std::lock_guard<std::mutex> lk(stripes[stripe_of(slot)]);

        if (auto it = m_uniqueness.find(slot); it != m_uniqueness.end())
            return *it;

        auto it = m_slots->push_back(slot);
        I index = it - m_slots->begin();
        m_uniqueness.emplace(index);

        return index;
    }

    /// @brief Lookup the slot of the given index.
    /// Thread-safe if m_slots is a segmented vector.
    /// @param index
    /// @return
    T lookup(I index) const
    {
        assert(index < m_slots->size() && "Index out of bounds");

        return m_slots->operator[](index);
    }

    size_t size() const { return m_slots->size(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots->capacity() * sizeof(T);
        usage += m_uniqueness.capacity() * (sizeof(I) + sizeof(absl::container_internal::ctrl_t));
        return usage;
    }

private:
    std::shared_ptr<tbb::concurrent_vector<T>> m_slots;
    gtl::parallel_flat_hash_set<I, IndexReferencedHash, IndexReferencedEqualTo, std::allocator<I>, 4, std::mutex> m_uniqueness;

    static constexpr size_t kStripes = 64;

    static_assert((kStripes & (kStripes - 1)) == 0, "kStripes must be a power of 2");

    std::array<std::mutex, kStripes> stripes;
};

static_assert(IsStableIndexedHashSet<IndexedHashSet<Slot<uint32_t>, uint32_t>>);

}

#endif
