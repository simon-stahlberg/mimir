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

#include "mimir/common/declarations.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::graphs
{

/**
 * ClassGraph
 */

/// @brief Get the index of the representative `ProblemVertex` in the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the representative `ProblemVertex` in the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_problem_vertex_index(const ClassVertex& vertex) { return std::get<0>(vertex.get_properties()); }

/// @brief Get the index of the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_problem_index(const ClassVertex& vertex) { return std::get<1>(vertex.get_properties()); }

/// @brief Get the index of the representative `ProblemEdge` in the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the representative `ProblemEdge` in the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_problem_edge_index(const ClassEdge& edge) { return std::get<0>(edge.get_properties()); }

/// @brief Get the index of the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_problem_index(const ClassEdge& edge) { return std::get<1>(edge.get_properties()); }
}

#endif
