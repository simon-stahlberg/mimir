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

#include "valla/indexed_hash_set.hpp"
#include "valla/iterator.hpp"
#include "valla/succinct_indexed_hash_set.hpp"

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
inline auto insert_sequence_recursively(Iterator it, Iterator end, typename Set1::index_type size, Set1& table, Set2& leaf_table)
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
    const auto i1 = insert_sequence_recursively(it, mid_it, mid, table, leaf_table);
    const auto i2 = insert_sequence_recursively(mid_it, end, size - mid, table, leaf_table);

    return table.insert(Slot<I>(i1, i2));
}

template<std::ranges::input_range Range, IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2, std::ranges::range_value_t<Range>>
inline auto insert_sequence(const Range& sequence, Set1& table, Set2& leaf_table)
{
    using I = typename Set1::index_type;

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<I>(std::distance(sequence.begin(), sequence.end()));

    if (size == 0)
        return get_empty_slot<I>();  ///< Special case for empty sequence.

    return Slot<I>(insert_sequence_recursively(sequence.begin(), sequence.end(), size, table, leaf_table), size);
}

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2, std::output_iterator<typename Set2::value_type> OutIterator>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void
read_sequence_recursively(typename Set1::index_type index, typename Set1::index_type size, const Set1& table, const Set2& leaf_table, OutIterator out)
{
    /* Base case */
    if (size == 1)
    {
        *out++ = leaf_table.lookup(index);
        return;
    }

    const auto& slot = table.lookup(index);

    /* Base case */
    if (size == 2)
    {
        *out++ = leaf_table.lookup(slot.i1);
        *out++ = leaf_table.lookup(slot.i2);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_sequence_recursively(slot.i1, mid, table, leaf_table, out);
    read_sequence_recursively(slot.i2, size - mid, table, leaf_table, out);
}

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2, std::output_iterator<typename Set2::value_type> OutIterator>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void read_sequence(const Slot<typename Set1::index_type>& root_slot, const Set1& table, const Set2& leaf_table, OutIterator out)
{
    if (root_slot.i2 == 0)  ///< Special case for empty sequence.
        return;

    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_sequence_recursively(root_slot.i1, root_slot.i2, table, leaf_table, out);
}

///////////////////////////////////////////
/// Special case with Index range
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::input_iterator Iterator, IsStableIndexedHashSet Set>
    requires IsSpecialCaseHashSet<Set, std::iter_value_t<Iterator>>
inline auto insert_sequence_recursively(Iterator it, Iterator end, typename Set::index_type size, Set& table)
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
    const auto i1 = insert_sequence_recursively(it, mid_it, mid, table);
    const auto i2 = insert_sequence_recursively(mid_it, end, size - mid, table);

    return table.insert(Slot<I>(i1, i2));
}

template<std::ranges::input_range Range, IsStableIndexedHashSet Set>
    requires IsSpecialCaseHashSet<Set, std::ranges::range_value_t<Range>>
inline auto insert_sequence(const Range& sequence, Set& table)
{
    using I = Set::index_type;

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<I>(std::distance(sequence.begin(), sequence.end()));

    if (size == 0)
        return get_empty_slot<I>();  ///< Special case for empty sequence.

    return Slot<I>(insert_sequence_recursively(sequence.begin(), sequence.end(), size, table), size);
}

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set, std::output_iterator<typename Set::index_type> OutIterator>
inline void read_sequence_recursively(typename Set::index_type index, typename Set::index_type size, const Set& table, OutIterator out)
{
    /* Base case */
    if (size == 1)
    {
        *out++ = index;
        return;
    }

    const auto slot = table.lookup(index);

    /* Base case */
    if (size == 2)
    {
        *out++ = slot.i1;
        *out++ = slot.i2;
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_sequence_recursively(slot.i1, mid, table, out);
    read_sequence_recursively(slot.i2, size - mid, table, out);
}

template<IsStableIndexedHashSet Set, std::output_iterator<typename Set::index_type> OutIterator>
inline void read_sequence(const Slot<typename Set::index_type>& root_slot, const Set& table, OutIterator out)
{
    if (root_slot.i2 == 0)  ///< Special case for empty sequence.
        return;

    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_sequence_recursively(root_slot.i1, root_slot.i2, table, out);
}

}

#endif