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

#ifndef MIMIR_GRAPHS_DIGRAPH_VERTEX_COLORED_HPP_
#define MIMIR_GRAPHS_DIGRAPH_VERTEX_COLORED_HPP_

#include "mimir/graphs/color.hpp"
#include "mimir/graphs/dynamic_graph.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"

namespace mimir::graphs
{

/**
 * Type aliases
 */

using StaticVertexColoredGraph = StaticGraph<ColoredVertex, EmptyEdge>;
using StaticVertexColoredForwardGraph = StaticForwardGraph<StaticGraph<ColoredVertex, EmptyEdge>>;
using StaticVertexColoredBidirectionalGraph = StaticBidirectionalGraph<StaticGraph<ColoredVertex, EmptyEdge>>;

using DynamicVertexColoredGraph = DynamicGraph<ColoredVertex, EmptyEdge>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const StaticVertexColoredGraph& graph);

}

#endif
