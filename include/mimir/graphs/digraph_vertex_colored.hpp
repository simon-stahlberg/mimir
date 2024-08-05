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

#include "mimir/graphs/color_function.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/graphs/dynamic_graph.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/static_graph.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * ColoredDigraphVertex
 */

class ColoredDigraphVertex
{
private:
    VertexIndex m_index;
    Color m_color;

public:
    ColoredDigraphVertex(VertexIndex index, Color color);

    bool operator==(const ColoredDigraphVertex& other) const;
    size_t hash() const;

    VertexIndex get_index() const;
    Color get_color() const;
};

/**
 * Type aliases
 */

using StaticVertexColoredDigraph = StaticGraph<ColoredDigraphVertex, DigraphEdge>;
using StaticVertexColoredForwardDigraph = StaticForwardGraph<StaticGraph<ColoredDigraphVertex, DigraphEdge>>;
using StaticVertexColoredBidirectionalDigraph = StaticBidirectionalGraph<StaticGraph<ColoredDigraphVertex, DigraphEdge>>;

using DynamicVertexColoredDigraph = DynamicGraph<ColoredDigraphVertex, DigraphEdge>;

/**
 * Static graph assertions
 */

static_assert(IsStaticGraph<StaticVertexColoredDigraph>);
static_assert(!IsDynamicGraph<StaticVertexColoredDigraph>);
static_assert(IsStaticGraph<StaticVertexColoredForwardDigraph>);
static_assert(!IsDynamicGraph<StaticVertexColoredForwardDigraph>);
static_assert(IsStaticGraph<StaticVertexColoredBidirectionalDigraph>);
static_assert(!IsDynamicGraph<StaticVertexColoredBidirectionalDigraph>);

/**
 * Dynamic graph assertions
 */

static_assert(!IsStaticGraph<DynamicVertexColoredDigraph>);
static_assert(IsDynamicGraph<DynamicVertexColoredDigraph>);

/**
 * Utils
 */

extern ColorList compute_vertex_colors(const StaticVertexColoredDigraph& graph);
extern ColorList compute_sorted_vertex_colors(const StaticVertexColoredDigraph& graph);

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const std::tuple<const StaticVertexColoredDigraph&, const ColorFunction&>& data);

}

#endif
