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

#ifndef MIMIR_GRAPHS_DIGRAPH_HPP_
#define MIMIR_GRAPHS_DIGRAPH_HPP_

#include "mimir/algorithms/nauty.hpp"

#include <span>
#include <vector>

namespace mimir
{

class Digraph
{
private:
    int m_num_vertices;

    int m_num_edges;
    bool m_is_directed;

    // Never deallocate memory, getters return a span on the relevant part.
    std::vector<std::vector<int>> m_forward_successors;
    std::vector<std::vector<int>> m_backward_successors;

public:
    explicit Digraph(bool is_directed = false);
    Digraph(int num_vertices, bool is_directed = false);

    /// @brief Add an edge to the graph.
    void add_edge(int src, int dst);

    /// @brief Reinitialize the graph to an empty graph with num_vertices many vertices.
    void reset(int num_vertices, bool is_directed = false);

    /// @brief Get a nauty graph representation.
    void to_nauty_graph(nauty_wrapper::Graph& out_graph) const;

    /**
     * Getters
     */
    int get_num_vertices() const;
    int get_num_edges() const;
    std::span<const std::vector<int>> get_forward_successors() const;
    std::span<const std::vector<int>> get_backward_successors() const;
};

}
#endif
