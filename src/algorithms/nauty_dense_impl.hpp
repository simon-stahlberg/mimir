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

#ifndef SRC_ALGORITHMS_NAUTY_DENSE_IMPL_HPP_
#define SRC_ALGORITHMS_NAUTY_DENSE_IMPL_HPP_

// Only include nauty_impl.hpp in a source file to avoid transitive includes of nauty.h.
#include <mimir/graphs/partitioning.hpp>
#include <nauty.h>
#include <string>
#include <vector>

namespace nauty_wrapper
{
class DenseGraphImpl
{
private:
    // num_vertices
    int n_;
    // vertex capacity
    int c_;
    // blocks_per_vertex
    int m_;
    // Whether the graph is directed.
    bool is_directed_;
    // Whether a certificate was obtained from the graph.
    bool obtained_certificate_;

    graph* graph_;
    graph* canon_graph_;

    void allocate_graph(graph** out_graph);
    void deallocate_graph(graph* out_graph);

public:
    DenseGraphImpl(int num_vertices, bool is_directed = false);
    DenseGraphImpl(const DenseGraphImpl& other);
    DenseGraphImpl& operator=(const DenseGraphImpl& other);
    DenseGraphImpl(DenseGraphImpl&& other) noexcept;
    DenseGraphImpl& operator=(DenseGraphImpl&& other) noexcept;
    ~DenseGraphImpl();

    void add_edge(int source, int target);

    std::string compute_certificate(const mimir::Partitioning& partitioning);

    void reset(int num_vertices, bool is_directed = false);

    bool is_directed() const;
};

}

#endif
