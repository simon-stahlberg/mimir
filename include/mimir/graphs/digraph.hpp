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

template<IsDigraphEdge Edge>
class Digraph
{
public:
    using EdgeType = Edge;
    using EdgeList = std::vector<EdgeType>;

private:
    size_t m_num_vertices;
    bool m_is_directed;

    EdgeList m_edges;

public:
    explicit Digraph(bool is_directed = false);
    Digraph(size_t num_vertices, bool is_directed = false);

    /// @brief Add an edge to the graph and return the index to it.
    /// If the graph is undirected, then the backward edge has index + 1.
    DigraphEdgeIndex add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight = 1.);

    /// @brief Reinitialize the graph to an empty graph with num_vertices many vertices.
    void reset(size_t num_vertices, bool is_directed = false);

    /**
     * Iterators
     */

    DigraphTargetIndexIterator<Edge> get_targets(DigraphVertexIndex source) const;
    DigraphSourceIndexIterator<Edge> get_sources(DigraphVertexIndex target) const;
    DigraphForwardEdgeIndexIterator<Edge> get_forward_edge_indices(DigraphVertexIndex source) const;
    DigraphBackwardEdgeIndexIterator<Edge> get_backward_edge_indices(DigraphVertexIndex target) const;
    DigraphForwardEdgeIterator<Edge> get_forward_edges(DigraphVertexIndex source) const;
    DigraphBackwardEdgeIterator<Edge> get_backward_edges(DigraphVertexIndex target) const;

    /**
     * Getters
     */
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
    bool is_directed() const;
    const EdgeList& get_edges() const;
};

static_assert(IsDigraph<Digraph<DigraphEdge>>);

/* ForwardDigraph */

/* BidirectionalDigraph */

/**
 * Implementations
 */

/* Digraph */

template<IsDigraphEdge Edge>
Digraph<Edge>::Digraph(bool is_directed) : m_num_vertices(0), m_is_directed(is_directed), m_edges()
{
}

template<IsDigraphEdge Edge>
Digraph<Edge>::Digraph(size_t num_vertices, bool is_directed) : m_num_vertices(num_vertices), m_is_directed(is_directed)
{
}

template<IsDigraphEdge Edge>
DigraphEdgeIndex Digraph<Edge>::add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight)
{
    if (source >= m_num_vertices || target >= m_num_vertices || source < 0 || target < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    const auto index = static_cast<DigraphEdgeIndex>(m_edges.size());
    m_edges.emplace_back(index, source, target, weight);
    if (!m_is_directed)
    {
        m_edges.emplace_back(index, target, source, weight);
    }
    return index;
}

template<IsDigraphEdge Edge>
void Digraph<Edge>::reset(size_t num_vertices, bool is_directed)
{
    m_num_vertices = num_vertices;
    m_is_directed = is_directed;
    m_edges.clear();
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge> Digraph<Edge>::get_targets(DigraphVertexIndex source) const
{
    return DigraphTargetIndexIterator<Edge>(source, m_edges);
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge> Digraph<Edge>::get_sources(DigraphVertexIndex target) const
{
    return DigraphSourceIndexIterator<Edge>(target, m_edges);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge> Digraph<Edge>::get_forward_edge_indices(DigraphVertexIndex source) const
{
    return DigraphForwardEdgeIndexIterator<Edge>(source, m_edges);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge> Digraph<Edge>::get_backward_edge_indices(DigraphVertexIndex target) const
{
    return DigraphBackwardEdgeIndexIterator<Edge>(target, m_edges);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge> Digraph<Edge>::get_forward_edges(DigraphVertexIndex source) const
{
    return DigraphForwardEdgeIterator<Edge>(source, m_edges);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge> Digraph<Edge>::get_backward_edges(DigraphVertexIndex target) const
{
    return DigraphBackwardEdgeIterator<Edge>(target, m_edges);
}

template<IsDigraphEdge Edge>
size_t Digraph<Edge>::get_num_vertices() const
{
    return m_num_vertices;
}

template<IsDigraphEdge Edge>
size_t Digraph<Edge>::get_num_edges() const
{
    return m_edges.size();
}

template<IsDigraphEdge Edge>
bool Digraph<Edge>::is_directed() const
{
    return m_is_directed;
}

template<IsDigraphEdge Edge>
const Digraph<Edge>::EdgeList& Digraph<Edge>::get_edges() const
{
    return m_edges;
}

}
#endif
