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

#ifndef MIMIR_DATASETS_STATE_SPACE_PROBLEM_GRAPH_HPP_
#define MIMIR_DATASETS_STATE_SPACE_PROBLEM_GRAPH_HPP_

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
 * ProblemGraph
 */

/// @typedef ProblemVertex
/// @brief `ProblemVertex` encapsulates information about a vertex in a `ProblemGraph`
using ProblemVertex = Vertex<search::State, formalism::Problem, DiscreteCost, ContinuousCost, bool, bool, bool, bool>;
using ProblemVertexList = std::vector<ProblemVertex>;

/// @brief Get the `State` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `State` of the given `ProblemVertex` in the `ProblemGraph`.
inline search::State get_state(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

/// @brief Get the `formalism::Problem` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `formalism::Problem` of the given `ProblemVertex` in the `ProblemGraph`.
inline const formalism::Problem& get_problem(const ProblemVertex& vertex) { return vertex.get_property<1>(); }

/// @brief Get the unit goal distance of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the unit goal distance of the given `ProblemVertex`.
inline DiscreteCost get_unit_goal_distance(const ProblemVertex& vertex) { return vertex.get_property<2>(); }

/// @brief Get the action cost goal distance of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`
/// @return the action cost goal distance of the given `ProblemVertex`.
inline ContinuousCost get_action_goal_distance(const ProblemVertex& vertex) { return vertex.get_property<3>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an initial vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an initial vertex, and false otherwise.
inline bool is_initial(const ProblemVertex& vertex) { return vertex.get_property<4>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is a goal vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is a goal vertex, and false otherwise.
inline bool is_goal(const ProblemVertex& vertex) { return vertex.get_property<5>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an unsolvable vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an unsolvable vertex, and false otherwise.
inline bool is_unsolvable(const ProblemVertex& vertex) { return vertex.get_property<6>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an alive vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an alive vertex, and false otherwise.
inline bool is_alive(const ProblemVertex& vertex) { return vertex.get_property<7>(); }

/// @typedef ProblemEdge
/// @brief `ProblemEdge` encapsulates information about an edge in a `ProblemGraph`.
using ProblemEdge = Edge<formalism::GroundAction, formalism::Problem, ContinuousCost>;
using ProblemEdgeList = std::vector<ProblemEdge>;

/// @brief Get the `GroundAction` of the given `ProblemEdge`.
/// @param edge is a `ProblemEdge`.
/// @return the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline formalism::GroundAction get_action(const ProblemEdge& edge) { return edge.get_property<0>(); }

/// @brief Get the `formalism::Problem` of the given `ProblemEdge`.
/// @param vertex is a `ProblemEdge`.
/// @return the `formalism::Problem` of the given `ProblemEdge` in the `ProblemGraph`.
inline const formalism::Problem& get_problem(const ProblemEdge& edge) { return edge.get_property<1>(); }

/// @brief Get the action cost associated with the `GroundAction` of the given `ProblemEdge`
/// @param edge is a `ProblemEdge`.
/// @return the action cost of associated with the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline ContinuousCost get_action_cost(const ProblemEdge& edge) { return edge.get_property<2>(); }

using StaticProblemGraph = StaticGraph<ProblemVertex, ProblemEdge>;
using StaticProblemGraphList = std::vector<StaticProblemGraph>;
/// @typedef ProblemGraph
/// @brief `ProblemGraph` implements a directed graph representing the state space of a single problem.
using ProblemGraph = StaticBidirectionalGraph<StaticProblemGraph>;
using ProblemGraphList = std::vector<ProblemGraph>;

/// @brief Write a string representation of the given `ProblemVertex` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ProblemVertex`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex);

/// @brief Write a string representation of the given `ProblemEdge` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ProblemEdge`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge);

}

#endif
