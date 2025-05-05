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

#ifndef VALLA_INCLUDE_DELTA_TREE_COMPRESSION_HPP_
#define VALLA_INCLUDE_DELTA_TREE_COMPRESSION_HPP_

#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <ranges>
#include <stack>

namespace valla::delta
{

/// @brief Recursively insert the elements from `it` until `end` into the `table`.
/// @param it points to the first element.
/// @param end points after the last element.
/// @param table is the table to uniquely insert the slots.
/// @return the index of the slot at the root.
template<std::forward_iterator Iterator>
    requires std::same_as<std::iter_value_t<Iterator>, Index>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, IndexedHashSet& table, Index& prev)
{
    /* Base cases */
    if (size == 1)
    {
        Index delta = *it - prev;
        prev = *it;
        return delta;  ///< Skip node creation
    }

    if (size == 2)
    {
        Index left_delta = *it - prev;
        Index right_delta = *(it + 1) - *it;
        prev = *(it + 1);
        return table.insert_slot(make_slot(left_delta, right_delta)).first->second;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto left_index = insert_recursively(it, mid_it, mid, table, prev);
    const auto right_index = insert_recursively(mid_it, end, size - mid, table, prev);

    return table.insert_slot(make_slot(left_index, right_index)).first->second;
}

/// @brief Inserts the elements from the given `state` into the `tree_table` and the `root_table`.
/// @param state is the given state.
/// @param tree_table is the tree table whose nodes encode the tree structure without size information.
/// @param root_table is the root_table whose nodes encode the root tree index + the size of the state that defines the tree structure.
/// @return A pair (it, bool) where it points to the entry in the root table and bool is true if and only if the state was newly inserted.
template<std::ranges::forward_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, Index>
auto insert(const Range& state, IndexedHashSet& tree_table, IndexedHashSet& root_table)
{
    assert(std::is_sorted(state.begin(), state.end()));

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<size_t>(std::distance(state.begin(), state.end()));

    if (size == 0)                                                     ///< Special case for empty state.
        return root_table.insert_slot(make_slot(Index(0), Index(0)));  ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    auto prev = Index(0);
    return root_table.insert_slot(make_slot(insert_recursively(state.begin(), state.end(), size, tree_table, prev), size));
}

/// @brief Recursively reads the state from the tree induced by the given `index` and the `len`.
/// @param index is the index of the slot in the tree table.
/// @param size is the length of the state that defines the shape of the tree at the index.
/// @param tree_table is the tree table.
/// @param out_state is the output state.
inline void read_state_recursively(Index index, size_t size, const IndexedHashSet& tree_table, State& ref_state, Index& prev)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(prev += index);
        return;
    }

    const auto [left_index, right_index] = read_slot(tree_table.get_slot(index));

    /* Base case */
    if (size == 2)
    {
        ref_state.push_back(prev += left_index);
        ref_state.push_back(prev += right_index);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_state_recursively(left_index, mid, tree_table, ref_state, prev);
    read_state_recursively(right_index, size - mid, tree_table, ref_state, prev);
}

/// @brief Read the `out_state` from the given `tree_index` from the `tree_table`.
/// @param index
/// @param size
/// @param tree_table
/// @param out_state
inline void read_state(Index tree_index, size_t size, const IndexedHashSet& tree_table, State& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    auto prev = Index(0);
    read_state_recursively(tree_index, size, tree_table, out_state, prev);
}

/// @brief Read the `out_state` from the given `root_index` from the `root_table`.
/// @param root_index is the index of the slot in the root table.
/// @param tree_table is the tree table.
/// @param root_table is the root table.
/// @param out_state is the output state.
inline void read_state(Index root_index, const IndexedHashSet& tree_table, const IndexedHashSet& root_table, State& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    const auto [tree_index, size] = read_slot(root_table.get_slot(root_index));

    read_state(tree_index, size, tree_table, out_state);
}

class const_iterator
{
private:
    const IndexedHashSet* m_tree_table;

    struct Entry
    {
        Index m_index;
        Index m_size;
    };

    std::stack<Entry> m_stack;

    Index m_value;

    static constexpr const Index END_POS = Index(-1);

    void advance()
    {
        while (!m_stack.empty())
        {
            auto entry = m_stack.top();
            m_stack.pop();

            if (entry.m_size == 1)
            {
                m_value += entry.m_index;
                return;
            }

            const auto [left, right] = read_slot(m_tree_table->get_slot(entry.m_index));

            Index mid = std::bit_floor(entry.m_size - 1);

            // Emplace right first to ensure left is visited first in dfs.
            m_stack.emplace(right, entry.m_size - mid);
            m_stack.emplace(left, mid);
        }

        m_value = END_POS;
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = Index;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

    const_iterator() : m_tree_table(nullptr), m_stack(), m_value(END_POS) {}
    const_iterator(const IndexedHashSet* tree_table, Slot root, bool begin) : m_tree_table(tree_table), m_stack(), m_value(END_POS)
    {
        if (begin)
        {
            const auto [tree_idx, size] = read_slot(root);
            if (size > 0)  ///< Push to stack only if there leafs
            {
                m_value = Index(0);
                m_stack.emplace(tree_idx, size);
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

inline const_iterator begin(Slot root, const IndexedHashSet& tree_table) { return const_iterator(&tree_table, root, true); }

inline const_iterator end() { return const_iterator(); }

}

#endif