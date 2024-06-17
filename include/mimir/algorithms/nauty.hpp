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
    explicit Graph(int num_vertices);
    ~Graph();

    void add_edge(int src, int dst);

    std::string compute_certificate(const std::vector<std::vector<int>>& vertex_partitioning) const;
};
}

#endif
