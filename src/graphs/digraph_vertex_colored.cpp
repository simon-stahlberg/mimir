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

#include "mimir/graphs/digraph_vertex_colored.hpp"

#include <algorithm>
#include <loki/loki.hpp>

namespace mimir
{

/**
 * ColoredDigraphVertex
 */

ColoredDigraphVertex::ColoredDigraphVertex(VertexIndex index, Color color) : m_index(index), m_color(color) {}

bool ColoredDigraphVertex::operator==(const ColoredDigraphVertex& other) const
{
    if (this != &other)
    {
        return (m_index == other.m_index) && (m_color == other.m_color);
    }
    return true;
}

size_t ColoredDigraphVertex::hash() const { return loki::hash_combine(m_index, m_color); }

VertexIndex ColoredDigraphVertex::get_index() const { return m_index; }

Color ColoredDigraphVertex::get_color() const { return m_color; }

/**
 * Utils
 */

ColorList compute_vertex_colors(const VertexColoredDigraph& graph)
{
    auto coloring = ColorList {};
    coloring.reserve(graph.get_num_vertices());
    for (const auto& vertex : graph.get_vertices())
    {
        coloring.push_back(vertex.get_color());
    }
    return coloring;
}

ColorList compute_sorted_vertex_colors(const VertexColoredDigraph& graph)
{
    auto coloring = compute_vertex_colors(graph);
    std::sort(coloring.begin(), coloring.end());
    return coloring;
}

}
