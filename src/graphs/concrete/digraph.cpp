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

#include "mimir/graphs/concrete/digraph.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

namespace mimir::graphs
{

template class StaticGraph<EmptyVertex, EmptyEdge>;
template class StaticForwardGraph<StaticGraph<EmptyVertex, EmptyEdge>>;
template class StaticBidirectionalGraph<StaticGraph<EmptyVertex, EmptyEdge>>;

template class DynamicGraph<EmptyVertex, EmptyEdge>;

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const StaticDigraph& digraph)
{
    out << "digraph {\n";
    for (const auto& vertex : digraph.get_vertices())
    {
        out << "t" << vertex.get_index() << "[";
        out << "label=\"" << vertex.get_index() << "\"]\n";
    }
    for (const auto& vertex : digraph.get_vertices())
    {
        for (const auto& succ_vertex : digraph.template get_adjacent_vertices<ForwardTag>(vertex.get_index()))
        {
            out << "t" << vertex.get_index() << "->" << "t" << succ_vertex.get_index() << "\n";
        }
    }
    out << "}";  // end digraph

    return out;
}

}
