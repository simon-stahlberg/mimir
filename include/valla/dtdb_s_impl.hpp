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
#include "valla/succinct_indexed_hash_set.hpp"

namespace valla
{

/**
 * Insert recursively
 */

template<std::forward_iterator Iterator, IsStableIndexedHashSet Set>
    requires std::same_as<std::iter_value_t<Iterator>, typename Set::index_type>  //
             && std::same_as<typename Set::value_type, Slot<typename Set::index_type>>
inline auto insert_sequence_recursively(Iterator it, Iterator end, typename Set::index_type size, Set& table)
{
    using I = Set::index_type;

    /* Base cases */
    if (size == I { 1 })
        return I(*it);  ///< Skip node creation

    if (size == I { 2 })
        return table.insert(Slot<I>(*it, *(it + 1)));

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto i1 = insert_sequence_recursively(it, mid_it, mid, table);
    const auto i2 = insert_sequence_recursively(mid_it, end, size - mid, table);

    return table.insert(Slot<I>(i1, i2));
}

template<std::ranges::forward_range Range, IsStableIndexedHashSet Set>
    requires std::same_as<std::ranges::range_value_t<Range>, typename Set::index_type>  //
             && std::same_as<typename Set::value_type, Slot<typename Set::index_type>>
inline auto insert_sequence(const Range& sequence, Set& table)
{
    using I = Set::index_type;

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<I>(std::distance(sequence.begin(), sequence.end()));

    if (size == I { 0 })
        return Slot<I>();  ///< Special case for empty sequence.

    return Slot<I>(insert_sequence_recursively(sequence.begin(), sequence.end(), size, table), size);
}

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set, std::output_iterator<typename Set::index_type> OutIterator>
inline void read_sequence_recursively(typename Set::index_type index, typename Set::index_type size, const Set& table, OutIterator out)
{
    using I = typename Set::index_type;

    /* Base case */
    if (size == I { 1 })
    {
        *out++ = index;
        return;
    }

    const auto slot = table.lookup(index);

    /* Base case */
    if (size == I { 2 })
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
    using I = typename Set::index_type;

    if (root_slot.i2 == I { 0 })  ///< Special case for empty sequence.
        return;

    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_sequence_recursively(root_slot.i1, root_slot.i2, table, out);
}

}

#endif