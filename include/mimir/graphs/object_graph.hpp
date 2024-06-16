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

#include "mimir/formalism/formalism.hpp"
#include "mimir/graphs/coloring.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{
class ObjectGraphFactory;

class ObjectGraph
{
private:
    std::shared_ptr<ProblemColorFunction> m_coloring_function;
    State m_state;

    Digraph m_digraph;

    ColorList m_vertex_colors;
    ColorList m_edge_colors;

public:
    ObjectGraph(std::shared_ptr<ProblemColorFunction> coloring_function, State state);

    const std::shared_ptr<ProblemColorFunction>& get_coloring_function() const;
    const Digraph& get_digraph() const;
    const ColorList& get_vertex_colors() const;
    const ColorList& get_edge_colors() const;
};

class ObjectGraphFactory
{
private:
    Problem m_problem;
    bool m_mark_true_goal_literals;

    std::shared_ptr<ProblemColorFunction> m_coloring_function;

public:
    ObjectGraphFactory(Problem problem, bool mark_true_goal_literals = false);

    /// @brief Create and return an object graph.
    ///
    /// TODO: we might want to return a const reference and reuse memory.
    /// Then the user can decide whether a copy and allocations are needed.
    ObjectGraph create(const State state);
};

}

#endif
