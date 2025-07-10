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

#ifndef VALLA_INCLUDE_DOUBLE_TREE_COMPRESSION_HPP_
#define VALLA_INCLUDE_DOUBLE_TREE_COMPRESSION_HPP_

#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/unique_object_pool.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <optional>
#include <ranges>
#include <stack>

namespace valla::d::plain
{

/**
 * Utility
 */

using RootSlotType = Slot<Index>;

constexpr inline Slot<Index> get_empty_root_slot() { return EMPTY_ROOT_SLOT; }

/**
 * Insert recursively
 */

/// @brief Recursively insert the elements from `it` until `end` into the `table`.
/// @param it points to the first element.
/// @param end points after the last element.
/// @param size is the number of elements in the range from it to end.
/// @param inner_table is the table to uniquely insert the inner slots.
/// @param leaf_table is the table to uniquely insert the leaf slots.
/// @return the index of the slot at the root.
template<std::input_iterator Iterator>
    requires std::same_as<std::iter_value_t<Iterator>, double>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, IndexedHashSet<Index>& inner_table, IndexedHashSet<double>& leaf_table)
{
    /* Base cases */
    if (size == 1)
        return *leaf_table.insert(Slot<double>(*it, *it)).first;

    if (size == 2)
        return *leaf_table.insert(Slot<double>(*it, *(it + 1))).first;

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto i1 = insert_recursively(it, mid_it, mid, inner_table, leaf_table);
    const auto i2 = insert_recursively(mid_it, end, size - mid, inner_table, leaf_table);

    return *inner_table.insert(Slot<Index>(i1, i2)).first;
}

/// @brief Inserts the elements from the given `state` into the `inner_table`.
/// @param state is the given state.
/// @param inner_table is the table to uniquely insert the inner slots.
/// @param leaf_table is the table to uniquely insert the leaf slots.
/// @return A pair (it, bool) where it points to the entry in the root table and bool is true if and only if the state was newly inserted.
template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, double>
auto insert(const Range& state, IndexedHashSet<Index>& inner_table, IndexedHashSet<double>& leaf_table)
{
    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<Index>(std::distance(state.begin(), state.end()));

    if (size == 0)                     ///< Special case for empty state.
        return get_empty_root_slot();  ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    return Slot<Index>(insert_recursively(state.begin(), state.end(), size, inner_table, leaf_table), size);
}

/**
 * Read recursively
 */

/// @brief Recursively reads the state from the tree induced by the given `index` and the `len`.
/// @param index is the index of the slot in the tree table.
/// @param size is the length of the state that defines the shape of the tree at the index.
/// @param inner_table is the table to uniquely insert the inner slots.
/// @param leaf_table is the table to uniquely insert the leaf slots.
/// @param out_state is the output state.
inline void
read_state_recursively(Index index, size_t size, const IndexedHashSet<Index>& inner_table, const IndexedHashSet<double>& leaf_table, DoubleList& ref_state)
{
    if (size == 1)
    {
        const auto slot = leaf_table[index];
        ref_state.push_back(slot.i1);
        return;
    }

    /* Base case */
    if (size == 2)
    {
        const auto slot = leaf_table[index];
        ref_state.push_back(slot.i1);
        ref_state.push_back(slot.i2);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto& slot = inner_table[index];
    read_state_recursively(slot.i1, mid, inner_table, leaf_table, ref_state);
    read_state_recursively(slot.i2, size - mid, inner_table, leaf_table, ref_state);
}

/// @brief Read the `out_state` from the given `tree_index` from the `inner_table`.
/// @param index
/// @param size
/// @param inner_table is the table to uniquely insert the inner slots.
/// @param leaf_table is the table to uniquely insert the leaf slots.
/// @param out_state
inline void read_state(Index tree_index, size_t size, const IndexedHashSet<Index>& inner_table, const IndexedHashSet<double>& leaf_table, DoubleList& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, size, inner_table, leaf_table, out_state);
}

/// @brief Read the `out_state` from the given `root_index` from the `root_table`.
/// @param root_index is the index of the slot in the root table.
/// @param inner_table is the table to uniquely insert the inner slots.
/// @param leaf_table is the table to uniquely insert the leaf slots.
/// @param root_table is the root table.
/// @param out_state is the output state.
inline void read_state(Slot<Index> root_slot, const IndexedHashSet<Index>& inner_table, const IndexedHashSet<double>& leaf_table, DoubleList& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_state(root_slot.i1, root_slot.i2, inner_table, leaf_table, out_state);
}

/**
 * ConstIterator
 */

static thread_local UniqueObjectPool<std::vector<Entry>> s_inner_stack_pool = UniqueObjectPool<std::vector<Entry>> {};
static thread_local UniqueObjectPool<std::vector<double>> s_leaf_stack_pool = UniqueObjectPool<std::vector<double>> {};

class const_iterator
{
private:
    const IndexedHashSet<Index>* m_inner_table;
    const IndexedHashSet<double>* m_leaf_table;
    UniqueObjectPoolPtr<std::vector<Entry>> m_inner_stack;
    UniqueObjectPoolPtr<std::vector<double>> m_leaf_stack;
    std::optional<double> m_value;

    const IndexedHashSet<Index>& inner_table() const
    {
        assert(m_inner_table);
        return *m_inner_table;
    }

    const IndexedHashSet<double>& leaf_table() const
    {
        assert(m_leaf_table);
        return *m_leaf_table;
    }

    void advance()
    {
        while (!(m_leaf_stack->empty() && m_inner_stack->empty()))
        {
            if (!m_leaf_stack->empty())
            {
                m_value = m_leaf_stack->back();
                m_leaf_stack->pop_back();
                return;
            }
            else
            {
                while (!m_inner_stack->empty())
                {
                    auto entry = m_inner_stack->back();
                    m_inner_stack->pop_back();

                    if (entry.m_size == 1)
                    {
                        m_leaf_stack->emplace_back(this->leaf_table()[entry.m_index].i1);
                        break;
                    }
                    else if (entry.m_size == 2)
                    {
                        const auto& slot = this->leaf_table()[entry.m_index];
                        m_leaf_stack->emplace_back(slot.i2);
                        m_leaf_stack->emplace_back(slot.i1);
                        break;
                    }

                    assert(entry.m_size > 2);

                    const auto slot = this->inner_table()[entry.m_index];

                    Index mid = std::bit_floor(entry.m_size - 1);

                    // Emplace i2 first to ensure i1 is visited first in dfs.
                    m_inner_stack->emplace_back(slot.i2, entry.m_size - mid);
                    m_inner_stack->emplace_back(slot.i1, mid);
                }
            }
        }

        m_value = std::nullopt;
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = double;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    const_iterator() : m_inner_table(nullptr), m_leaf_table(nullptr), m_inner_stack(), m_value(std::nullopt) {}
    const_iterator(const const_iterator& other) :
        m_inner_table(other.m_inner_table),
        m_leaf_table(other.m_leaf_table),
        m_inner_stack(other.m_inner_stack.clone()),
        m_leaf_stack(other.m_leaf_stack.clone()),
        m_value(other.m_value)
    {
    }
    const_iterator& operator=(const const_iterator& other)
    {
        if (*this != other)
        {
            m_inner_table = other.m_inner_table;
            m_leaf_table = other.m_leaf_table;
            m_inner_stack = other.m_inner_stack.clone();
            m_leaf_stack = other.m_leaf_stack.clone();
            m_value = other.m_value;
        }
        return *this;
    }
    const_iterator(const_iterator&& other) = default;
    const_iterator& operator=(const_iterator&& other) = default;
    const_iterator(const IndexedHashSet<Index>& inner_table, const IndexedHashSet<double>& leaf_table, Slot<Index> root, bool begin) :
        m_inner_table(&inner_table),
        m_leaf_table(&leaf_table),
        m_inner_stack(),
        m_leaf_stack(),
        m_value(std::nullopt)
    {
        assert(m_inner_table);

        if (begin && root.i2 > 0)
        {
            m_inner_stack = s_inner_stack_pool.get_or_allocate();
            m_inner_stack->clear();
            m_leaf_stack = s_leaf_stack_pool.get_or_allocate();
            m_leaf_stack->clear();

            if (root.i2 == 1)
            {
                m_leaf_stack->emplace_back(this->leaf_table()[root.i1].i1);
            }
            else if (root.i2 == 2)
            {
                const auto& slot = this->leaf_table()[root.i1];
                m_leaf_stack->emplace_back(slot.i2);
                m_leaf_stack->emplace_back(slot.i1);
            }
            else if (root.i2 > 2)
            {
                m_inner_stack->emplace_back(root.i1, root.i2);
            }

            advance();
        }
    }
    value_type operator*() const { return *m_value; }
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

inline const_iterator begin(Slot<Index> root, const IndexedHashSet<Index>& inner_table, const IndexedHashSet<double>& leaf_table)
{
    return const_iterator(inner_table, leaf_table, root, true);
}

inline const_iterator end() { return const_iterator(); }

}

#endif