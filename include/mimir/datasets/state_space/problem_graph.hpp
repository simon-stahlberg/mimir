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

#include "mimir/common/declarations.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir::graphs
{

/**
 * ProblemGraph
 */

/// @brief Get the `PackedState` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `PackedState` of the given `ProblemVertex` in the `ProblemGraph`.
inline search::PackedState get_packed_state(const ProblemVertex& vertex) { return std::get<0>(vertex.get_properties()); }

/// @brief Get the `StateRepository` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `StateRepository` of the given `ProblemVertex` in the `ProblemGraph`.
inline const search::StateRepository& get_state_repository(const ProblemVertex& vertex) { return std::get<1>(vertex.get_properties()); }

/// @brief Get the `State` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `State` of the given `ProblemVertex` in the `ProblemGraph`.
inline search::State get_state(const ProblemVertex& vertex) { return get_state_repository(vertex)->get_state(*get_packed_state(vertex)); }

/// @brief Get the `formalism::Problem` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `formalism::Problem` of the given `ProblemVertex` in the `ProblemGraph`.
inline const formalism::Problem& get_problem(const ProblemVertex& vertex) { return get_state_repository(vertex)->get_problem(); }

/// @brief Get the unit goal distance of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the unit goal distance of the given `ProblemVertex`.
inline DiscreteCost get_unit_goal_distance(const ProblemVertex& vertex) { return std::get<2>(vertex.get_properties()); }

/// @brief Get the action cost goal distance of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`
/// @return the action cost goal distance of the given `ProblemVertex`.
inline ContinuousCost get_action_goal_distance(const ProblemVertex& vertex) { return std::get<3>(vertex.get_properties()); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an initial vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an initial vertex, and false otherwise.
inline bool is_initial(const ProblemVertex& vertex) { return std::get<4>(vertex.get_properties()); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is a goal vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is a goal vertex, and false otherwise.
inline bool is_goal(const ProblemVertex& vertex) { return std::get<5>(vertex.get_properties()); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an unsolvable vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an unsolvable vertex, and false otherwise.
inline bool is_unsolvable(const ProblemVertex& vertex) { return std::get<6>(vertex.get_properties()); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an alive vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an alive vertex, and false otherwise.
inline bool is_alive(const ProblemVertex& vertex) { return std::get<7>(vertex.get_properties()); }

/// @brief Get the `GroundAction` of the given `ProblemEdge`.
/// @param edge is a `ProblemEdge`.
/// @return the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline formalism::GroundAction get_action(const ProblemEdge& edge) { return std::get<0>(edge.get_properties()); }

/// @brief Get the `formalism::Problem` of the given `ProblemEdge`.
/// @param vertex is a `ProblemEdge`.
/// @return the `formalism::Problem` of the given `ProblemEdge` in the `ProblemGraph`.
inline const formalism::Problem& get_problem(const ProblemEdge& edge) { return std::get<1>(edge.get_properties()); }

/// @brief Get the action cost associated with the `GroundAction` of the given `ProblemEdge`
/// @param edge is a `ProblemEdge`.
/// @return the action cost of associated with the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline ContinuousCost get_action_cost(const ProblemEdge& edge) { return std::get<2>(edge.get_properties()); }
}

#endif
