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

#ifndef VALLA_INCLUDE_DTDB_S_IMPL_HPP_
#define VALLA_INCLUDE_DTDB_S_IMPL_HPP_

#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/succinct_indexed_hash_set.hpp"
#include "valla/unique_object_pool.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <stack>

namespace valla
{

///////////////////////////////////////////
/// General case with special leaf table
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::input_iterator Iterator, IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2, std::iter_value_t<Iterator>>
inline auto insert_recursively(Iterator it, Iterator end, typename Set1::index_type size, Set1& table, Set2& leaf_table)
{
    using I = typename Set1::index_type;

    /* Base cases */
    if (size == 1)
        return leaf_table.insert(*it);

    if (size == 2)
    {
        const auto i1 = leaf_table.insert(*it);
        const auto i2 = leaf_table.insert(*(it + 1));
        return table.insert(Slot<I>(i1, i2));
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto i1 = insert_recursively(it, mid_it, mid, table, leaf_table);
    const auto i2 = insert_recursively(mid_it, end, size - mid, table, leaf_table);

    return table.insert(Slot<I>(i1, i2));
}

template<std::ranges::input_range Range, IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2, std::ranges::range_value_t<Range>>
inline auto insert(const Range& state, Set1& table, Set2& leaf_table)
{
    using I = typename Set1::index_type;

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<I>(std::distance(state.begin(), state.end()));

    if (size == 0)
        return get_empty_slot<I>();  ///< Special case for empty state.

    return Slot<I>(insert_recursively(state.begin(), state.end(), size, table, leaf_table), size);
}

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void read_state_recursively(typename Set1::index_type index,
                                   typename Set1::index_type size,
                                   const Set1& table,
                                   const Set2& leaf_table,
                                   std::vector<typename Set2::value_type>& ref_state)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(leaf_table.lookup(index));
        return;
    }

    const auto& slot = table.lookup(index);

    /* Base case */
    if (size == 2)
    {
        ref_state.push_back(leaf_table.lookup(slot.i1));
        ref_state.push_back(leaf_table.lookup(slot.i2));
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_state_recursively(slot.i1, mid, table, leaf_table, ref_state);
    read_state_recursively(slot.i2, size - mid, table, leaf_table, ref_state);
}

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void read_state(typename Set1::index_type tree_index,
                       typename Set1::index_type size,
                       const Set1& table,
                       const Set2& leaf_table,
                       std::vector<typename Set2::value_type>& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, size, table, leaf_table, out_state);
}

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void
read_state(const Slot<typename Set1::index_type>& root_slot, const Set1& table, const Set2& leaf_table, std::vector<typename Set2::value_type>& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_state(root_slot.i1, root_slot.i2, table, leaf_table, out_state);
}

/**
 * ConstIterator
 */

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
class const_iterator_general
{
public:
    using difference_type = std::ptrdiff_t;
    using value_type = typename Set2::value_type;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    using index_type = typename Set1::index_type;

private:
    const Set1* m_inner_table;
    const Set2* m_leaf_table;
    UniqueObjectPoolPtr<std::vector<Entry<index_type>>> m_inner_stack;
    UniqueObjectPoolPtr<std::vector<value_type>> m_leaf_stack;
    std::optional<value_type> m_value;

    const Set1& inner_table() const
    {
        assert(m_inner_table);
        return *m_inner_table;
    }

    const Set2& leaf_table() const
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
                        m_leaf_stack->emplace_back(this->leaf_table().lookup(entry.m_index));
                        break;
                    }
                    else if (entry.m_size == 2)
                    {
                        const auto& slot = this->inner_table().lookup(entry.m_index);
                        m_leaf_stack->emplace_back(this->leaf_table().lookup(slot.i2));
                        m_leaf_stack->emplace_back(this->leaf_table().lookup(slot.i1));
                        break;
                    }

                    assert(entry.m_size > 2);

                    const auto& slot = this->inner_table().lookup(entry.m_index);

                    const auto mid = std::bit_floor(entry.m_size - 1);

                    // Emplace i2 first to ensure i1 is visited first in dfs.
                    m_inner_stack->emplace_back(slot.i2, entry.m_size - mid);
                    m_inner_stack->emplace_back(slot.i1, mid);
                }
            }
        }

        m_value = std::nullopt;
    }

public:
    const_iterator_general() : m_inner_table(nullptr), m_leaf_table(nullptr), m_inner_stack(), m_leaf_stack(), m_value(std::nullopt) {}
    const_iterator_general(const const_iterator_general& other) :
        m_inner_table(other.m_inner_table),
        m_leaf_table(other.m_leaf_table),
        m_inner_stack(other.m_inner_stack.clone()),
        m_leaf_stack(other.m_leaf_stack.clone()),
        m_value(other.m_value)
    {
    }
    const_iterator_general& operator=(const const_iterator_general& other)
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
    const_iterator_general(const_iterator_general&& other) = default;
    const_iterator_general& operator=(const_iterator_general&& other) = default;
    const_iterator_general(const Set1& inner_table, const Set2& leaf_table, const Slot<index_type>& root_slot, bool begin) :
        m_inner_table(&inner_table),
        m_leaf_table(&leaf_table),
        m_inner_stack(),
        m_leaf_stack(),
        m_value(std::nullopt)
    {
        assert(m_inner_table);

        if (begin && root_slot.i2 > 0)
        {
            m_inner_stack = get_stack_pool<std::vector<Entry<index_type>>>().get_or_allocate();
            m_inner_stack->clear();
            m_leaf_stack = get_stack_pool<std::vector<value_type>>().get_or_allocate();
            m_leaf_stack->clear();

            if (root_slot.i2 == 1)
            {
                m_leaf_stack->emplace_back(this->leaf_table().lookup(root_slot.i1));
            }
            else if (root_slot.i2 == 2)
            {
                const auto& slot = this->inner_table().lookup(root_slot.i1);
                m_leaf_stack->emplace_back(this->leaf_table().lookup(slot.i2));
                m_leaf_stack->emplace_back(this->leaf_table().lookup(slot.i1));
            }
            else if (root_slot.i2 > 2)
            {
                m_inner_stack->emplace_back(root_slot.i1, root_slot.i2);
            }

            advance();
        }
    }
    value_type operator*() const { return *m_value; }
    const_iterator_general& operator++()
    {
        advance();
        return *this;
    }
    const_iterator_general operator++(int)
    {
        auto it = *this;
        ++it;
        return it;
    }
    bool operator==(const const_iterator_general& other) const { return m_value == other.m_value; }
    bool operator!=(const const_iterator_general& other) const { return !(*this == other); }
};

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline auto begin(Slot<typename Set1::index_type> root, const Set1& table, const Set2& leaf_table)
{
    return const_iterator_general<Set1, Set2>(table, leaf_table, root, true);
}

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline auto end(const Set1&, const Set2&)
{
    return const_iterator_general<Set1, Set2>();
}

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline auto range(Slot<typename Set1::index_type> root, const Set1& table, const Set2& leaf_table)
{
    return std::ranges::subrange(begin(root, table, leaf_table), end(table, leaf_table));
}

///////////////////////////////////////////
/// Special case with Index range
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::input_iterator Iterator, IsStableIndexedHashSet Set>
    requires IsSpecialCaseHashSet<Set, std::iter_value_t<Iterator>>
inline auto insert_recursively(Iterator it, Iterator end, typename Set::index_type size, Set& table)
{
    using I = Set::index_type;

    /* Base cases */
    if (size == 1)
        return I(*it);  ///< Skip node creation

    if (size == 2)
        return table.insert(Slot<I>(*it, *(it + 1)));

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto i1 = insert_recursively(it, mid_it, mid, table);
    const auto i2 = insert_recursively(mid_it, end, size - mid, table);

    return table.insert(Slot<I>(i1, i2));
}

template<std::ranges::input_range Range, IsStableIndexedHashSet Set>
    requires IsSpecialCaseHashSet<Set, std::ranges::range_value_t<Range>>
inline auto insert(const Range& state, Set& table)
{
    using I = Set::index_type;

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<I>(std::distance(state.begin(), state.end()));

    if (size == 0)
        return get_empty_slot<I>();  ///< Special case for empty state.

    return Slot<I>(insert_recursively(state.begin(), state.end(), size, table), size);
}

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set>
inline void
read_state_recursively(typename Set::index_type index, typename Set::index_type size, const Set& table, std::vector<typename Set::index_type>& ref_state)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(index);
        return;
    }

    const auto slot = table.lookup(index);

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
    read_state_recursively(slot.i1, mid, table, ref_state);
    read_state_recursively(slot.i2, size - mid, table, ref_state);
}

template<IsStableIndexedHashSet Set>
inline void read_state(typename Set::index_type tree_index, typename Set::index_type size, const Set& table, std::vector<typename Set::index_type>& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, size, table, out_state);
}

template<IsStableIndexedHashSet Set>
inline void read_state(const Slot<typename Set::index_type>& root_slot, const Set& table, std::vector<typename Set::index_type>& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_state(root_slot.i1, root_slot.i2, table, out_state);
}

/**
 * ConstIterator
 */

template<IsStableIndexedHashSet Set>
class const_iterator_uint
{
public:
    using difference_type = std::ptrdiff_t;
    using value_type = typename Set::index_type;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

private:
    const Set* m_table;
    UniqueObjectPoolPtr<std::vector<Entry<value_type>>> m_stack;
    value_type m_value;

    static constexpr const value_type END_POS = std::numeric_limits<value_type>::max();

    const Set& table() const
    {
        assert(m_table);
        return *m_table;
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

            const auto slot = table().lookup(entry.m_index);

            const auto mid = std::bit_floor(entry.m_size - 1);

            // Emplace i2 first to ensure i1 is visited first in dfs.
            m_stack->emplace_back(slot.i2, entry.m_size - mid);
            m_stack->emplace_back(slot.i1, mid);
        }

        m_value = END_POS;
    }

public:
    const_iterator_uint() : m_table(nullptr), m_stack(), m_value(END_POS) {}
    const_iterator_uint(const const_iterator_uint& other) : m_table(other.m_table), m_stack(other.m_stack.clone()), m_value(other.m_value) {}
    const_iterator_uint& operator=(const const_iterator_uint& other)
    {
        if (*this != other)
        {
            m_table = other.m_table;
            m_stack = other.m_stack.clone();
            m_value = other.m_value;
        }
        return *this;
    }
    const_iterator_uint(const_iterator_uint&& other) = default;
    const_iterator_uint& operator=(const_iterator_uint&& other) = default;
    const_iterator_uint(const Set& table, Slot<value_type> root, bool begin) : m_table(&table), m_stack(), m_value(END_POS)
    {
        assert(m_table);

        if (begin)
        {
            m_stack = get_stack_pool<std::vector<Entry<value_type>>>().get_or_allocate();
            m_stack->clear();

            if (root.i2 > 0)  ///< Push to stack only if there leafs
            {
                m_stack->emplace_back(root.i1, root.i2);
                advance();
            }
        }
    }
    value_type operator*() const { return m_value; }
    const_iterator_uint& operator++()
    {
        advance();
        return *this;
    }
    const_iterator_uint operator++(int)
    {
        auto it = *this;
        ++it;
        return it;
    }
    bool operator==(const const_iterator_uint& other) const { return m_value == other.m_value; }
    bool operator!=(const const_iterator_uint& other) const { return !(*this == other); }
};

template<IsStableIndexedHashSet Set>
inline auto begin(Slot<typename Set::index_type> root, const Set& table)
{
    return const_iterator_uint<Set>(table, root, true);
}

template<IsStableIndexedHashSet Set>
inline auto end(const Set&)
{
    return const_iterator_uint<Set>();
}

template<IsStableIndexedHashSet Set>
inline auto range(Slot<typename Set::index_type> root, const Set& table)
{
    return std::ranges::subrange(begin(root, table), end(table));
}
}

#endif