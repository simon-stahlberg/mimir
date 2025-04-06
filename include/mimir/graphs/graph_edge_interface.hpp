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

#ifndef MIMIR_GRAPHS_GRAPH_EDGE_INTERFACE_HPP_
#define MIMIR_GRAPHS_GRAPH_EDGE_INTERFACE_HPP_

#include "mimir/graphs/types.hpp"

#include <concepts>

namespace mimir::graphs
{

template<typename T>
concept IsEdge = requires(T a) {
    typename T::EdgePropertiesTypes;

    { a.get_index() } -> std::convertible_to<EdgeIndex>;
    { a.get_source() } -> std::convertible_to<VertexIndex>;
    { a.get_target() } -> std::convertible_to<VertexIndex>;
};

/// Check whether `T` is an Edge with the given `EdgeProperties`s.
template<typename T, typename... EdgeProperties>
concept HasEdgeProperties = IsEdge<T> && std::is_same_v<typename T::EdgePropertiesTypes, std::tuple<std::decay_t<EdgeProperties>...>>;

}

#endif
