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
    BitsetConstView ordering;

    RootSlot() : slot(0), ordering() {}
    RootSlot(Slot slot, BitsetConstView ordering) : slot(slot), ordering(ordering) {}

    Slot get_slot() const { return slot; }
    Index get_index() const { return read_pos(slot, 0); }
    Index get_size() const { return read_pos(slot, 1); }
    BitsetConstView get_ordering() const { return ordering; }
};

struct RootSlotHash
{
public:
    size_t operator()(RootSlot el) const { return cantor_pair(cantor_pair(el.get_index(), el.get_size()), el.get_ordering().get_index()); }
};

struct RootSlotEqualTo
{
public:
    bool operator()(RootSlot lhs, RootSlot rhs) const
    {
        return lhs.get_slot() == rhs.get_slot() && lhs.get_ordering().get_index() == rhs.get_ordering().get_index();
    }
};

class RootIndexedHashSet
{
public:
    RootIndexedHashSet() : m_slot_to_index(), m_index_to_slot() {}

    auto insert_slot(RootSlot slot)
    {
        const auto result = m_slot_to_index.emplace(slot, m_slot_to_index.size());

        if (result.second)
        {
            m_index_to_slot.push_back(slot);
        }

        return result;
    }

    RootSlot get_slot(Index index) const
    {
        assert(index < m_index_to_slot.size() && "Index out of bounds");

        return m_index_to_slot[index];
    }

    size_t size() const { return m_index_to_slot.size(); }

    size_t get_memory_usage() const
    {
        size_t usage = 0;

        usage += m_slot_to_index.capacity() * (sizeof(RootSlot) + sizeof(Index));
        usage += m_slot_to_index.capacity();

        usage += m_index_to_slot.capacity() * sizeof(RootSlot);

        return usage;
    }

private:
    absl::flat_hash_map<RootSlot, Index, RootSlotHash, RootSlotEqualTo> m_slot_to_index;
    std::vector<RootSlot> m_index_to_slot;
};
}

#endif
