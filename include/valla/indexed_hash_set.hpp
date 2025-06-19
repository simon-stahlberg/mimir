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
template<typename S>
class IndexedHashSet
{
public:
    auto insert(S slot)
    {
        assert(m_slot_to_index.size() != std::numeric_limits<Index>::max() && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        const auto result = m_slot_to_index.emplace(slot, m_slot_to_index.size());

        if (result.second)
        {
            m_index_to_slot.push_back(slot);
        }

        return result;
    }

    S operator[](Index index) const
    {
        assert(index < m_index_to_slot.size() && "Index out of bounds");

        return m_index_to_slot[index];
    }

    size_t size() const { return m_index_to_slot.size(); }

private:
    absl::flat_hash_map<S, Index, SlotHash<S>> m_slot_to_index;
    std::vector<S> m_index_to_slot;
};

template<>
class IndexedHashSet<double>
{
public:
    struct Value
    {
        uint32_t index;
        uint32_t ref_count;
    };

    auto insert(double slot)
    {
        assert(m_slot_to_index.size() != std::numeric_limits<Index>::max() && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        Index index;
        if (m_free_list.empty())
        {
            index = m_slot_to_index.size();
        }
        else
        {
            index = m_free_list.top();
            m_free_list.pop();
        }

        auto result = m_slot_to_index.emplace(slot, Value { index, 1 });

        if (result.second)
        {
            if (index == m_index_to_slot.size())
            {
                m_index_to_slot.push_back(slot);
            }
            else
            {
                m_index_to_slot[index] = slot;
            }
        }
        else
        {
            ++result.first->second.ref_count;
        }

        return result;
    }

    void erase(double slot)
    {
        auto it = m_slot_to_index.find(slot);
        assert(it != m_slot_to_index.end());

        --it->second.ref_count;

        if (it->second.ref_count == 0)
        {
            m_free_list.push(it->second.index);
            m_slot_to_index.erase(it);
        }
    }

    double operator[](Index index) const
    {
        assert(index < m_index_to_slot.size() && "Index out of bounds");

        return m_index_to_slot[index];
    }

    size_t size() const { return m_index_to_slot.size(); }

private:
    absl::flat_hash_map<double, Value, SlotHash<double>> m_slot_to_index;
    std::vector<double> m_index_to_slot;
    std::stack<Index> m_free_list;
};
}

#endif
