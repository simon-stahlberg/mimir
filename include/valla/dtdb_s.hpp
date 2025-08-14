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

/**
 * Insert recursively
 */

template<std::ranges::forward_range Range, IsStableIndexedHashSet Set>
    requires std::same_as<std::ranges::range_value_t<Range>, typename Set::index_type>  //
             && std::same_as<typename Set::value_type, Slot<typename Set::index_type>>
inline auto insert_sequence(const Range& sequence, Set& table);

/**
 * Read recursively
 */

template<IsStableIndexedHashSet Set, std::output_iterator<typename Set::index_type> OutIterator>
inline void read_sequence(const Slot<typename Set::index_type>& root_slot, const Set& table, OutIterator out);

}

#include "valla/dtdb_s_impl.hpp"

#endif