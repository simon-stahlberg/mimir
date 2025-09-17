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
#include <ranges>

namespace valla
{
/// @brief `IsStableIndexedHashSet` is an indexed hash set whose indices remain stable.
template<class T>
concept IsStableIndexedHashSet =
    // associated types on the cvref-stripped type
    requires {
        typename std::remove_cvref_t<T>::value_type;
        typename std::remove_cvref_t<T>::index_type;
    } &&
    // index must be unsigned integral
    std::unsigned_integral<typename std::remove_cvref_t<T>::index_type> &&
    // readable on const U&
    requires(const std::remove_cvref_t<T>& ca, typename std::remove_cvref_t<T>::index_type i) {
        { ca.lookup(i) } -> std::convertible_to<typename std::remove_cvref_t<T>::value_type>;
    } &&
    // writable on non-const U&
    requires(std::remove_cvref_t<T>& a, typename std::remove_cvref_t<T>::value_type v) {
        { a.insert(v) } -> std::same_as<typename std::remove_cvref_t<T>::index_type>;
    };

/// @brief `IsUnstableIndexedHashSet` is an indexed hash set whose indices are unstable such that special root nodes need to stabilize them.
template<typename T>
concept IsUnstableIndexedHashSet =
    // associated types on the cvref-stripped type
    requires {
        typename std::remove_cvref_t<T>::value_type;
        typename std::remove_cvref_t<T>::index_type;
    } &&
    // index must be unsigned integral
    std::unsigned_integral<typename std::remove_cvref_t<T>::index_type> &&
    requires(const std::remove_cvref_t<T>& ca, typename std::remove_cvref_t<T>::index_type i) {
        { ca.lookup_internal(i) } -> std::convertible_to<typename std::remove_cvref_t<T>::value_type>;
        { ca.lookup_root(i) } -> std::convertible_to<typename std::remove_cvref_t<T>::value_type>;
    } &&
    // writable on non-const U&
    requires(std::remove_cvref_t<T>& a, typename std::remove_cvref_t<T>::value_type v, const std::vector<typename std::remove_cvref_t<T>::index_type>& s) {
        { a.resize_to_fit(s) } -> std::same_as<void>;
        { a.insert_internal(v) } -> std::same_as<typename std::remove_cvref_t<T>::index_type>;
        { a.insert_root(v) } -> std::same_as<typename std::remove_cvref_t<T>::index_type>;
    };
}
#endif