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

#ifndef MIMIR_GRAPHS_GRAPH_HPP_
#define MIMIR_GRAPHS_GRAPH_HPP_

#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_iterators.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir::graphs
{

/**
 * Declarations
 */

/* Graph */

/// @brief Graph is a type of graph with functionality to add vertices and edges but never remove them.
/// @tparam Edge
/// @tparam Vertex
template<IsVertex Vertex, IsEdge Edge>
class Graph
{
public:
    using VertexType = Vertex;
    using VertexList = std::vector<Vertex>;
    using EdgeType = Edge;
    using EdgeList = std::vector<Edge>;

private:
    VertexList m_vertices;
    EdgeList m_edges;

public:
    Graph();

    template<typename... Args>
    VertexIndex add_vertex(Args&&... args);

    template<typename... Args>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args);

    template<typename... Args>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args);

    /// @brief Reinitialize the graph to an empty graph with num_vertices many vertices.
    void reset();

    /**
     * Iterators
     */

    TargetVertexIterator<Vertex, Edge> get_targets(VertexIndex source) const;
    SourceVertexIterator<Vertex, Edge> get_sources(VertexIndex target) const;
    ForwardEdgeIterator<Edge> get_forward_edges(VertexIndex source) const;
    BackwardEdgeIterator<Edge> get_backward_edges(VertexIndex target) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
};

/* ForwardGraph */
/// @brief ForwardGraph is a translated Graph for more efficient forward iteration.

/* BidirectionalGraph */
/// @brief BidirectionalGraph is a translated Graph for more efficient bidirectional iteration.

/**
 * Implementations
 */

/* Digraph */

template<IsVertex Vertex, IsEdge Edge>
Graph<Vertex, Edge>::Graph() : m_vertices(), m_edges()
{
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
VertexIndex Graph<Vertex, Edge>::add_vertex(Args&&... args)
{
    const auto index = m_vertices.size();
    m_vertices.emplace_back(index, std::forward<Args>(args)...);
    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
EdgeIndex Graph<Vertex, Edge>::add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (source >= get_num_vertices() || target >= get_num_vertices() || source < 0 || target < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    const auto index = m_edges.size();
    m_edges.emplace_back(index, source, target, std::forward<Args>(args)...);
    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
std::pair<EdgeIndex, EdgeIndex> Graph<Vertex, Edge>::add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (source >= get_num_vertices() || target >= get_num_vertices() || source < 0 || target < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    // Need to copy args to keep them in valid state.
    const auto forward_edge_index = add_directed_edge(source, target, args...);
    const auto backward_edge_index = add_directed_edge(target, source, std::forward<Args>(args)...);
    return std::make_pair(forward_edge_index, backward_edge_index);
}

template<IsVertex Vertex, IsEdge Edge>
void Graph<Vertex, Edge>::reset()
{
    m_vertices.clear();
    m_edges.clear();
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge> Graph<Vertex, Edge>::get_targets(VertexIndex source) const
{
    return TargetVertexIterator<Vertex, Edge>(source, m_vertices, m_edges);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge> Graph<Vertex, Edge>::get_sources(VertexIndex target) const
{
    return SourceVertexIterator<Vertex, Edge>(target, m_vertices, m_edges);
}

template<IsVertex Vertex, IsEdge Edge>
ForwardEdgeIterator<Edge> Graph<Vertex, Edge>::get_forward_edges(VertexIndex source) const
{
    return ForwardEdgeIterator<Edge>(source, m_edges);
}

template<IsVertex Vertex, IsEdge Edge>
BackwardEdgeIterator<Edge> Graph<Vertex, Edge>::get_backward_edges(VertexIndex target) const
{
    return BackwardEdgeIterator<Edge>(target, m_edges);
}

template<IsVertex Vertex, IsEdge Edge>
const Graph<Vertex, Edge>::VertexList& Graph<Vertex, Edge>::get_vertices() const
{
    return m_vertices;
}

template<IsVertex Vertex, IsEdge Edge>
const Graph<Vertex, Edge>::EdgeList& Graph<Vertex, Edge>::get_edges() const
{
    return m_edges;
}

template<IsVertex Vertex, IsEdge Edge>
size_t Graph<Vertex, Edge>::get_num_vertices() const
{
    return m_vertices.size();
}

template<IsVertex Vertex, IsEdge Edge>
size_t Graph<Vertex, Edge>::get_num_edges() const
{
    return m_edges.size();
}

}
#endif
