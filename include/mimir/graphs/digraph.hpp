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

#include <vector>

namespace mimir
{

class Digraph
{
private:
    int m_num_vertices;
    int m_num_edges;

    std::vector<std::vector<int>> m_adjacency_lists;

public:
    explicit Digraph(bool is_directed = false);
    Digraph(int num_vertices, bool is_directed = false);

    void add_edge(int src, int dst);

    int get_num_vertices() const;
    int get_num_edges() const;
    const std::vector<std::vector<int>>& get_adjacency_lists() const;

    /// @brief Get a nauty graph representation.
    void to_nauty_graph(nauty_wrapper::Graph& out) const;
};

}
#endif