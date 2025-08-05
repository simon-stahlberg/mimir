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

#ifndef VALLA_INCLUDE_DTDB_H_HPP_
#define VALLA_INCLUDE_DTDS_H_HPP_

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

namespace valla
{

///////////////////////////////////////////
/// General case with special leaf table
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::ranges::input_range Range, IsUnstableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2, std::ranges::range_value_t<Range>>
auto insert(const Range& state, Set1& inner_table, Set2& leaf_table);

/**
 * Read recursively
 */

template<IsUnstableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline void read_state(typename Set1::index_type root_index, const Set1& tree_table, const Set2& leaf_table, std::vector<typename Set2::value_type>& out_state);

/**
 * ConstIterator
 */

template<IsUnstableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline auto begin(typename Set1::index_type root, const Set1& inner_table, const Set2& leaf_table);

template<IsUnstableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline auto end(const Set1&, const Set2&);

template<IsUnstableIndexedHashSet Set1, IsStableIndexedHashSet Set2>
    requires AreGeneralCaseHashSets<Set1, Set2>
inline auto range(typename Set1::index_type root, const Set1& inner_table, const Set2& leaf_table);

///////////////////////////////////////////
/// Special case with Index range
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::ranges::input_range Range, IsUnstableIndexedHashSet Set>
    requires IsSpecialCaseHashSet<Set, std::ranges::range_value_t<Range>>
auto insert(const Range& state, Set& table);

/**
 * Read recursively
 */

template<IsUnstableIndexedHashSet Set>
inline void read_state(typename Set::index_type root_index, const Set& table, std::vector<typename Set::index_type>& out_state);

/**
 * ConstIterator
 */

template<IsUnstableIndexedHashSet Set>
inline auto begin(typename Set::index_type root, const Set& table);

template<IsUnstableIndexedHashSet Set>
inline auto end(const Set&);

template<IsUnstableIndexedHashSet Set>
inline auto range(typename Set::index_type root, const Set& table);

}

#include "valla/dtdb_h_impl.hpp"

#endif