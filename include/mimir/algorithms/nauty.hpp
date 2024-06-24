/*
 * Copyright (C) 2023 Simon Stahlberg
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

#ifndef MIMIR_ALGORITHMS_NAUTY_HPP_
#define MIMIR_ALGORITHMS_NAUTY_HPP_

#include <memory>
#include <vector>

/// @brief Wrap a namespace around nauty's interface
namespace nauty_wrapper
{
class GraphImpl;

class Graph
{
private:
    std::unique_ptr<GraphImpl> m_impl;

public:
    Graph();
    explicit Graph(int num_vertices);
    Graph(const Graph& other);
    Graph& operator=(const Graph& other);
    Graph(Graph&& other) noexcept;
    Graph& operator=(Graph&& other) noexcept;
    ~Graph();

    void add_edge(int src, int dst);

    /// @brief Compute a certificate for the graph and the given vertex partitioning as lab and ptn.
    /// @param lab A permutation of the vertex ids.
    /// @param ptn Groups subsequent ids in lab as group where the first n-1 vertex ids
    /// of a group are marked as 1 and the last vertex id as 0, indicating the end of the group.
    std::string compute_certificate(const std::vector<int>& lab, const std::vector<int>& ptn) const;

    /// @brief Reinitialize the graph.
    void reset(int num_vertices);
};
}

#endif
