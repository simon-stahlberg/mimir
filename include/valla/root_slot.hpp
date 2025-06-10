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
/// The number of ordering bits is n + 1, i.e., one for each tree node.
struct RootSlot
{
    Slot slot;
    BitsetConstView ordering;

    RootSlot() : slot(0), ordering() {}
    RootSlot(Slot slot, BitsetConstView ordering) : slot(slot), ordering(ordering) {}
};

struct RootSlotHash
{
private:
    std::reference_wrapper<const BitsetPool> m_pool;

public:
    explicit RootSlotHash(const BitsetPool& pool) : m_pool(pool) {}

    /// @brief TODO: we might want to use bitmasks.
    /// @param el
    /// @return
    size_t operator()(RootSlot el) const
    {
        size_t size = read_pos(el.slot, 1);
        size_t seed = 0;
        for (size_t bit = 0; bit < std::bit_ceil(size); ++bit)
        {
            hash_combine(seed, el.ordering.get(bit, m_pool.get()));
        }
        hash_combine(seed, SlotHash {}(el.slot));
        return seed;
    }
};

struct RootSlotEqualTo
{
private:
    std::reference_wrapper<const BitsetPool> m_pool;

public:
    explicit RootSlotEqualTo(const BitsetPool& pool) : m_pool(pool) {}

    /// @brief TODO: we might want to use bitmasks.
    /// @param lhs
    /// @param rhs
    /// @return
    bool operator()(RootSlot lhs, RootSlot rhs) const
    {
        size_t lhs_size = read_pos(lhs.slot, 1);
        size_t rhs_size = read_pos(rhs.slot, 1);
        if (lhs_size != rhs_size || lhs.slot != rhs.slot)
            return false;
        for (size_t bit = 0; bit < std::bit_ceil(lhs_size); ++bit)
        {
            if (lhs.ordering.get(bit, m_pool.get()) != rhs.ordering.get(bit, m_pool.get()))
                return false;
        }
        return true;
    }
};

class RootIndexedHashSet
{
public:
    explicit RootIndexedHashSet(BitsetPool& pool) :
        m_slot_to_index(absl::flat_hash_map<RootSlot, Index, RootSlotHash, RootSlotEqualTo>(0, RootSlotHash(pool), RootSlotEqualTo(pool)))
    {
    }

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
