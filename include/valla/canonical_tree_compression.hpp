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

#ifndef VALLA_INCLUDE_CANONICAL_TREE_COMPRESSION_HPP_
#define VALLA_INCLUDE_CANONICAL_TREE_COMPRESSION_HPP_

#include "valla/bitset_pool.hpp"
#include "valla/declarations.hpp"
#include "valla/details/shared_memory_pool.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/root_slot.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <ranges>
#include <stack>

namespace valla::canonical
{

/**
 * Utility
 */

using RootSlotType = RootSlot;

inline RootSlot get_empty_root_slot(const BitsetRepository& repo) { return RootSlot(0, &repo.get_empty_bitset()); }

/**
 * Insert recursively
 */

/// @brief Recursively insert the elements from `it` until `end` into the `table`.
/// @param it points to the first element.
/// @param end points after the last element.
/// @param size is the number of elements in the range from it to end.
/// @param view is the ordering.
/// @param bit is the position in the ordering for the next tree node being created.
/// @param table is the table to uniquely insert the slots.
/// @return the index of the slot at the root.
template<std::input_iterator Iterator>
    requires std::same_as<std::iter_value_t<Iterator>, Index>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, Bitset view, size_t bit, IndexedHashSet& table)
{
    /* Base cases */
    if (size == 1)
        return *it;  ///< Skip node creation

    if (size == 2)
    {
        Index i1 = *it;
        Index i2 = *(it + 1);

        // std::cout << "base_insert: i1=" << i1 << " i2=" << i2 << " bit=" << bit << " comp" << (i2 < i1) << std::endl;

        if (i2 < i1)
        {
            std::swap(i1, i2);
            view.set(bit);
        }

        return table.insert(make_slot(i1, i2)).first->second;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;

    Index i1 = insert_recursively(it, mid_it, mid, view, 2 * bit + 1, table);
    Index i2 = insert_recursively(mid_it, end, size - mid, view, 2 * bit + 2, table);

    // std::cout << "inductive_insert: i1=" << i1 << " i2=" << i2 << " bit=" << bit << " comp" << (i2 < i1) << std::endl;

    if (i2 < i1)
    {
        std::swap(i1, i2);
        view.set(bit);
    }

    return table.insert(make_slot(i1, i2)).first->second;
}

/// @brief Inserts the elements from the given `state` into the `tree_table`.
/// @param state is the given state.
/// @param tree_table is the tree table whose nodes encode the tree structure without size information.
/// @param pool is the bitset pool for allocation.
/// @param repo is the bitset repository for uniqueness.
/// @return A pair (it, bool) where it points to the entry in the root table and bool is true if and only if the state was newly inserted.
template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, Index>
auto insert(const Range& state, IndexedHashSet& tree_table, BitsetPool& pool, BitsetRepository& repo)
{
    assert(std::is_sorted(state.begin(), state.end()));

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<size_t>(std::distance(state.begin(), state.end()));

    if (size == 0)                         ///< Special case for empty state.
        return get_empty_root_slot(repo);  ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    // Since we represent the ordering as a binary tree, there is some padding because we round up to use 64 bit blocks for efficiency.
    // std::cout << "num bits=" << std::bit_ceil(size) << std::endl;
    auto ordering = pool.allocate(std::bit_ceil(size));

    // std::cout << "bitset_ptr=" << ordering.get_blocks() << std::endl;

    const auto bit = size_t(0);
    const auto tree_index = insert_recursively(state.begin(), state.end(), size, ordering, bit, tree_table);

    // Undo the bitset allocation when proven that an identical bitset already exists
    const auto result = repo.insert(ordering);
    if (!result.second)
    {
        pool.pop_allocation();
    }

    return RootSlot(make_slot(tree_index, size), &*result.first);
}

/**
 * Read recursively
 */

/// @brief Recursively reads the state from the tree induced by the given `index` and the `len`.
/// @param index is the index of the slot in the tree table.
/// @param size is the length of the state that defines the shape of the tree at the index.
/// @param tree_table is the tree table.
/// @param out_state is the output state.
inline void read_state_recursively(Index index, size_t size, size_t bit, const Bitset& ordering, const IndexedHashSet& tree_table, State& ref_state)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(index);
        return;
    }

    auto [i1, i2] = read_slot(tree_table.get_slot(index));

    const auto must_swap = ordering.get(bit);
    if (must_swap)
        std::swap(i1, i2);

    /* Base case */
    if (size == 2)
    {
        // std::cout << "base_read: i1=" << i1 << " i2=" << i2 << " bit=" << bit << " comp" << must_swap << std::endl;

        ref_state.push_back(i1);
        ref_state.push_back(i2);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    // std::cout << "inductive_read: i1=" << i1 << " i2=" << i2 << " bit=" << bit << " comp" << must_swap << std::endl;

    /* Conquer */
    read_state_recursively(i1, mid, 2 * bit + 1, ordering, tree_table, ref_state);
    read_state_recursively(i2, size - mid, 2 * bit + 2, ordering, tree_table, ref_state);
}

/// @brief Read the `out_state` from the given `tree_index` from the `tree_table`.
/// @param index
/// @param size
/// @param tree_table
/// @param out_state
inline void read_state(Index tree_index, size_t size, const Bitset& ordering, const IndexedHashSet& tree_table, State& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    const auto bit = size_t(0);

    // std::cout << "bitset_ptr=" << ordering.get_blocks() << std::endl;

    read_state_recursively(tree_index, size, bit, ordering, tree_table, out_state);
}

/// @brief Read the `out_state` from the given `root_index` from the `root_table`.
/// @param root_index is the index of the slot in the root table.
/// @param tree_table is the tree table.
/// @param root_table is the root table.
/// @param out_state is the output state.
inline void read_state(const RootSlot& root_slot, const IndexedHashSet& tree_table, State& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    const auto [tree_index, size] = read_slot(root_slot.slot);
    const auto ordering = root_slot.get_ordering();

    read_state(tree_index, size, ordering, tree_table, out_state);
}

/**
 * ConstIterator
 */

struct Entry
{
    Index m_index;
    Index m_size;
    Index m_bit;
};

static thread_local SharedMemoryPool<std::vector<Entry>> s_stack_pool = SharedMemoryPool<std::vector<Entry>> {};

inline void copy(const std::vector<Entry>& src, std::vector<Entry>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

class const_iterator
{
private:
    const IndexedHashSet* m_tree_table;
    const Bitset* m_ordering;
    SharedMemoryPoolPtr<std::vector<Entry>> m_stack;
    Index m_value;

    static constexpr const Index END_POS = Index(-1);

    const_iterator(const IndexedHashSet* tree_table, const Bitset* ordering, SharedMemoryPoolPtr<std::vector<Entry>> stack, Index value) :
        m_tree_table(tree_table),
        m_ordering(ordering),
        m_stack(stack),
        m_value(value)
    {
    }

    const_iterator clone() const { return const_iterator(m_tree_table, m_ordering, m_stack.clone(), m_value); }

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

            auto [i1, i2] = read_slot(m_tree_table->get_slot(entry.m_index));

            const auto must_swap = m_ordering->get(entry.m_bit);

            if (must_swap)
                std::swap(i1, i2);

            Index mid = std::bit_floor(entry.m_size - 1);

            // Emplace right first to ensure left is visited first in dfs.
            m_stack->emplace_back(i2, entry.m_size - mid, 2 * entry.m_bit + 2);
            m_stack->emplace_back(i1, mid, 2 * entry.m_bit + 1);
        }

        m_value = END_POS;
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = Index;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    const_iterator() : m_tree_table(nullptr), m_stack(), m_value(END_POS) {}
    const_iterator(const IndexedHashSet* tree_table, const RootSlot* root, bool begin) :
        m_tree_table(tree_table),
        m_ordering(&root->get_ordering()),
        m_stack(),
        m_value(END_POS)
    {
        assert(m_tree_table && root);

        if (begin)
        {
            m_stack = s_stack_pool.get_or_allocate();
            m_stack->clear();

            const auto [tree_idx, size] = read_slot(root->get_slot());

            if (size > 0)  ///< Push to stack only if there leafs
            {
                m_stack->emplace_back(tree_idx, size, 0);
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
        auto tmp = clone();
        ++(*this);
        return tmp;
    }

    bool operator==(const const_iterator& other) const { return m_value == other.m_value; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
};

static_assert(std::input_iterator<const_iterator>);

inline const_iterator begin(const RootSlot& root, const IndexedHashSet& tree_table) { return const_iterator(&tree_table, &root, true); }

inline const_iterator end() { return const_iterator(); }

}

#endif