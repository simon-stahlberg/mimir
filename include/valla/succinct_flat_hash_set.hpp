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

#ifndef VALLA_INCLUDE_SUCCINCT_FLAT_HASH_SET_HPP_
#define VALLA_INCLUDE_SUCCINCT_FLAT_HASH_SET_HPP_

#include "valla/concepts.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/hash.hpp"
#include "valla/statistics.hpp"
#include "valla/utils.hpp"

#include <absl/container/internal/raw_hash_set.h>
#include <cstddef>
#include <cstdint>
#include <sdsl/int_vector.hpp>
#include <vector>

namespace valla
{
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>>
class succinct_flat_hash_set
{
public:
    class const_iterator;
    friend class const_iterator;

    using value_type = T;
    using const_iterator_type = const_iterator;

private:
    GrowthInfo m_growth_info;
    sdsl::int_vector<> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

private:
    std::pair<const_iterator, bool> insert_impl(const T& key)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");
        assert(Uint64tCoder<T>::width(key) <= m_slots.width() && "Insert failed. Slot width is insufficient to store the key.");

        m_statistics.increment_num_probes();

        size_t h = m_hash(key);
        absl::container_internal::h2_t h2 = absl::container_internal::H2(h);
        assert(static_cast<int>(h2) >= 0);

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, m_growth_info.mask());

        while (true)
        {
            absl::container_internal::Group group(&m_controls[probe.offset()]);

            for (const auto i : group.Match(h2))
            {
                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);
                assert(is_within_bounds(m_slots, offset));

                if (m_equal_to(m_slots[offset], key))
                    return { const_iterator(*this, offset), false };
            }

            auto mask_empty = group.MaskEmpty();
            if (mask_empty)
            {
                int i = mask_empty.LowestBitSet();

                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);

                assert(is_within_bounds(m_slots, offset));
                assert(m_controls[offset] == absl::container_internal::ctrl_t::kEmpty);

                m_slots[offset] = Uint64tCoder<T>::to_uint64_t(key, m_slots.width());
                m_controls[offset] = static_cast<absl::container_internal::ctrl_t>(h2);
                if (offset < absl::container_internal::NumClonedBytes())
                    m_controls[capacity() + offset] = static_cast<absl::container_internal::ctrl_t>(h2);

                m_growth_info.increment_size();

                return { const_iterator(*this, offset), true };
            }

            m_statistics.increase_total_probe_length(absl::container_internal::Group::kWidth);

            probe.next();
        }
    }

    void resize_width(uint8_t old_width, uint8_t new_width)
    {
        auto slots = sdsl::int_vector<>(capacity(), 0, new_width);

        if (size() > 0)
        {
            for (I i = 0; i < capacity(); ++i)
                if (static_cast<int>(m_controls[i]) >= 0)
                    slots[i] = Uint64tCoder<T>::to_uint64_t(Uint64tCoder<T>::from_uint64_t(m_slots[i], old_width), new_width);
        }

        std::swap(m_slots, slots);
    }

public:
    succinct_flat_hash_set(size_t capacity = absl::container_internal::Group::kWidth, uint8_t width = 1, Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) :
        m_growth_info(capacity),
        m_slots(this->capacity(), 0, width),  ///< bit width must be at least one, else it is set to 64
        m_controls(this->capacity() + absl::container_internal::NumClonedBytes(), absl::container_internal::ctrl_t::kEmpty),
        m_hash(hash),
        m_equal_to(equal_to)
    {
        assert(width > 0 && width <= 64 && "width must be in range [1,64].");
    }

    succinct_flat_hash_set(Hash hash, EqualTo equal_to) : succinct_flat_hash_set(absl::container_internal::Group::kWidth, 1, hash, equal_to) {}

    std::pair<const_iterator, bool> insert(const T& key)
    {
        const auto new_width = Uint64tCoder<T>::width(key);
        const auto old_width = m_slots.width();

        if (new_width > old_width)
            resize_width(old_width, new_width);

        if (m_growth_info.growth_left() == 0)
            rehash();

        return insert_impl(key);
    }

    void rehash()
    {
        auto tmp = succinct_flat_hash_set(2 * capacity(), m_slots.width(), m_hash, m_equal_to);

        for (size_t i = 0; i < capacity(); ++i)
            if (static_cast<int>(m_controls[i]) >= 0)
                tmp.insert(m_slots[i]);

        tmp.m_statistics += m_statistics;

        std::swap(*this, tmp);
    }

    class const_iterator
    {
    private:
        const succinct_flat_hash_set* m_set;
        size_t m_pos;

        const succinct_flat_hash_set& set() const
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

        const_iterator(const succinct_flat_hash_set& set, bool begin) : m_set(&set), m_pos(begin ? -1 : set.capacity())
        {
            if (begin)
                advance();
        }

        const_iterator(const succinct_flat_hash_set& set, size_t pos) : m_set(&set), m_pos(pos) { assert(static_cast<int>(set.m_controls[pos]) >= 0); }

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
    const std::vector<absl::container_internal::ctrl_t>& controls() const { return m_controls; }
    size_t size() const { return m_growth_info.size(); }
    size_t capacity() const { return m_growth_info.capacity(); }
    uint8_t width() const { return m_slots.width(); }

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
