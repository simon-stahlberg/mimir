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

#ifndef SRC_ALGORITHMS_NAUTY_SPARSE_IMPL_HPP_
#define SRC_ALGORITHMS_NAUTY_SPARSE_IMPL_HPP_

// Only include nauty_sparse_impl.hpp in a source file to avoid transitive includes of nauty.h.
#include <mimir/graphs/partitioning.hpp>
#include <nausparse.h>
#include <nauty.h>
#include <string>
#include <vector>

namespace nauty_wrapper
{
class SparseGraphImpl
{
private:
    // num_vertices
    int n_;
    // vertex capacity
    int c_;
    // Whether the graph is directed.
    bool is_directed_;
    // Whether a certificate was obtained from the graph.
    bool obtained_certificate_;

    // Track existing edges to avoid duplicates
    std::vector<bool> m_adj_matrix_;

    sparsegraph graph_;
    sparsegraph canon_graph_;

    void copy_graph_data(const sparsegraph& in_graph, sparsegraph& out_graph) const;

    void initialize_graph_data(sparsegraph& out_graph) const;

    void allocate_graph(sparsegraph& out_graph) const;
    void deallocate_graph(sparsegraph& the_graph) const;

public:
    explicit SparseGraphImpl(int num_vertices, bool is_directed = false);
    SparseGraphImpl(const SparseGraphImpl& other);
    SparseGraphImpl& operator=(const SparseGraphImpl& other);
    SparseGraphImpl(SparseGraphImpl&& other) noexcept;
    SparseGraphImpl& operator=(SparseGraphImpl&& other) noexcept;
    ~SparseGraphImpl();

    void add_edge(int source, int target);

    std::string compute_certificate(const mimir::Partitioning& partitioning);

    void reset(int num_vertices, bool is_directed = false);

    bool is_directed() const;
};

extern std::ostream& operator<<(std::ostream& out, const sparsegraph& graph);

}

#endif
