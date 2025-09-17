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

#ifndef VALLA_INCLUDE_COMPACT_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_COMPACT_HASH_ID_MAP_HPP_

#include "valla/bijective_hash.hpp"
#include "valla/concepts.hpp"
#include "valla/debug.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/statistics.hpp"
#include "valla/utils.hpp"

#include <absl/container/flat_hash_map.h>
#include <absl/container/internal/raw_hash_set.h>
#include <cstddef>
#include <cstdint>
#include <sdsl/int_vector.hpp>
#include <vector>

namespace valla
{
template<IsUint64tCodable T, typename Hash = BijectiveHash<uint64_t>, typename EqualTo = EqualTo<T>>
class compact_hash_id_map
{
public:
    class const_iterator;
    friend class const_iterator;

    using value_type = T;

protected:
    enum class disp_t : uint8_t
    {
        kOverflow = 255,
    };

    GrowthInfo m_growth_info;

    sdsl::int_vector<> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;
    std::vector<disp_t> m_displacement;
    absl::flat_hash_map<uint32_t, uint32_t> m_displacement_ext;

    uint8_t m_width;

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

    inline uint64_t H(const T& key, uint8_t width) const { return m_hash.hash(Uint64tCoder<T>::to_uint64_t(key, width), width); }
    inline uint64_t Q(uint64_t h) { return h >> m_growth_info.log2_capacity(); }
    inline uint64_t R(uint64_t h) { return h & m_growth_info.mask(); }

    /* Split Q into Q1 (stored in m_slots with variable bit-width) and Q2 (the control byte used for SIMD probing) */
    static inline uint64_t Q1(uint64_t q) { return (q >> 7); }
    static inline absl::container_internal::h2_t Q2(uint64_t q) { return q & 0x7F; }

    /// @brief Test occupancy of a given position.
    /// @param i is the position.
    /// @return true iff there is an element stored at the given position i, and false otherwise.
    inline bool is_occupied(size_t i) const { return static_cast<int>(m_controls[i]) >= 0; }

    /// @brief Get the displacement of an element stored in a given position.
    /// @param i is the position.
    /// @return
    inline uint32_t displacement(size_t i) const
    {
        assert(is_occupied(i));
        return (m_displacement[i] == disp_t::kOverflow) ? m_displacement_ext.at(i) : static_cast<uint32_t>(m_displacement[i]);
    }

    /// @brief Get the home location of an element hashed to the given position.
    /// @param i is the position.
    /// @return
    inline uint64_t home(size_t i) const
    {
        assert(is_occupied(i));
        return (i - displacement(i)) & m_growth_info.mask();
    }

    /// @brief Decode the key stored at the given position.
    /// @param i
    /// @return
    inline T decode_key(size_t i) const
    {
        uint64_t q1 = m_slots[i];
        uint64_t q2 = static_cast<uint64_t>(m_controls[i]);
        uint64_t q = (q1 << 7) | q2;
        uint64_t r = home(i);
        uint64_t h = (q * m_growth_info.capacity()) | r;
        uint64_t inverse_h = m_hash.invert_hash(h, m_width);

        DEBUG_LOG("q1=" << q1 << " q2=" << q2 << " q=" << q << " r=" << r << " h=" << h << " inverse_h=" << inverse_h);

        return Uint64tCoder<T>::from_uint64_t(inverse_h, m_width);
    }

    std::pair<uint64_t, bool> insert_impl(const T& key)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");
        assert(Uint64tCoder<T>::width(key) <= m_width && "Insert failed. Slot width is insufficient to store the key.");

        m_statistics.increment_num_probes();

        const auto h = H(key, m_width);
        const auto q = Q(h);
        const auto r = R(h);
        const auto q1 = Q1(q);
        const auto q2 = Q2(q);

        DEBUG_LOG("this=" << this << " key=" << key << " h=" << h << " q=" << q << " r=" << r << " q1=" << q1 << " q2=" << static_cast<int>(q2)
                          << " width=" << static_cast<int>(m_width));

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, m_growth_info.mask());

        while (true)
        {
            absl::container_internal::Group group(&m_controls[probe.offset()]);

            for (const auto i : group.Match(q2))
            {
                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);
                assert(is_within_bounds(m_slots, offset));

                if (home(offset) == r && m_equal_to(m_slots[offset], q1))  // matching home, q1, and q2 implies matching key
                {
                    return { offset, false };
                }
            }

            auto mask_empty = group.MaskEmpty();
            if (mask_empty)
            {
                int i = mask_empty.LowestBitSet();

                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);

                assert(is_within_bounds(m_slots, offset));
                assert(m_controls[offset] == absl::container_internal::ctrl_t::kEmpty);

                m_slots[offset] = q1;
                m_controls[offset] = static_cast<absl::container_internal::ctrl_t>(q2);
                if (offset < absl::container_internal::NumClonedBytes())
                    m_controls[capacity() + offset] = static_cast<absl::container_internal::ctrl_t>(q2);

                uint32_t d = (offset - r) & m_growth_info.mask();

                if (d < static_cast<uint8_t>(disp_t::kOverflow))
                    m_displacement[offset] = static_cast<disp_t>(d);
                else
                {
                    m_displacement[offset] = disp_t::kOverflow;
                    m_displacement_ext[offset] = d;
                }

                m_growth_info.increment_size();

                return { offset, true };
            }

            m_statistics.increase_total_probe_length(absl::container_internal::Group::kWidth);

            probe.next();
        }
    }

    void resize_width(uint8_t new_width)
    {
        auto tmp = compact_hash_id_map(capacity(), new_width, m_hash, m_equal_to);

        if (size() > 0)
            for (uint64_t i = 0; i < capacity(); ++i)
                if (static_cast<int>(m_controls[i]) >= 0)
                    tmp.insert(decode_key(i));

        tmp.m_statistics += m_statistics;

        std::swap(*this, tmp);
    }

public:
    compact_hash_id_map(size_t capacity = absl::container_internal::Group::kWidth, uint8_t width = 1, Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) :
        m_growth_info(capacity),
        m_slots(this->capacity(), 0, std::max(1, static_cast<int>(width) - 7)),  ///< bit width must be at least one, else it is set to 64
        m_controls(this->capacity() + absl::container_internal::NumClonedBytes(), absl::container_internal::ctrl_t::kEmpty),
        m_displacement(this->capacity()),
        m_displacement_ext(),
        m_width(width),
        m_hash(hash),
        m_equal_to(equal_to)
    {
        assert(width > 0 && width <= 64 && "width must be in range [1,64].");
    }

    compact_hash_id_map(Hash hash, EqualTo equal_to) : compact_hash_id_map(absl::container_internal::Group::kWidth, 1, hash, equal_to) {}

    // Moveable but not copieable
    compact_hash_id_map(const compact_hash_id_map&) = delete;
    compact_hash_id_map& operator=(const compact_hash_id_map&) = delete;
    compact_hash_id_map(compact_hash_id_map&&) = default;
    compact_hash_id_map& operator=(compact_hash_id_map&&) = default;

    std::pair<uint64_t, bool> insert(const T& key)
    {
        const auto new_width = Uint64tCoder<T>::width(key);

        if (new_width > m_width)
            resize_width(new_width);

        if (m_growth_info.growth_left() == 0)
            rehash();

        return insert_impl(key);
    }

    T operator[](uint64_t pos) const
    {
        assert(is_occupied(pos));
        return decode_key(pos);
    }

    void rehash()
    {
        auto tmp = compact_hash_id_map(2 * capacity(), m_width, m_hash, m_equal_to);

        for (size_t i = 0; i < capacity(); ++i)
            if (static_cast<int>(m_controls[i]) >= 0)
                tmp.insert(decode_key(i));

        tmp.m_statistics += m_statistics;

        std::swap(*this, tmp);
    }

    class const_iterator
    {
    private:
        const compact_hash_id_map* m_set;
        size_t m_pos;

        const compact_hash_id_map& set() const
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

        const_iterator(const compact_hash_id_map& set, bool begin) : m_set(&set), m_pos(begin ? -1 : set.capacity())
        {
            if (begin)
                advance();
        }

        const_iterator(const compact_hash_id_map& set, size_t pos) : m_set(&set), m_pos(pos) { assert(static_cast<int>(set.m_controls[pos]) >= 0); }

        value_type operator*() const { return set().decode_key(m_pos); }

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
    size_t size() const { return m_growth_info.size(); }
    size_t capacity() const { return m_growth_info.capacity(); }
    uint8_t width() const { return m_width; }
    const HashSetStatistics& statistics() const { return m_statistics; }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots.capacity() / 8;
        usage += m_controls.capacity() * sizeof(typename decltype(m_controls)::value_type);
        usage += m_displacement.capacity() * sizeof(typename decltype(m_displacement)::value_type);
        usage += m_displacement_ext.capacity() * (sizeof(typename decltype(m_displacement_ext)::value_type));
        return usage;
    }
};

/**
 * Implementations
 */
}

#endif
