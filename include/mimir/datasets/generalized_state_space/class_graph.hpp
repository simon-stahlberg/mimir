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

#ifndef MIMIR_DATASETS_GENERALIZED_STATE_SPACE_CLASS_GRAPH_HPP_
#define MIMIR_DATASETS_GENERALIZED_STATE_SPACE_CLASS_GRAPH_HPP_

#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/graphs/types.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::graphs
{

/**
 * ClassGraph
 */

/// @typedef ClassVertex
/// @brief `ClassVertex` encapsulates information about a vertex in a `ClassGraph`.
using ClassVertex = Vertex<Index, Index>;
using ClassVertexList = std::vector<ClassVertex>;

/// @brief Get the index of the representative `ProblemVertex` in the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the representative `ProblemVertex` in the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_problem_vertex_index(const ClassVertex& vertex) { return vertex.get_property<0>(); }

/// @brief Get the index of the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_problem_index(const ClassVertex& vertex) { return vertex.get_property<1>(); }

/// @typedef ClassEdge
/// @brief `ClassEdge` encapsulates information about an edge in a `ClassGraph`.
using ClassEdge = Edge<Index, Index>;
using ClassEdgeList = std::vector<ClassEdge>;

/// @brief Get the index of the representative `ProblemEdge` in the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the representative `ProblemEdge` in the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_problem_edge_index(const ClassEdge& edge) { return edge.get_property<0>(); }

/// @brief Get the index of the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_problem_index(const ClassEdge& edge) { return edge.get_property<1>(); }

using StaticClassGraph = StaticGraph<ClassVertex, ClassEdge>;
/// @typedef ClassGraph
/// @brief `ClassGraph` implements a directed graph representing the state space of a class of problems.
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

/// @brief Write a string representation of the given `ClassVertex` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ClassVertex`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex);

/// @brief Write a string representation of the given `ClassEdge` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ClassEdge`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ClassEdge& edge);
}

#endif
