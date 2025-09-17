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

#ifndef VALLA_INCLUDE_SUCCINCT_INDEXED_HASH_SET_HPP_
#define VALLA_INCLUDE_SUCCINCT_INDEXED_HASH_SET_HPP_

#include "valla/succinct_flat_hash_set.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <sdsl/int_vector.hpp>
#include <stack>

namespace valla
{
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>>
class SuccinctIndexedHashSet
{
public:
    using value_type = T;
    using index_type = I;

private:
    struct IndexReferencedHash
    {
        std::shared_ptr<const sdsl::int_vector<>> vec;
        Hash hash;

        IndexReferencedHash(std::shared_ptr<const sdsl::int_vector<>> vec) : vec(std::move(vec)), hash() {}

        size_t operator()(I el) const
        {
            assert(is_within_bounds(*vec, el));
            /* We obtain a stronger hash by decoding the slot and passing it into the Slot hash. */
            return hash(Uint64tCoder<T>::from_uint64_t(vec->operator[](el), vec->width()));
        }
    };

    struct IndexReferencedEqualTo
    {
        std::shared_ptr<const sdsl::int_vector<>> vec;
        EqualTo equal_to;

        IndexReferencedEqualTo(std::shared_ptr<const sdsl::int_vector<>> vec) : vec(std::move(vec)), equal_to() {}

        size_t operator()(I lhs, I rhs) const
        {
            assert(is_within_bounds(*vec, lhs));
            assert(is_within_bounds(*vec, rhs));
            return equal_to(vec->operator[](lhs), vec->operator[](rhs));
        }
    };

    void resize_width(uint8_t old_width, uint8_t new_width)
    {
        /* Rebuild slots */
        auto slots = std::make_shared<sdsl::int_vector<>>(m_capacity, 0, new_width);

        for (I i = 0; i < m_size; ++i)
            slots->operator[](i) = Uint64tCoder<T>::to_uint64_t(Uint64tCoder<T>::from_uint64_t(m_slots->operator[](i), old_width), new_width);

        std::swap(m_slots, slots);

        /* Rebuild uniqueness */
        m_uniqueness = succinct_flat_hash_set<I, I, IndexReferencedHash, IndexReferencedEqualTo>(IndexReferencedHash(m_slots), IndexReferencedEqualTo(m_slots));
        for (I i = 0; i < m_size; ++i)
            m_uniqueness.insert(i);
    }

public:
    SuccinctIndexedHashSet() :
        m_size(0),
        m_capacity(1),
        m_slots(std::make_shared<sdsl::int_vector<>>(1, 0, 2)),  // size 1, value 0, width 2
        m_uniqueness(IndexReferencedHash(m_slots), IndexReferencedEqualTo(m_slots))
    {
    }
    // Moveable but not copieable
    SuccinctIndexedHashSet(const SuccinctIndexedHashSet& other) = delete;
    SuccinctIndexedHashSet& operator=(const SuccinctIndexedHashSet& other) = delete;
    SuccinctIndexedHashSet(SuccinctIndexedHashSet&& other) = default;
    SuccinctIndexedHashSet& operator=(SuccinctIndexedHashSet&& other) = default;

    I insert(T slot)
    {
        assert(m_uniqueness.size() != std::numeric_limits<I>::max() && "SuccinctIndexedHashSet: Index overflow! The maximum number of slots reached.");

        /* Resize on insufficient capacity. */
        if (m_size == m_capacity)
        {
            m_capacity *= 2;
            m_slots->resize(m_capacity);
        }

        const auto new_width = Uint64tCoder<T>::width(slot);
        const auto old_width = m_slots->width();

        /* Rebuild on insufficient width. */
        if (new_width > old_width)
            resize_width(old_width, new_width);

        I index = m_size++;

        m_slots->operator[](index) = Uint64tCoder<T>::to_uint64_t(slot, m_slots->width());

        const auto result = m_uniqueness.insert(index);

        if (!result.second)
            --m_size;

        return *result.first;
    }

    T lookup(I index) const
    {
        assert(index < m_slots->size() && "Index out of bounds");

        return Uint64tCoder<T>::from_uint64_t(m_slots->operator[](index), m_slots->width());
    }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    uint8_t width() const { return m_slots->width(); }
    const sdsl::int_vector<>& slots() const { return *m_slots; }
    const succinct_flat_hash_set<I, I, IndexReferencedHash, IndexReferencedEqualTo> uniqueness() const { return m_uniqueness; }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots->capacity() / 8;
        usage += m_uniqueness.mem_usage();
        return usage;
    }

private:
    size_t m_size;
    size_t m_capacity;
    std::shared_ptr<sdsl::int_vector<>> m_slots;
    succinct_flat_hash_set<I, I, IndexReferencedHash, IndexReferencedEqualTo> m_uniqueness;
};

static_assert(IsStableIndexedHashSet<SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>>);
}

#endif
