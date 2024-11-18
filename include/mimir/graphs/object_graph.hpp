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

#ifndef MIMIR_GRAPHS_OBJECT_GRAPH_HPP_
#define MIMIR_GRAPHS_OBJECT_GRAPH_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/object_graph_pruning_strategy.hpp"
#include "mimir/search/state.hpp"

#include <ostream>

namespace mimir
{

/**
 * ObjectGraph
 */

/// @brief Create an object graph as a vertex colored graph for a given state.
/// @param color_function is the function used to color the vertices in the object graph.
/// @param pddl_repositories is the PDDLFactory.
/// @param problem is the Problem.
/// @param state is the state.
/// @param mark_true_goal_literals represents whether literals that are true should have a special color.
/// @param pruning_strategy is the strategy used for pruning information from the object graph.
/// @return a vertex colored graph that represents the object graph.
extern StaticVertexColoredDigraph create_object_graph(const ProblemColorFunction& color_function,
                                                      const PDDLRepositories& pddl_repositories,
                                                      Problem problem,
                                                      State state,
                                                      Index state_index = 0,
                                                      bool mark_true_goal_literals = false,
                                                      const ObjectGraphPruningStrategy& pruning_strategy = ObjectGraphPruningStrategy());

}

#endif
