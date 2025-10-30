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

#ifndef MIMIR_GRAPHS_FORMATTER_IMPL_HPP_
#define MIMIR_GRAPHS_FORMATTER_IMPL_HPP_

#include "mimir/graphs/formatter_decl.hpp"

namespace mimir
{
namespace graphs
{
/**
 * Declarations
 */

template<Property P>
std::ostream& operator<<(std::ostream& os, const Vertex<P>& vertex)
{
    return mimir::print(os, vertex);
}

template<Property P>
std::ostream& operator<<(std::ostream& os, const Edge<P>& edge)
{
    return mimir::print(os, edge);
}

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const StaticGraph<V, E>& graph)
{
    return mimir::print(out, graph);
}

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticForwardGraph<G>& graph)
{
    return mimir::print(out, graph);
}

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticBidirectionalGraph<G>& graph)
{
    return mimir::print(out, graph);
}

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const DynamicGraph<V, E>& graph)
{
    return mimir::print(out, graph);
}
}  // end graphs

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticGraph<V, E>& graph)
{
    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

    /* Node definitions */
    for (const auto& vertex : graph.get_vertices())
    {
        out << "n" << vertex.get_index() << " [label=\"" << vertex << "\"];\n";
    }
    out << "\n";

    /* Edge definitions */
    for (const auto& edge : graph.get_edges())
    {
        out << "n" << edge.get_source() << " -> " << "n" << edge.get_target() << " [label=\"" << edge << "\"];\n";
    }
    out << "\n";

    out << "}\n";  // end graph

    return out;
}

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticForwardGraph<G>& graph)
{
    out << graph.get_graph();
    return out;
}

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticBidirectionalGraph<G>& graph)
{
    out << graph.get_graph();
    return out;
}

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::DynamicGraph<V, E>& graph)
{
    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

    /* Node definitions */
    for (const auto& [v_idx, v] : graph.get_vertices())
    {
        out << "n" << v.get_index() << " [label=\"" << v << "\"];\n";
    }
    out << "\n";

    /* Edge definitions */
    for (const auto& [e_idx, e] : graph.get_edges())
    {
        out << "n" << e.get_source() << " -> " << "n" << e.get_target() << " [label=\"" << e << "\"];\n";
    }
    out << "\n";

    out << "}\n";  // end graph

    return out;
}

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& os, const mimir::graphs::Vertex<P>& vertex)
{
    os << "index=" << vertex.get_index() << ", properties=";
    os << vertex.get_properties();
    return os;
}

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& os, const mimir::graphs::Edge<P>& edge)
{
    os << "index=" << edge.get_index() << ", properties=";
    os << edge.get_properties();
    return os;
}
}

#endif