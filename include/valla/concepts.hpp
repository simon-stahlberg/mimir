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

#ifndef VALLA_INCLUDE_CONCEPTS_HPP_
#define VALLA_INCLUDE_CONCEPTS_HPP_

#include "valla/slot.hpp"

#include <concepts>

namespace valla
{
/// @brief `IsStableIndexedHashSet` is an indexed hash set whose indices remain stable.
template<typename T>
concept IsStableIndexedHashSet = requires(T a, typename T::value_type v, typename T::index_type i) {
    typename T::value_type;
    typename T::index_type;

    requires std::unsigned_integral<typename T::index_type>;

    { a.insert(v) } -> std::same_as<typename T::index_type>;
    { a.lookup(i) } -> std::convertible_to<typename T::value_type>;
};

/// @brief `IsUnstableIndexedHashSet` is an indexed hash set whose indices are unstable such that special root nodes need to stabilize them.
template<typename T>
concept IsUnstableIndexedHashSet = requires(T a, typename T::value_type v, typename T::index_type i) {
    typename T::value_type;
    typename T::index_type;

    requires std::unsigned_integral<typename T::index_type>;

    { a.insert_internal(v) } -> std::same_as<typename T::index_type>;
    { a.insert_root(v) } -> std::same_as<typename T::index_type>;
    { a.lookup_internal(i) } -> std::convertible_to<typename T::value_type>;
    { a.lookup_root(i) } -> std::convertible_to<typename T::value_type>;
};

/// @brief `AreGeneralCaseHashSets` ensures value and index type compatibility between an inner and a leaf indexed hash sets.
template<typename Set1, typename Set2, typename V = typename Set2::value_type>
concept AreGeneralCaseHashSets = std::same_as<V, typename Set2::value_type>                             //
                                 && std::same_as<typename Set1::index_type, typename Set2::index_type>  //
                                 && std::same_as<typename Set1::value_type, Slot<typename Set1::index_type>>;

///@brief `IsSpecialCaseHashSet` ensures value and index type compatibility between a indexed hash set.
template<typename Set, typename V = typename Set::value_type>
concept IsSpecialCaseHashSet = std::same_as<V, typename Set::index_type>  //
                               && std::same_as<typename Set::value_type, Slot<typename Set::index_type>>;
}

#endif