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

#ifndef VALLA_INCLUDE_PLAIN_UINT_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_PLAIN_UINT_HASH_ID_MAP_HPP_

#include "valla/declarations.hpp"
#include "valla/hash_id_map.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/unique_object_pool.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <ranges>
#include <stack>

namespace valla::plain::uint::hash_id_map
{

/**
 * Insert recursively
 */

/// @brief Recursively insert the elements from `it` until `end` into the `table`.
/// @param it points to the first element.
/// @param end points after the last element.
/// @param size is the number of elements in the range from it to end.
/// @param table is the table to uniquely insert the slots.
/// @return the index of the slot at the root.
template<std::input_iterator Iterator, typename Hash, typename EqualTo, size_t InitialCapacity>
    requires std::same_as<std::iter_value_t<Iterator>, Index>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, TreeHashIDMap<Hash, EqualTo, InitialCapacity>& table)
{
    /* Base cases */
    if (size == 1)
        return *it;  ///< Skip node creation

    if (size == 2)
        return table.insert_internal(Slot<Index>(*it, *(it + 1)));

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */

    const auto mid_it = it + mid;
    const auto i1 = insert_recursively(it, mid_it, mid, table);
    const auto i2 = insert_recursively(mid_it, end, size - mid, table);

    return table.insert_internal(Slot<Index>(i1, i2));
}

/// @brief Inserts the elements from the given `state` into the `table`.
/// @param state is the given state.
/// @param table is the tree table whose nodes encode the tree structure without size information.
/// @return A pair (it, bool) where it points to the entry in the root table and bool is true if and only if the state was newly inserted.
template<std::ranges::input_range Range, typename Hash, typename EqualTo, size_t InitialCapacity>
    requires std::same_as<std::ranges::range_value_t<Range>, Index>
Index insert(const Range& state, TreeHashIDMap<Hash, EqualTo, InitialCapacity>& table)
{
    assert(std::is_sorted(state.begin(), state.end()));

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<Index>(std::distance(state.begin(), state.end()));

    if (size == 0)  ///< Special case for empty state.
        return 0;   ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    if ((static_cast<double>(table.size() + 2 * size) / table.capacity()) > table.max_load_factor())
        table.rehash();

    return table.insert_root(Slot<Index>(insert_recursively(state.begin(), state.end(), size, table), size));
}

/**
 * Read recursively
 */

/// @brief Recursively reads the state from the tree induced by the given `index` and the `len`.
/// @param index is the index of the slot in the tree table.
/// @param size is the length of the state that defines the shape of the tree at the index.
/// @param tree_table is the tree table.
/// @param out_state is the output state.
template<typename Hash, typename EqualTo, size_t InitialCapacity>
inline void read_state_recursively(Index index, size_t size, const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& tree_table, IndexList& ref_state)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(index);
        return;
    }

    const auto slot = tree_table.lookup_internal(index);

    /* Base case */
    if (size == 2)
    {
        ref_state.push_back(slot.i1);
        ref_state.push_back(slot.i2);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_state_recursively(slot.i1, mid, tree_table, ref_state);
    read_state_recursively(slot.i2, size - mid, tree_table, ref_state);
}

/// @brief Read the `out_state` from the given `tree_index` from the `tree_table`.
/// @param index
/// @param size
/// @param tree_table
/// @param out_state
template<typename Hash, typename EqualTo, size_t InitialCapacity>
inline void read_state(Index tree_index, size_t size, const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& tree_table, IndexList& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, size, tree_table, out_state);
}

/// @brief Read the `out_state` from the given `root_index` from the `root_table`.
/// @param root_index is the index of the slot in the root table.
/// @param tree_table is the tree table.
/// @param root_table is the root table.
/// @param out_state is the output state.
template<typename Hash, typename EqualTo, size_t InitialCapacity>
inline void read_state(Index root_index, const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& tree_table, IndexList& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    const auto& slot = tree_table.lookup_root(root_index);

    read_state(slot.i1, slot.i2, tree_table, out_state);
}

/**
 * ConstIterator
 */

static thread_local UniqueObjectPool<std::vector<Entry>> s_stack_pool = UniqueObjectPool<std::vector<Entry>> {};

template<typename Hash, typename EqualTo, size_t InitialCapacity>
class const_iterator
{
private:
    const TreeHashIDMap<Hash, EqualTo, InitialCapacity>* m_tree_table;
    UniqueObjectPoolPtr<std::vector<Entry>> m_stack;
    Index m_value;

    static constexpr const Index END_POS = Index(-1);

    const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& tree_table() const
    {
        assert(m_tree_table);
        return *m_tree_table;
    }

    void advance()
    {
        while (!m_stack->empty())
        {
            auto entry = m_stack->back();
            m_stack->pop_back();

            if (entry.m_size == 1)
            {
                m_value = entry.m_index;
                return;
            }

            const auto slot = tree_table().lookup_internal(entry.m_index);

            Index mid = std::bit_floor(entry.m_size - 1);

            // Emplace i2 first to ensure i1 is visited first in dfs.
            m_stack->emplace_back(slot.i2, entry.m_size - mid);
            m_stack->emplace_back(slot.i1, mid);
        }

        m_value = END_POS;
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = Index;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    const_iterator() : m_tree_table(nullptr), m_stack(), m_value(END_POS) {}
    const_iterator(const const_iterator& other) : m_tree_table(other.m_tree_table), m_stack(other.m_stack.clone()), m_value(other.m_value) {}
    const_iterator& operator=(const const_iterator& other)
    {
        if (*this != other)
        {
            m_tree_table = other.m_tree_table;
            m_stack = other.m_stack.clone();
            m_value = other.m_value;
        }
        return *this;
    }
    const_iterator(const_iterator&& other) = default;
    const_iterator& operator=(const_iterator&& other) = default;
    const_iterator(const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& tree_table, Index root, bool begin) :
        m_tree_table(&tree_table),
        m_stack(),
        m_value(END_POS)
    {
        assert(m_tree_table);

        if (begin)
        {
            m_stack = s_stack_pool.get_or_allocate();
            m_stack->clear();

            const auto& root_slot = tree_table.lookup_root(root);

            if (root_slot.i2 > 0)  ///< Push to stack only if there leafs
            {
                m_stack->emplace_back(root_slot.i1, root_slot.i2);
                advance();
            }
        }
    }
    value_type operator*() const { return m_value; }
    const_iterator& operator++()
    {
        advance();
        return *this;
    }
    const_iterator operator++(int)
    {
        auto it = *this;
        ++it;
        return it;
    }
    bool operator==(const const_iterator& other) const { return m_value == other.m_value; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
};

template<typename Hash, typename EqualTo, size_t InitialCapacity>
inline const_iterator<Hash, EqualTo, InitialCapacity> begin(Index root, const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& table)
{
    return const_iterator(table, root, true);
}

template<typename Hash, typename EqualTo, size_t InitialCapacity>
inline const_iterator<Hash, EqualTo, InitialCapacity> end(const TreeHashIDMap<Hash, EqualTo, InitialCapacity>&)
{
    return const_iterator<Hash, EqualTo, InitialCapacity>();
}

template<typename Hash, typename EqualTo, size_t InitialCapacity>
inline auto range(Index root, const TreeHashIDMap<Hash, EqualTo, InitialCapacity>& table)
{
    return std::ranges::subrange(begin(root, table), end(table));
}

}

#endif