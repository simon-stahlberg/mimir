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

#ifndef VALLA_INCLUDE_COMPACT_BUCKET_HASH_SET_HPP_
#define VALLA_INCLUDE_COMPACT_BUCKET_HASH_SET_HPP_

#include "valla/compact_hash.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/hash.hpp"
#include "valla/uint64tcoder.hpp"
#include "valla/utils.hpp"

#include <absl/container/internal/raw_hash_set.h>
#include <sdsl/int_vector.hpp>

namespace valla
{
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = CompactHash<T>, typename EqualTo = EqualTo<T>, size_t InitialCapacity = 128>
class compact_bucket_hash_set
{
public:
    class const_iterator;
    friend class const_iterator;

    using value_type = T;
    using const_iterator_type = const_iterator;

private:
    static_assert(is_power_of_two(InitialCapacity) && "InitialCapacity must be a power of two.");
    static_assert(InitialCapacity >= 128, "InitialCapacity must be greater than 128.");

    static constexpr size_t MAX_BUCKET_SIZE = 256;

    class Bucket
    {
        // Split quotient into control byte (7 bits of hash) + overflow
        std::vector<absl::container_internal::ctrl_t> m_controls;
        sdsl::int_vector<> m_overflow;
    };

    static_assert(sizeof(Bucket) == 24);

    GrowthInfo m_growth_info;
    sdsl::int_vector<> m_slots;
    sdsl::int_vector<1> m_v;  ///< virgin bits
    sdsl::int_vector<1> m_c;  ///< change bits
    sdsl::int_vector<4> m_a;  ///< at-home array

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

private:
    std::pair<const_iterator, bool> insert_impl(const T& key) { uint64_t coded = Uint64tCoder<T>::to_uint64_t(key); }

    void resize_width(uint8_t old_width, uint8_t new_width)
    {
        auto slots = sdsl::int_vector<>(capacity(), 0, new_width);

        if (size() > 0)
            for (I i = 0; i < capacity(); ++i)
                slots[i] = Uint64tCoder<T>::to_uint64_t(Uint64tCoder<T>::from_uint64_t(m_slots[i], old_width), new_width);

        std::swap(m_slots, slots);
    }

public:
    compact_bucket_hash_set(size_t capacity, uint8_t bit_width, Hash hash, EqualTo equal_to) :
        m_growth_info(std::max(size_t(128), capacity)),
        m_slots(this->capacity(), 0, std::max(uint8_t(1), bit_width)),
        m_v(this->capacity()),
        m_c(this->capacity()),
        m_a(this->capacity()),
        m_hash(hash),
        m_equal_to(equal_to)
    {
    }

    compact_bucket_hash_set(Hash hash, EqualTo equal_to) : compact_bucket_hash_set(InitialCapacity, 1, hash, equal_to) {}

    compact_bucket_hash_set() : compact_bucket_hash_set(Hash {}, EqualTo {}) {}

    std::pair<const_iterator, bool> insert(const T& key)
    {
        const auto new_width = Uint64tCoder<T>::bit_width(key);
        const auto old_width = m_slots.width();

        if (new_width > old_width)
            resize_width(old_width, new_width);

        if (m_growth_info.growth_left() == 0)
            rehash();

        return insert_impl(key);
    }

    void rehash() {}

    class const_iterator
    {
    private:
        const compact_bucket_hash_set* m_set;
        size_t m_pos;
        I m_value;

        const compact_bucket_hash_set& set() const
        {
            assert(m_set);
            return *m_set;
        }

        void advance()
        {
            do
            {
                ++m_pos;
            } while (m_pos < set().capacity() && static_cast<int>((set().m_controls)[m_pos]) < 0);
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        const_iterator() : m_set(nullptr), m_pos(-1) {}

        const_iterator(const compact_bucket_hash_set& set, bool begin) : m_set(set), m_pos(begin ? -1 : set.capacity())
        {
            if (begin)
                advance();
        }

        const_iterator(const compact_bucket_hash_set& set, size_t pos) : m_set(&set), m_pos(pos) { assert(static_cast<int>(set.m_controls[pos]) >= 0); }

        value_type operator*() const { return Uint64tCoder<T>::from_uint64_t((set().m_slots)[m_pos], set().m_slots.width()); }

        const_iterator& operator++()
        {
            ++m_pos;
            advance();
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const { return m_pos == other.m_pos; }

        bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    const_iterator begin() const { return const_iterator(*this, true); }
    const_iterator end() const { return const_iterator(*this, false); }

    const GrowthInfo& growth_info() const { return m_growth_info; }
    const sdsl::int_vector<>& slots() const { return m_slots; }
    const sdsl::int_vector<1>& v() const { return m_v; }
    const sdsl::int_vector<1>& c() const { return m_c; }
    const sdsl::int_vector<4>& a() const { return m_a; }
    size_t size() const { return m_growth_info.size(); }
    size_t capacity() const { return m_growth_info.capacity(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots.capacity() / 8;
        usage += m_controls.capacity() * sizeof(absl::container_internal::ctrl_t);
        return usage;
    }
};
}

#endif
