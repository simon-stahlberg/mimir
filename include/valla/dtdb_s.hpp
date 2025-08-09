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

#ifndef VALLA_INCLUDE_DTDB_S_HPP_
#define VALLA_INCLUDE_DTDB_S_HPP_

#include "valla/concepts.hpp"

namespace valla
{

///////////////////////////////////////////
/// General case with special leaf table
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::ranges::input_range Range, IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2, std::ranges::range_value_t<Range>>
inline auto insert_sequence(const Range& sequence, Set1& table, Set2& leaf_table);

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set1, IsStableIndexedHashSet Set2, std::output_iterator<typename Set2::value_type> OutIterator>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void read_sequence(const Slot<typename Set1::index_type>& root_slot, const Set1& table, const Set2& leaf_table, OutIterator out);

///////////////////////////////////////////
/// Special case with Index range
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::ranges::input_range Range, IsStableIndexedHashSet Set>
    requires IsSpecialCaseHashSet<Set, std::ranges::range_value_t<Range>>
inline auto insert_sequence(const Range& sequence, Set& table);

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set, std::output_iterator<typename Set::index_type> OutIterator>
inline void read_sequence(const Slot<typename Set::index_type>& root_slot, const Set& table, OutIterator out);

}

#include "valla/dtdb_s_impl.hpp"

#endif