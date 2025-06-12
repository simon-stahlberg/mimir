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
    const Bitset& get_ordering() const
    {
        assert(ordering);
        return *ordering;
    }
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

class RootIndexedHashSet
{
private:
public:
    const RootSlot& get_empty_root() const { return m_empty_root; }

    auto insert(RootSlot slot) { return m_slot_to_index.emplace(slot, m_slot_to_index.size()); }

    explicit RootIndexedHashSet(const BitsetRepository& repository) :
        m_slot_to_index(),
        m_empty_root(insert(RootSlot(0, &repository.get_empty_bitset())).first->first)
    {
    }

    size_t size() const { return m_slot_to_index.size(); }

private:
    absl::node_hash_map<RootSlot, Index, RootSlotHash, RootSlotEqualTo> m_slot_to_index;

    const RootSlot& m_empty_root;
};
}

#endif
