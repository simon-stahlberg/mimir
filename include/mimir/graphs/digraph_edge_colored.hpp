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

#ifndef MIMIR_GRAPHS_DIGRAPH_EDGE_COLORED_HPP_
#define MIMIR_GRAPHS_DIGRAPH_EDGE_COLORED_HPP_

#include "mimir/graphs/digraph_vertex_colored.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

class ColoredDigraphEdge
{
private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    Color m_color;

public:
    ColoredDigraphEdge(EdgeIndex index, VertexIndex source, VertexIndex target, Color color);

    EdgeIndex get_index() const;
    VertexIndex get_source() const;
    VertexIndex get_target() const;
    Color get_color() const;
};

using VertexColoredDigraph = Graph<ColoredDigraphVertex, ColoredDigraphEdge>;

static_assert(IsGraph<Digraph>);

}
#endif
