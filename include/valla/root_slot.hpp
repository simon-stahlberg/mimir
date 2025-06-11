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

#ifndef VALLA_INCLUDE_ROOT_SLOT_HPP_
#define VALLA_INCLUDE_ROOT_SLOT_HPP_

#include "valla/bitset_pool.hpp"
#include "valla/declarations.hpp"

namespace valla
{
/// @brief `RootSlot` encapsulates a base slot consisting of the slot encoding the tree node index and the size, and the ordering bitset.
struct RootSlot
{
    Slot slot;
    const Bitset* ordering;

    RootSlot() : slot(0), ordering() {}
    RootSlot(Slot slot, const Bitset* ordering) : slot(slot), ordering(ordering) {}

    Slot get_slot() const { return slot; }
    Index get_index() const { return read_pos(slot, 0); }
    Index get_size() const { return read_pos(slot, 1); }
    const Bitset& get_ordering() const { return *ordering; }
};

static_assert(sizeof(RootSlot) == 16);
static_assert(sizeof(RootSlot*) == 8);

struct RootSlotHash
{
    size_t operator()(const RootSlot& el) const { return cantor_pair(SlotHash {}(el.slot), el.get_ordering().get_index()); }
};

struct RootSlotEqualTo
{
    bool operator()(const RootSlot& lhs, const RootSlot& rhs) const
    {
        return lhs.get_slot() == rhs.get_slot() && lhs.get_ordering().get_index() == rhs.get_ordering().get_index();
    }
};

struct DerefRootSlotHash
{
    size_t operator()(const RootSlot* el) const { return RootSlotHash {}(*el); }
};

struct DerefRootSlotEqualTo
{
    bool operator()(const RootSlot* lhs, const RootSlot* rhs) const { return RootSlotEqualTo {}(*lhs, *rhs); }
};

class RootIndexedHashSet
{
private:
    void resize_to_fit()
    {
        if (m_index_to_slot.back().size() == m_offset)
        {
            const auto segment_size = m_index_to_slot.back().size() * 2;
            m_index_to_slot.push_back(std::vector<RootSlot>(segment_size));
            ++m_segment;
            m_offset = 0;
            m_capacity += segment_size;
        }
    }

public:
    const RootSlot* get_empty_root() const { return m_empty_root; }

    auto insert_slot(RootSlot slot)
    {
        resize_to_fit();

        const auto& element = m_index_to_slot[m_segment][m_offset] = slot;

        const auto result = m_slot_to_index.emplace(&element, m_size);

        if (result.second)
        {
            ++m_offset;
            ++m_size;
        }

        return result;
    }

    explicit RootIndexedHashSet(const BitsetRepository& repository) :
        m_slot_to_index(),
        m_index_to_slot(1, std::vector<RootSlot>(1)),
        m_segment(0),
        m_offset(0),
        m_size(0),
        m_capacity(1),
        m_empty_root(insert_slot(RootSlot(0, repository.get_empty_bitset())).first->first)
    {
    }

    const RootSlot& get_slot(Index index) const
    {
        assert(index < m_size && "Index out of bounds");

        const auto segment = std::bit_width(index + 1) - 1;
        const auto offset = index - ((size_t(1) << segment) - 1);

        return m_index_to_slot[segment][offset];
    }

    size_t size() const { return m_size; }

    size_t get_memory_usage() const
    {
        size_t usage = 0;

        usage += m_slot_to_index.capacity() * (sizeof(RootSlot) + sizeof(Index));
        usage += m_slot_to_index.capacity();

        usage += m_capacity * sizeof(RootSlot);

        return usage;
    }

private:
    absl::flat_hash_map<const RootSlot*, Index, DerefRootSlotHash, DerefRootSlotEqualTo> m_slot_to_index;
    std::vector<std::vector<RootSlot>> m_index_to_slot;
    size_t m_segment;
    size_t m_offset;
    size_t m_size;
    size_t m_capacity;

    const RootSlot* m_empty_root;
};
}

#endif
