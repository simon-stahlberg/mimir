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

#ifndef MIMIR_GRAPHS_STATIC_GRAPH_INTERFACE_HPP_
#define MIMIR_GRAPHS_STATIC_GRAPH_INTERFACE_HPP_

#include "mimir/graphs/graph_interface.hpp"

namespace mimir::graphs
{

/**
 * Static graphs do not allow for deletion of vertices and edges, allowing vector data structures.
 */

template<typename T>
concept IsStaticGraph = requires(T a) {
    requires IsVertexListGraph<T>;
    requires IsEdgeListGraph<T>;
    requires IsIncidenceGraph<T>;
    requires IsAdjacencyGraph<T>;
    requires IsBidirectionalGraph<T>;

    // Ensure that DynamicGraph uses std::vector to store vertices and edges.
    { a.get_vertices() } -> std::same_as<const std::vector<typename T::VertexType>&>;
    { a.get_edges() } -> std::same_as<const std::vector<typename T::EdgeType>&>;
};

}

#endif
