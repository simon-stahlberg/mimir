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

#ifndef MIMIR_GRAPHS_DIGRAPH_HPP_
#define MIMIR_GRAPHS_DIGRAPH_HPP_

#include "mimir/graphs/dynamic_graph.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"

namespace mimir::graphs
{
/**
 * Type aliases
 */

using StaticDigraph = StaticGraph<Vertex<>, Edge<>>;
using StaticForwardDigraph = StaticForwardGraph<StaticGraph<Vertex<>, Edge<>>>;
using StaticBidirectionalDigraph = StaticBidirectionalGraph<StaticGraph<Vertex<>, Edge<>>>;

using DynamicDigraph = DynamicGraph<Vertex<>, Edge<>>;

/**
 * Static graph assertions
 */

static_assert(IsStaticGraph<StaticDigraph>);
static_assert(IsStaticGraph<StaticForwardDigraph>);
static_assert(IsStaticGraph<StaticBidirectionalDigraph>);

/**
 * Dynamic graph assertions
 */

static_assert(IsDynamicGraph<DynamicDigraph>);

}
#endif
