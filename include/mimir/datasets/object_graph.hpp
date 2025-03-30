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

#ifndef MIMIR_DATASETS_OBJECT_GRAPH_HPP_
#define MIMIR_DATASETS_OBJECT_GRAPH_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/concrete/digraph_vertex_colored.hpp"
#include "mimir/search/declarations.hpp"

#include <ostream>

namespace mimir::datasets
{

/**
 * ObjectGraph
 */

/// @brief Create an object graph as a vertex colored graph for a given state.
/// It can be used for state equivalence testing or as input to graph neural networks.
/// @param state is the state.
/// @param problem is the Problem.
/// @param pddl_repositories is the PDDLRepository.
/// @param color_function is the function used to color the vertices in the object graph.
/// @param mark_true_goal_literals represents whether literals that are true should have a special color.
/// @return a vertex colored graph that precisely represents the given state.
extern graphs::StaticVertexColoredDigraph create_object_graph(search::State state,
                                                              const formalism::ProblemImpl& problem,
                                                              const GeneralizedColorFunctionImpl& color_function,
                                                              bool mark_true_goal_literals = false);

}

#endif
