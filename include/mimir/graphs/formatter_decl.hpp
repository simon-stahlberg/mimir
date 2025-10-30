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

#ifndef MIMIR_GRAPHS_FORMATTER_DECL_HPP_
#define MIMIR_GRAPHS_FORMATTER_DECL_HPP_

#include "mimir/graphs/declarations.hpp"

namespace mimir
{
namespace graphs
{
/**
 * Declarations
 */

template<Property P>
std::ostream& operator<<(std::ostream& os, const Vertex<P>& vertex);

template<Property P>
std::ostream& operator<<(std::ostream& os, const Edge<P>& edge);

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const StaticGraph<V, E>& graph);

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticForwardGraph<G>& graph);

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticBidirectionalGraph<G>& graph);

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const DynamicGraph<V, E>& graph);
}  // end graphs

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticGraph<V, E>& graph);

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticForwardGraph<G>& graph);

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticBidirectionalGraph<G>& graph);

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::DynamicGraph<V, E>& graph);

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& os, const mimir::graphs::Vertex<P>& vertex);

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& os, const mimir::graphs::Edge<P>& edge);
}

#endif