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

#include "mimir/datasets/declarations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/concrete/vertex_colored_graph.hpp"
#include "mimir/search/declarations.hpp"

#include <ostream>

namespace mimir::datasets
{

/// @brief Create an `ObjectGraph` for a given `search::State` from a given `formalism::ProblemImpl`.
/// @param state is the state.
/// @param problem is the Problem.
extern graphs::StaticVertexColoredGraph create_object_graph(search::State state, const formalism::ProblemImpl& problem);

}

#endif
