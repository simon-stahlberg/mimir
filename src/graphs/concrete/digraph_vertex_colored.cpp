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

#include "mimir/graphs/concrete/digraph_vertex_colored.hpp"

#include <algorithm>

namespace mimir::graphs
{

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const StaticVertexColoredDigraph& graph)
{
    out << "digraph {\n";
    for (const auto& vertex : graph.get_vertices())
    {
        out << "t" << vertex.get_index() << "[";
        out << "label=\"" << get_color(vertex) << "\"]\n";
    }
    for (const auto& vertex : graph.get_vertices())
    {
        for (const auto& succ_vertex : graph.template get_adjacent_vertices<ForwardTag>(vertex.get_index()))
        {
            out << "t" << vertex.get_index() << "->" << "t" << succ_vertex.get_index() << "\n";
        }
    }
    out << "}";  // end digraph

    return out;
}

}
