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

#ifndef VALLA_INCLUDE_SIMPLE_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_SIMPLE_HASH_ID_MAP_HPP_

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
/// @brief `SimpleHashIDMap implements a hash ID map with open addressing in a Swiss table format where the position of a key implicitly becomes the index.
/// @tparam Key is the key.
/// @tparam Hash is the hash functor for a key.
/// @tparam EqualTo is the equality comparison functor for a key.
template<typename Key, std::unsigned_integral I, typename Hash = Hash<Key>, typename EqualTo = EqualTo<Key>>
class SimpleHashIDMap
{
public:
    using value_type = Key;
    using index_type = I;

protected:
    GrowthInfo m_growth_info;
    std::vector<Key> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

public:
    SimpleHashIDMap(size_t capacity = absl::container_internal::Group::kWidth, Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) :
        m_growth_info(capacity),
        m_slots(capacity),
        m_controls(capacity + absl::container_internal::NumClonedBytes(), absl::container_internal::ctrl_t::kEmpty),
        m_hash(hash),
        m_equal_to(equal_to)
    {
    }

    // Moveable but not copieable
    SimpleHashIDMap(const SimpleHashIDMap&) = delete;
    SimpleHashIDMap& operator=(const SimpleHashIDMap&) = delete;
    SimpleHashIDMap(SimpleHashIDMap&&) = default;
    SimpleHashIDMap& operator=(SimpleHashIDMap&&) = default;

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
                if (offset < absl::container_internal::NumClonedBytes())
                    m_controls[capacity() + offset] = static_cast<absl::container_internal::ctrl_t>(h2);

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

}

#endif
