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
#include "mimir/graphs/concrete/digraph_vertex_colored.hpp"
#include "mimir/search/declarations.hpp"

#include <ostream>

namespace mimir::datasets
{

/**
 * ObjectGraph
 */

class ObjectGraph
{
private:
    graphs::StaticVertexColoredDigraph m_graph;

    formalism::ObjectMap<graphs::VertexIndex> m_object_to_v_idx;
    formalism::ToObjectMap<graphs::VertexIndex> m_v_idx_to_object;

public:
    /// @brief Create an `ObjectGraph` for a given `search::State` from a given `formalism::ProblemImpl` using the given `formalism::ColorFunctionImpl`.
    /// @param state is the state.
    /// @param problem is the Problem.
    /// @param color_function is the function used to color the vertices in the object graph.
    ObjectGraph(search::State state, const formalism::ProblemImpl& problem, const formalism::ColorFunctionImpl& color_function);

    /// @brief Apply a given permutation to a given list of objects.
    /// @param objects is the list of objects to permute.
    /// @param v_idx_permutation is the vertex index permutation.
    /// @return is the list of permuted objects.
    formalism::ObjectList apply_permutation(const formalism::ObjectList& objects, const std::vector<int>& v_idx_permutation) const;

    /// @brief Get the underlying `graphs::StaticVertexColoredDigraph`.
    /// @return the underlying `graphs::StaticVertexColoredDigraph`.
    const graphs::StaticVertexColoredDigraph& get_graph() const;
};

}

#endif
