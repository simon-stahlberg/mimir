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
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/static_graph.hpp"

namespace mimir
{
class DigraphVertex
{
private:
    VertexIndex m_index;

public:
    explicit DigraphVertex(VertexIndex index);

    bool operator==(const DigraphVertex& other) const;
    size_t hash() const;

    VertexIndex get_index() const;
};

class DigraphEdge
{
private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;

public:
    DigraphEdge(EdgeIndex index, VertexIndex source, VertexIndex target);

    bool operator==(const DigraphEdge& other) const;
    size_t hash() const;

    EdgeIndex get_index() const;
    VertexIndex get_source() const;
    VertexIndex get_target() const;
};

/**
 * Type aliases
 */

using StaticDigraph = StaticGraph<DigraphVertex, DigraphEdge>;
using StaticForwardDigraph = StaticForwardGraph<StaticGraph<DigraphVertex, DigraphEdge>>;
using StaticBidirectionalDigraph = StaticBidirectionalGraph<StaticGraph<DigraphVertex, DigraphEdge>>;

using DynamicDigraph = DynamicGraph<DigraphVertex, DigraphEdge>;

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

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const StaticDigraph& digraph);

}
#endif
