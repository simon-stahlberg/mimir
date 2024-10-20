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
#include "mimir/algorithms/nauty.hpp"
#include "mimir/graphs/declarations.hpp"

#include <nauty.h>
#include <sstream>
#include <string>
#include <vector>

namespace nauty_wrapper
{
class DenseGraphImpl
{
private:
    // num_vertices
    size_t n_;
    // vertex capacity
    size_t c_;
    // blocks_per_vertex
    size_t m_;
    // The input graph
    graph* graph_;
    bool use_default_ptn_;

    mimir::ColorList canon_coloring_;
    std::vector<int> lab_;
    std::vector<int> ptn_;

    // The canonical graph
    graph* canon_graph_;

    // Output streams
    std::stringstream canon_graph_repr_;
    std::stringstream canon_graph_compressed_repr_;

    void allocate_graph(graph** out_graph);
    void deallocate_graph(graph* out_graph);

public:
    explicit DenseGraphImpl(size_t num_vertices);
    DenseGraphImpl(const DenseGraphImpl& other);
    DenseGraphImpl& operator=(const DenseGraphImpl& other);
    DenseGraphImpl(DenseGraphImpl&& other) noexcept;
    DenseGraphImpl& operator=(DenseGraphImpl&& other) noexcept;
    ~DenseGraphImpl();

    void add_edge(size_t source, size_t target);

    void add_vertex_coloring(const mimir::ColorList& vertex_coloring);

    Certificate compute_certificate();

    void clear(size_t num_vertices);

    bool is_directed() const;

    bool has_loop() const;
};

}

#endif
