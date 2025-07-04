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

#include "valla/declarations.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <stack>

namespace valla
{
/// @brief `IndexedHashSet` encapsulates a bijective function f : Slot -> Index with inverse mapping f^{-1} : Index -> Slot
/// where the indices in the image are enumerated 0,1,2,... and so on.
class IndexedHashSet
{
public:
    IndexedHashSet() : m_index_to_slot(), m_slot_to_index(0, IndexReferencedSlotHash(m_index_to_slot), IndexReferencedSlotEqualTo(m_index_to_slot)) {}

    auto insert(Slot slot)
    {
        assert(m_slot_to_index.size() != std::numeric_limits<Index>::max() && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        Index index = m_index_to_slot.size();

        m_index_to_slot.push_back(slot);

        const auto result = m_slot_to_index.emplace(index);

        if (!result.second)
            m_index_to_slot.pop_back();

        return result;
    }

    Slot operator[](Index index) const
    {
        assert(index < m_index_to_slot.size() && "Index out of bounds");

        return m_index_to_slot[index];
    }

    size_t size() const { return m_index_to_slot.size(); }

private:
    std::vector<Slot> m_index_to_slot;
    absl::flat_hash_set<Index, IndexReferencedSlotHash, IndexReferencedSlotEqualTo> m_slot_to_index;
};

}

#endif
