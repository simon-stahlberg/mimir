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
#include <memory>
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
        std::shared_ptr<const std::vector<T>> vec;
        Hash hash;

        explicit IndexReferencedHash(std::shared_ptr<const std::vector<T>> vec) : vec(std::move(vec)), hash() {}

        size_t operator()(I el) const
        {
            assert(is_within_bounds(*vec, el));
            return hash(vec->operator[](el));
        }
    };

    struct IndexReferencedEqualTo
    {
        std::shared_ptr<const std::vector<T>> vec;
        EqualTo equal_to;

        explicit IndexReferencedEqualTo(std::shared_ptr<const std::vector<T>> vec) : vec(std::move(vec)), equal_to() {}

        size_t operator()(I lhs, I rhs) const
        {
            assert(is_within_bounds(*vec, lhs));
            assert(is_within_bounds(*vec, rhs));
            return equal_to(vec->operator[](lhs), vec->operator[](rhs));
        }
    };

public:
    IndexedHashSet() : m_slots(std::make_shared<std::vector<T>>()), m_uniqueness(0, IndexReferencedHash(m_slots), IndexReferencedEqualTo(m_slots)) {}

    // Moveable but not copieable
    IndexedHashSet(const IndexedHashSet& other) = delete;
    IndexedHashSet& operator=(const IndexedHashSet& other) = delete;
    IndexedHashSet(IndexedHashSet&& other) = default;
    IndexedHashSet& operator=(IndexedHashSet&& other) = default;

    I insert(T slot)
    {
        assert(m_uniqueness.size() != std::numeric_limits<I>::max() && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        I index = m_slots->size();

        m_slots->push_back(slot);

        const auto result = m_uniqueness.emplace(index);

        if (!result.second)
            m_slots->pop_back();

        return *result.first;
    }

    const T& lookup(I index) const
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
    std::shared_ptr<std::vector<T>> m_slots;
    absl::flat_hash_set<I, IndexReferencedHash, IndexReferencedEqualTo> m_uniqueness;

    template<std::unsigned_integral I_, typename Hash_, typename EqualTo_, IsStableIndexedHashSet RootSet_, size_t InitialCapacity_>
        requires std::same_as<typename RootSet_::value_type, Slot<I_>> && std::same_as<typename RootSet_::index_type, I_>
    friend class TreeHashIDMap;
};

static_assert(IsStableIndexedHashSet<IndexedHashSet<Slot<uint32_t>, uint32_t>>);

}

#endif
