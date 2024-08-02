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

#include "mimir/common/grouped_vector.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_iterators.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

/* Graph */

/// @brief A `Graph` G consists of vertices V and edges E.
/// A `Graph` provides functionality for adding vertices and edges, and iterating over vertices and edges.
/// Iterating over the outgoing or incoming edges,
/// as well as the ingoing or outgoing adjacent vertices requires time O(|E|).
///
/// A `Graph` can be translated into an equivalent ForwardGraph or Bidirectional Graph in O(|V|+|E|*Log2(|E|))
/// to iterate more efficiently over exactly the outgoing or incoming edges,
/// as well as the ingoing or outgoing adjacent vertices.
/// @tparam Vertex is vertex type.
/// @tparam Edge is the edge type.
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

    // Slice over all edges for iteration.
    std::vector<EdgeIndex> m_slice;

public:
    Graph();

    template<typename... Args>
    VertexIndex add_vertex(Args&&... args);

    template<typename... Args>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args);

    template<typename... Args>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args);

    /// @brief Reinitialize the graph to an empty graph.
    void reset();

    /**
     * Iterators
     */

    TargetVertexIterator<Vertex, Edge> get_targets(VertexIndex source) const;
    SourceVertexIterator<Vertex, Edge> get_sources(VertexIndex target) const;
    TargetVertexIndexIterator<Edge> get_target_indices(VertexIndex source) const;
    SourceVertexIndexIterator<Edge> get_source_indices(VertexIndex target) const;
    ForwardEdgeIterator<Edge> get_forward_edges(VertexIndex source) const;
    BackwardEdgeIterator<Edge> get_backward_edges(VertexIndex target) const;
    ForwardEdgeIndexIterator<Edge> get_forward_edge_indices(VertexIndex source) const;
    BackwardEdgeIndexIterator<Edge> get_backward_edge_indices(VertexIndex target) const;

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
template<IsGraph G>
class ForwardGraph
{
public:
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;

public:
    explicit ForwardGraph(G graph);

    /**
     * Iterators
     */

    TargetVertexIterator<VertexType, EdgeType> get_targets(VertexIndex source) const;
    SourceVertexIterator<VertexType, EdgeType> get_sources(VertexIndex target) const;
    TargetVertexIndexIterator<EdgeType> get_target_indices(VertexIndex source) const;
    SourceVertexIndexIterator<EdgeType> get_source_indices(VertexIndex target) const;
    ForwardEdgeIterator<EdgeType> get_forward_edges(VertexIndex source) const;
    BackwardEdgeIterator<EdgeType> get_backward_edges(VertexIndex target) const;
    ForwardEdgeIndexIterator<EdgeType> get_forward_edge_indices(VertexIndex source) const;
    BackwardEdgeIndexIterator<EdgeType> get_backward_edge_indices(VertexIndex target) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
};

/* BidirectionalGraph */

/// @brief BidirectionalGraph is a translated Graph for more efficient bidirectional iteration.
template<IsGraph G>
class BidirectionalGraph
{
public:
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;
    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_target;

public:
    explicit BidirectionalGraph(G graph);

    /**
     * Iterators
     */

    TargetVertexIterator<VertexType, EdgeType> get_targets(VertexIndex source) const;
    SourceVertexIterator<VertexType, EdgeType> get_sources(VertexIndex target) const;
    TargetVertexIndexIterator<EdgeType> get_target_indices(VertexIndex source) const;
    SourceVertexIndexIterator<EdgeType> get_source_indices(VertexIndex target) const;
    ForwardEdgeIterator<EdgeType> get_forward_edges(VertexIndex source) const;
    BackwardEdgeIterator<EdgeType> get_backward_edges(VertexIndex target) const;
    ForwardEdgeIndexIterator<EdgeType> get_forward_edge_indices(VertexIndex source) const;
    BackwardEdgeIndexIterator<EdgeType> get_backward_edge_indices(VertexIndex target) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
};

/**
 * Implementations
 */

/* Graph */

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
    return TargetVertexIterator<Vertex, Edge>(source, m_vertices, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge> Graph<Vertex, Edge>::get_sources(VertexIndex target) const
{
    return SourceVertexIterator<Vertex, Edge>(target, m_vertices, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIndexIterator<Edge> Graph<Vertex, Edge>::get_target_indices(VertexIndex source) const
{
    return TargetVertexIndexIterator<Edge>(source, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIndexIterator<Edge> Graph<Vertex, Edge>::get_source_indices(VertexIndex target) const
{
    return SourceVertexIndexIterator<Edge>(target, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
ForwardEdgeIterator<Edge> Graph<Vertex, Edge>::get_forward_edges(VertexIndex source) const
{
    return ForwardEdgeIterator<Edge>(source, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
BackwardEdgeIterator<Edge> Graph<Vertex, Edge>::get_backward_edges(VertexIndex target) const
{
    return BackwardEdgeIterator<Edge>(target, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
ForwardEdgeIndexIterator<Edge> Graph<Vertex, Edge>::get_forward_edge_indices(VertexIndex source) const
{
    return ForwardEdgeIndexIterator<Edge>(source, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
BackwardEdgeIndexIterator<Edge> Graph<Vertex, Edge>::get_backward_edge_indices(VertexIndex target) const
{
    return BackwardEdgeIndexIterator<Edge>(target, m_edges, m_slice);
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

/* ForwardGraph */

/// @brief Groups edge indices by source or target, depending on forward is true or false.
/// @tparam Vertex is the type of vertices in the graph.
/// @tparam Edge is the type of edges in the graph.
/// @param graph is the graph.
/// @param forward true will group by source and false will group by target.
/// @return
template<IsVertex Vertex, IsEdge Edge>
static IndexGroupedVector<const EdgeIndex> compute_index_grouped_edge_indices(const Graph<Vertex, Edge>& graph, bool forward)
{
    auto grouped_by_source_data = std::vector<std::pair<VertexIndex, EdgeIndex>> {};
    for (const auto& edge : graph.get_edges())
    {
        grouped_by_source_data.emplace_back(((forward) ? edge.get_source() : edge.get_target()), edge.get_index());
    }

    std::sort(grouped_by_source_data.begin(), grouped_by_source_data.end(), [](const auto& l, const auto& r) { return l.first < r.first; });

    auto index_grouped_by_source = IndexGroupedVector<std::pair<VertexIndex, EdgeIndex>>::create(
        std::move(grouped_by_source_data),
        [](const auto& l, const auto& r) { return l.first != r.first; },
        [](const auto& e) { return e.first; },
        graph.get_num_vertices());

    auto edge_index_grouped_by_source_builder = IndexGroupedVectorBuilder<const EdgeIndex>();
    for (const auto& group : index_grouped_by_source)
    {
        edge_index_grouped_by_source_builder.start_group();

        for (const auto& element : group)
        {
            edge_index_grouped_by_source_builder.add_group_element(element.second);
        }
    }
    return edge_index_grouped_by_source_builder.get_result();
}

template<IsGraph G>
ForwardGraph<G>::ForwardGraph(G graph) : m_graph(std::move(graph)), m_edge_indices_grouped_by_source(compute_index_grouped_edge_indices(m_graph, true))
{
}

template<IsGraph G>
TargetVertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_targets(VertexIndex source) const
{
    return TargetVertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType>(source,
                                                                                                          m_graph.get_vertices(),
                                                                                                          m_graph.get_edges(),
                                                                                                          m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
SourceVertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_sources(VertexIndex target) const
{
    return m_graph.get_sources(target);
}

template<IsGraph G>
TargetVertexIndexIterator<typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_target_indices(VertexIndex source) const
{
    return TargetVertexIndexIterator<typename ForwardGraph<G>::EdgeType>(source, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
SourceVertexIndexIterator<typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_source_indices(VertexIndex target) const
{
    return m_graph.get_source_indices(target);
}

template<IsGraph G>
ForwardEdgeIterator<typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_forward_edges(VertexIndex source) const
{
    return ForwardEdgeIterator<typename ForwardGraph<G>::EdgeType>(source, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
BackwardEdgeIterator<typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_backward_edges(VertexIndex target) const
{
    m_graph.get_backward_edges(target);
}

template<IsGraph G>
ForwardEdgeIndexIterator<typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_forward_edge_indices(VertexIndex source) const
{
    return ForwardEdgeIndexIterator<typename ForwardGraph<G>::EdgeType>(source,
                                                                        m_graph.get_vertices(),
                                                                        m_graph.get_edges(),
                                                                        m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
BackwardEdgeIndexIterator<typename ForwardGraph<G>::EdgeType> ForwardGraph<G>::get_backward_edge_indices(VertexIndex target) const
{
    m_graph.get_backward_edges(target);
}

template<IsGraph G>
const ForwardGraph<G>::VertexList& ForwardGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsGraph G>
const ForwardGraph<G>::EdgeList& ForwardGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsGraph G>
size_t ForwardGraph<G>::get_num_vertices() const
{
    return m_graph.get_num_vertices();
}

template<IsGraph G>
size_t ForwardGraph<G>::get_num_edges() const
{
    return m_graph.get_num_edges();
}

/* BidirectionalGraph */

template<IsGraph G>
BidirectionalGraph<G>::BidirectionalGraph(G graph) :
    m_graph(std::move(graph)),
    m_edge_indices_grouped_by_source(compute_index_grouped_edge_indices(m_graph, true)),
    m_edge_indices_grouped_by_target(compute_index_grouped_edge_indices(m_graph, false))
{
}

template<IsGraph G>
TargetVertexIterator<typename BidirectionalGraph<G>::VertexType, typename BidirectionalGraph<G>::EdgeType>
BidirectionalGraph<G>::get_targets(VertexIndex source) const
{
    return TargetVertexIterator<typename BidirectionalGraph<G>::VertexType, typename BidirectionalGraph<G>::EdgeType>(
        source,
        m_graph.get_vertices(),
        m_graph.get_edges(),
        m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
SourceVertexIterator<typename BidirectionalGraph<G>::VertexType, typename BidirectionalGraph<G>::EdgeType>
BidirectionalGraph<G>::get_sources(VertexIndex target) const
{
    return SourceVertexIterator<typename BidirectionalGraph<G>::VertexType, typename BidirectionalGraph<G>::EdgeType>(
        target,
        m_graph.get_vertices(),
        m_graph.get_edges(),
        m_edge_indices_grouped_by_target.at(target));
}

template<IsGraph G>
TargetVertexIndexIterator<typename BidirectionalGraph<G>::EdgeType> BidirectionalGraph<G>::get_target_indices(VertexIndex source) const
{
    return TargetVertexIndexIterator<typename BidirectionalGraph<G>::EdgeType>(source, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
SourceVertexIndexIterator<typename BidirectionalGraph<G>::EdgeType> BidirectionalGraph<G>::get_source_indices(VertexIndex target) const
{
    return SourceVertexIndexIterator<typename BidirectionalGraph<G>::EdgeType>(target, m_graph.get_edges(), m_edge_indices_grouped_by_target.at(target));
}

template<IsGraph G>
ForwardEdgeIterator<typename BidirectionalGraph<G>::EdgeType> BidirectionalGraph<G>::get_forward_edges(VertexIndex source) const
{
    return ForwardEdgeIterator<typename BidirectionalGraph<G>::EdgeType>(source, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
BackwardEdgeIterator<typename BidirectionalGraph<G>::EdgeType> BidirectionalGraph<G>::get_backward_edges(VertexIndex target) const
{
    return BackwardEdgeIterator<typename BidirectionalGraph<G>::EdgeType>(target, m_graph.get_edges(), m_edge_indices_grouped_by_target.at(target));
}

template<IsGraph G>
ForwardEdgeIndexIterator<typename BidirectionalGraph<G>::EdgeType> BidirectionalGraph<G>::get_forward_edge_indices(VertexIndex source) const
{
    return ForwardEdgeIndexIterator<typename BidirectionalGraph<G>::EdgeType>(source, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(source));
}

template<IsGraph G>
BackwardEdgeIndexIterator<typename BidirectionalGraph<G>::EdgeType> BidirectionalGraph<G>::get_backward_edge_indices(VertexIndex target) const
{
    return BackwardEdgeIndexIterator<typename BidirectionalGraph<G>::EdgeType>(target, m_graph.get_edges(), m_edge_indices_grouped_by_target.at(target));
}

template<IsGraph G>
const BidirectionalGraph<G>::VertexList& BidirectionalGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsGraph G>
const BidirectionalGraph<G>::EdgeList& BidirectionalGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsGraph G>
size_t BidirectionalGraph<G>::get_num_vertices() const
{
    return m_graph.get_num_vertices();
}

template<IsGraph G>
size_t BidirectionalGraph<G>::get_num_edges() const
{
    return m_graph.get_num_edges();
}
}
#endif
