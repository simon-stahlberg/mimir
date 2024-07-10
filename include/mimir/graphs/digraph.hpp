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

#include "mimir/graphs/digraph_edge.hpp"
#include "mimir/graphs/digraph_interface.hpp"
#include "mimir/graphs/digraph_iterators.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

/* Digraph */

class Digraph
{
private:
    size_t m_num_vertices;
    bool m_is_directed;

    DigraphEdgeList m_edges;

public:
    using EdgeType = DigraphEdge;

    explicit Digraph(bool is_directed = false);
    Digraph(size_t num_vertices, bool is_directed = false);

    /// @brief Extend the current number of vertices beyond the current number of vertices.
    void increase_num_vertices(size_t new_num_vertices);

    /// @brief Add an edge to the graph and return the index to it.
    /// If the graph is undirected, then the backward edge has index + 1.
    DigraphEdgeIndex add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight = 1.);

    /// @brief Reinitialize the graph to an empty graph with num_vertices many vertices.
    void reset(size_t num_vertices, bool is_directed = false);

    /**
     * Iterators
     */

    DigraphTargetIndexIterator<DigraphEdge> get_targets(DigraphVertexIndex source) const;
    DigraphSourceIndexIterator<DigraphEdge> get_sources(DigraphVertexIndex target) const;
    DigraphForwardEdgeIndexIterator<DigraphEdge> get_forward_edge_indices(DigraphVertexIndex source) const;
    DigraphBackwardEdgeIndexIterator<DigraphEdge> get_backward_edge_indices(DigraphVertexIndex target) const;
    DigraphForwardEdgeIterator<DigraphEdge> get_forward_edges(DigraphVertexIndex source) const;
    DigraphBackwardEdgeIterator<DigraphEdge> get_backward_edges(DigraphVertexIndex target) const;

    /**
     * Getters
     */
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
    bool is_directed() const;
    const DigraphEdgeList& get_edges() const;
};

static_assert(IsDigraph<Digraph>);

/* ForwardDigraph */

/* BidirectionalDigraph */

}
#endif
