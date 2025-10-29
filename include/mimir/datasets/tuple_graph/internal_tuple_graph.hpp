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

#ifndef MIMIR_DATASETS_TUPLE_GRAPH_INTERNAL_TUPLE_GRAPH_HPP_
#define MIMIR_DATASETS_TUPLE_GRAPH_INTERNAL_TUPLE_GRAPH_HPP_

#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/algorithms/iw/types.hpp"

#include <ostream>
#include <vector>

namespace mimir::graphs
{
/// @brief `TupleGraphVertex` encapsulates information about a vertex in a tuple graph.
using TupleGraphVertex = Vertex<std::tuple<search::iw::AtomIndexList, IndexList>>;
using TupleGraphVertexList = std::vector<TupleGraphVertex>;

inline const search::iw::AtomIndexList& get_atom_tuple(const TupleGraphVertex& vertex) { return std::get<0>(vertex.get_properties()); }

inline const IndexList& get_problem_vertices(const TupleGraphVertex& vertex) { return std::get<1>(vertex.get_properties()); }

using StaticTupleGraph = StaticGraph<TupleGraphVertex, EmptyEdge>;
using InternalTupleGraph = StaticBidirectionalGraph<StaticTupleGraph>;
}

#endif