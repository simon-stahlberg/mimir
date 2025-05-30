/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#ifndef MIMIR_SEARCH_OPENLISTS_INTERFACE_HPP_
#define MIMIR_SEARCH_OPENLISTS_INTERFACE_HPP_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace mimir::search
{

template<typename T>
concept IsOpenList = requires(T a, typename T::KeyType key, typename T::ItemType item) {
    requires std::totally_ordered<typename T::KeyType>;

    { a.insert(key, item) } -> std::same_as<void>;
    { a.top() } -> std::convertible_to<typename T::ItemType>;
    { a.pop() } -> std::same_as<void>;
    { a.clear() } -> std::same_as<void>;
    { a.empty() } -> std::same_as<bool>;
    { a.size() } -> std::same_as<std::size_t>;
};

template<typename First, typename... Rest>
concept HaveSameItemType = (... && std::same_as<typename First::ItemType, typename Rest::ItemType>);

template<typename T>
concept IsOpenListComposition = requires(T a, typename T::ItemType item) {
    { a.top() } -> std::convertible_to<typename T::ItemType>;
    { a.pop() } -> std::same_as<void>;
    { a.clear() } -> std::same_as<void>;
    { a.empty() } -> std::same_as<bool>;
    { a.size() } -> std::same_as<std::size_t>;
};

}

#endif