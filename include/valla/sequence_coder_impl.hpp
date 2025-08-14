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

#ifndef VALLA_INCLUDE_SEQUENCE_CODER_IMPL_HPP_
#define VALLA_INCLUDE_SEQUENCE_CODER_IMPL_HPP_

#include "valla/concepts.hpp"

#include <concepts>
#include <iterator>
#include <ranges>

namespace valla
{

template<std::ranges::input_range Range, IsStableIndexedHashSet Set, std::output_iterator<typename Set::index_type> OutIterator>
    requires std::same_as<std::ranges::range_value_t<Range>, typename Set::value_type>
inline void encode_as_unsigned_integrals(const Range& sequence, Set& table, OutIterator out)
{
    for (const auto& element : sequence)
        *out++ = table.insert(element);
}

template<std::ranges::input_range Range, IsStableIndexedHashSet Set, std::output_iterator<typename Set::value_type> OutIterator>
    requires std::same_as<std::ranges::range_value_t<Range>, typename Set::index_type>
inline void decode_from_unsigned_integrals(const Range& sequence, Set& table, OutIterator out)
{
    for (const auto& index : sequence)
        *out++ = table.lookup(index);
}

}

#endif