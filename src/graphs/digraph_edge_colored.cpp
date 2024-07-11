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

#include "mimir/graphs/digraph_edge_colored.hpp"

#include <algorithm>
#include <loki/loki.hpp>

namespace mimir
{

/**
 * ColoredDigraphEdge
 */

ColoredDigraphEdge::ColoredDigraphEdge(EdgeIndex index, VertexIndex source, VertexIndex target, Color color) :
    m_index(index),
    m_source(source),
    m_target(target),
    m_color(color)
{
}

EdgeIndex ColoredDigraphEdge::get_index() const { return m_index; }

VertexIndex ColoredDigraphEdge::get_source() const { return m_source; }

VertexIndex ColoredDigraphEdge::get_target() const { return m_target; }

Color ColoredDigraphEdge::get_color() const { return m_color; }

}
