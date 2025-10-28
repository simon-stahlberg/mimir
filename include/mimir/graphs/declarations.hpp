/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_GRAPHS_DECLARATIONS_HPP_
#define MIMIR_GRAPHS_DECLARATIONS_HPP_

#include <concepts>
#include <cstddef>
#include <loki/loki.hpp>
#include <tuple>
#include <vector>

namespace mimir::graphs
{
template<class T>
concept Property = requires(const T& a, const T& b) {
    { loki::Hash<T> {}(a) } -> std::convertible_to<std::size_t>;
    { loki::EqualTo<T> {}(a, b) } -> std::same_as<bool>;
};

template<Property... Ts>
using Properties = std::tuple<Ts...>;
template<Property... Ts>
using PropertiesList = std::vector<Properties<Ts...>>;
}

#endif